#include <algorithm>
#include <arm_neon.h>
#include <array>
#include <cstdint>
#include <stdint.h>
#include <sys/types.h>

#define ALPHABET_LEN 26

// Only works for words with alphabet a-z and length of at most 32
// characters
int LevenshteinMyers32(const char *q_wrd, int q_wrd_len, const char *d_wrd,
                       int d_wrd_len) {

  uint32_t bm[ALPHABET_LEN] = {0}; // Bitmap for each letter in the alphabet

  // Initialize the bitmap
  for (int i = 0; i < q_wrd_len; i++) {
    bm[q_wrd[i] - 'a'] |= 1 << i;
  }

  uint32_t vp = 0xFFFFFFFF;
  uint32_t vn = 0;
  int score = q_wrd_len;

  for (int i = 0; i < d_wrd_len; i++) {
    uint32_t c_bm = bm[d_wrd[i] - 'a'];

    uint32_t x = c_bm | vn;
    uint32_t d0 = ((vp + (x & vp)) ^ vp) | x;
    uint32_t hn = vp & d0;
    uint32_t hp = vn | ~(vp | d0);
    uint32_t y = (hp << 1) | 1;
    vn = y & d0;
    vp = (hn << 1) | ~(y | d0);

    if ((hp & (1 << (q_wrd_len - 1))) != 0) {
      score++;
    } else if ((hn & (1 << (q_wrd_len - 1))) != 0) {
      score--;
    }
  }

  return score;
}

static const uint32x4_t NULL_V = vdupq_n_u32(0);
static const uint32x4_t ONE_V = vdupq_n_u32(1);

struct Myers32x4Input {
  const char *q_wrd;
  int q_wrd_len;
  const char *d_wrds[4];
  int d_wrd_lens[4];
};

std::array<uint32_t, 4> LevenshteinMyers32x4Simd(const Myers32x4Input &input) {
  uint32_t bm[ALPHABET_LEN] = {0}; // Bitmap for each letter in the alphabet

  const char *q_wrd = input.q_wrd;
  uint32_t q_wrd_len = input.q_wrd_len;
  uint32x4_t scores = {q_wrd_len, q_wrd_len, q_wrd_len, q_wrd_len};

  uint32x4_t vp = vdupq_n_u32(0xFFFFFFFF);
  uint32x4_t vn = vdupq_n_u32(0);
  uint32x4_t x, y, hn, hp, d0;

  // Initialize the bitmap
  for (int i = 0; i < q_wrd_len; i++) {
    bm[q_wrd[i] - 'a'] |= 1 << i;
  }

  uint32x4_t d_wrd_lens = {
      (uint32_t)input.d_wrd_lens[0],
      (uint32_t)input.d_wrd_lens[1],
      (uint32_t)input.d_wrd_lens[2],
      (uint32_t)input.d_wrd_lens[3],
  };

  uint32x4_t q_wrd_len_ls = vshlq_u32(ONE_V, vdupq_n_u32(q_wrd_len - 1));

  int max_d_wrd_len =
      std::max(std::max(input.d_wrd_lens[0], input.d_wrd_lens[1]),
               std::max(input.d_wrd_lens[2], input.d_wrd_lens[3]));

  for (int i = 0; i < max_d_wrd_len; i++) {
    uint32_t c_bm_0 = bm[input.d_wrds[0][i] - 'a'];
    uint32_t c_bm_1 = bm[input.d_wrds[1][i] - 'a'];
    uint32_t c_bm_2 = bm[input.d_wrds[2][i] - 'a'];
    uint32_t c_bm_3 = bm[input.d_wrds[3][i] - 'a'];
    uint32x4_t c_bm = {c_bm_0, c_bm_1, c_bm_2, c_bm_3};

    x = vorrq_u32(c_bm, vn);
    d0 = vorrq_u32(veorq_u32(vaddq_u32(vandq_u32(vp, x), vp), vp), x);
    hn = vandq_u32(vp, d0);
    hp = vorrq_u32(vn, vmvnq_u32(vorrq_u32(vp, d0)));
    y = vorrq_u32(vshlq_n_u32(hp, 1), ONE_V);
    vn = vandq_u32(y, d0);
    vp = vorrq_u32(vshlq_n_u32(hn, 1), vmvnq_u32(vorrq_u32(y, d0)));

    uint32x4_t add_score = vandq_u32(hp, q_wrd_len_ls);
    uint32x4_t sub_score = vandq_u32(hn, q_wrd_len_ls);

    uint32x4_t continue_eval = vcltq_u32(vdupq_n_u32(i), d_wrd_lens);
    uint32x4_t should_add =
        vandq_u32(continue_eval, vmvnq_u32(vceqq_u32(add_score, NULL_V)));
    uint32x4_t should_not_add = vmvnq_u32(should_add);
    uint32x4_t should_sub =
        vandq_u32(continue_eval, vmvnq_u32(vceqq_u32(sub_score, NULL_V)));

    scores = vaddq_u32(scores, vandq_u32(should_add, ONE_V));
    scores = vsubq_u32(scores,
                       vandq_u32(vandq_u32(should_not_add, should_sub), ONE_V));
  }

  return std::array<uint32_t, 4>{
      vgetq_lane_u32(scores, 0), vgetq_lane_u32(scores, 1),
      vgetq_lane_u32(scores, 2), vgetq_lane_u32(scores, 3)};
}
