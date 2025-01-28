#include <algorithm>
#include <arm_neon.h>
#include <array>
#include <cstdint>
#include <stdint.h>
#include <sys/types.h>

#define A_ALPHABET_LEN 26

static const uint16x8_t NULL_V_16 = vdupq_n_u16(0);
static const uint16x8_t ONE_V_16 = vdupq_n_u16(1);

struct Myers16x8Input {
  const char *q_wrd;
  int q_wrd_len;
  const char *d_wrds[8];
  uint16_t d_wrd_lens[8];
};

std::array<uint16_t, 8> LevenshteinMyers16x8Simd(const Myers16x8Input &input) {
  uint16_t bm[A_ALPHABET_LEN] = {0}; // Bitmap for each letter in the alphabet

  const char *q_wrd = input.q_wrd;
  uint16_t q_wrd_len = input.q_wrd_len;
  uint16x8_t scores = vdupq_n_u16(q_wrd_len);

  uint16x8_t vp = vdupq_n_u16(0xFFFF);
  uint16x8_t vn = vdupq_n_u16(0);
  uint16x8_t x, y, hn, hp, d0;

  // Initialize the bitmap
  for (int i = 0; i < q_wrd_len; i++) {
    bm[q_wrd[i] - 'a'] |= 1 << i;
  }

  uint16x8_t d_wrd_lens = vld1q_u16(input.d_wrd_lens);

  uint16x8_t q_wrd_len_ls = vshlq_u16(ONE_V_16, vdupq_n_u16(q_wrd_len - 1));

  int max_d_wrd_len =
      std::max(std::max(std::max(input.d_wrd_lens[0], input.d_wrd_lens[1]),
                        std::max(input.d_wrd_lens[2], input.d_wrd_lens[3])),
               std::max(std::max(input.d_wrd_lens[4], input.d_wrd_lens[5]),
                        std::max(input.d_wrd_lens[6], input.d_wrd_lens[7])));

  for (int i = 0; i < max_d_wrd_len; i++) {
    uint16_t c_bm_0 = bm[input.d_wrds[0][i] - 'a'];
    uint16_t c_bm_1 = bm[input.d_wrds[1][i] - 'a'];
    uint16_t c_bm_2 = bm[input.d_wrds[2][i] - 'a'];
    uint16_t c_bm_3 = bm[input.d_wrds[3][i] - 'a'];
    uint16_t c_bm_4 = bm[input.d_wrds[4][i] - 'a'];
    uint16_t c_bm_5 = bm[input.d_wrds[5][i] - 'a'];
    uint16_t c_bm_6 = bm[input.d_wrds[6][i] - 'a'];
    uint16_t c_bm_7 = bm[input.d_wrds[7][i] - 'a'];
    uint16x8_t c_bm = {c_bm_0, c_bm_1, c_bm_2, c_bm_3,
                       c_bm_4, c_bm_5, c_bm_6, c_bm_7};

    x = vorrq_u16(c_bm, vn);
    d0 = vorrq_u16(veorq_u16(vaddq_u16(vandq_u16(vp, x), vp), vp), x);
    hn = vandq_u16(vp, d0);
    hp = vorrq_u16(vn, vmvnq_u16(vorrq_u16(vp, d0)));
    y = vorrq_u16(vshlq_n_u16(hp, 1), ONE_V_16);
    vn = vandq_u16(y, d0);
    vp = vorrq_u16(vshlq_n_u16(hn, 1), vmvnq_u16(vorrq_u16(y, d0)));

    uint16x8_t add_score = vandq_u16(hp, q_wrd_len_ls);
    uint16x8_t sub_score = vandq_u16(hn, q_wrd_len_ls);

    uint16x8_t continue_eval = vcltq_u16(vdupq_n_u16(i), d_wrd_lens);
    uint16x8_t should_add =
        vandq_u16(continue_eval, vmvnq_u16(vceqq_u16(add_score, NULL_V_16)));
    uint16x8_t should_not_add = vmvnq_u16(should_add);
    uint16x8_t should_sub =
        vandq_u16(continue_eval, vmvnq_u16(vceqq_u16(sub_score, NULL_V_16)));

    scores = vaddq_u16(scores, vandq_u16(should_add, ONE_V_16));
    scores = vsubq_u16(
        scores, vandq_u16(vandq_u16(should_not_add, should_sub), ONE_V_16));
  }

  return std::array<uint16_t, 8>{
      vgetq_lane_u16(scores, 0), vgetq_lane_u16(scores, 1),
      vgetq_lane_u16(scores, 2), vgetq_lane_u16(scores, 3),
      vgetq_lane_u16(scores, 4), vgetq_lane_u16(scores, 5),
      vgetq_lane_u16(scores, 6), vgetq_lane_u16(scores, 7)};
}
