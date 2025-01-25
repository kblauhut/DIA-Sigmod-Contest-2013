#include <algorithm>
#include <arm_neon.h>
#include <array>
#include <cstdint>
#include <stdint.h>
#include <sys/types.h>

#include <chrono>
#include <iostream>
#include <random>
#include <vector>

#define ALPHABET_LEN 26

// void printUint32Binary(uint32_t v) {
//   for (int i = 31; i >= 0; i--) {
//     printf("%d", (v >> i) & 1);
//   }
//   printf("\n");
// }

// void printUint32x4Binary(uint32x4_t v) {
//   uint32_t *v_arr = (uint32_t *)&v;
//   for (int i = 0; i < 4; i++) {
//     for (int j = 31; j >= 0; j--) {
//       printf("%d", (v_arr[i] >> j) & 1);
//     }
//     printf(" ");
//   }
//   printf("\n");
// }

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

void performanceTest() {
  const int NUM_TESTS = 10000000;
  const int WORD_LEN = 32;
  const int SIMD_BATCH_SIZE = 4;

  // Generate random data
  std::mt19937 rng(42);
  std::uniform_int_distribution<int> char_dist('a', 'z');

  auto generate_word = [&]() {
    std::string word;
    for (int i = 0; i < WORD_LEN; ++i) {
      word += char_dist(rng);
    }
    return word;
  };

  // Generate query word
  std::string q_wrd = generate_word();

  // Generate words for testing
  std::vector<std::string> d_words;
  for (int i = 0; i < NUM_TESTS; ++i) {
    d_words.push_back(generate_word());
  }

  // Measure performance for LevenshteinMyers32
  auto start = std::chrono::high_resolution_clock::now();
  int total_score32 = 0;
  for (int i = 0; i < NUM_TESTS; ++i) {
    total_score32 += LevenshteinMyers32(q_wrd.c_str(), q_wrd.size(),
                                        d_words[i].c_str(), d_words[i].size());
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto duration32 =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();

  std::cout << "LevenshteinMyers32 Total Score: " << total_score32 << "\n";
  std::cout << "LevenshteinMyers32 Duration: " << duration32 << " ms\n";

  // Measure performance for LevenshteinMyers32x4Simd
  start = std::chrono::high_resolution_clock::now();
  int total_score32x4 = 0;
  for (int i = 0; i < NUM_TESTS; i += SIMD_BATCH_SIZE) {
    Myers32x4Input input;
    input.q_wrd = q_wrd.c_str();
    input.q_wrd_len = q_wrd.size();

    for (int j = 0; j < SIMD_BATCH_SIZE; ++j) {
      if (i + j < NUM_TESTS) {
        input.d_wrds[j] = d_words[i + j].c_str();
        input.d_wrd_lens[j] = d_words[i + j].size();
      } else {
        input.d_wrds[j] = ""; // Filler word
        input.d_wrd_lens[j] = 0;
      }
    }

    auto simd_scores = LevenshteinMyers32x4Simd(input);
    for (int j = 0; j < SIMD_BATCH_SIZE; ++j) {
      total_score32x4 += simd_scores[j];
    }
  }
  end = std::chrono::high_resolution_clock::now();
  auto duration32x4 =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();

  std::cout << "LevenshteinMyers32x4Simd Total Score: " << total_score32x4
            << "\n";
  std::cout << "LevenshteinMyers32x4Simd Duration: " << duration32x4 << " ms\n";

  // Compare performance
  std::cout << "Speedup: " << static_cast<double>(duration32) / duration32x4
            << "x\n";
}

int main() {
  performanceTest();
  return 0;
}
// int main() {
//   const char *q_wrd = "hello";

//   MyersInput input = {q_wrd, 5,
//                       .d_wrds = {"homesweethome", "hallo", "hallo", "hello"},
//                       .d_wrd_lens = {13, 5, 5, 5}};

//   std::array<uint32_t, 4> scores = LevenshteinMyers32x4Simd(input);
//   printf("--- \n");
//   int score = LevenshteinMyers32(q_wrd, 5, "helo", 4);
//   printf("--- \n");
//   int score2 = LevenshteinMyers32(q_wrd, 5, "hello", 5);
//   printf("--- \n");

//   for (int i = 0; i < 4; i++) {
//     printf("Score %d: %d\n", i, scores[i]);
//   }

//   return 0;
// }
