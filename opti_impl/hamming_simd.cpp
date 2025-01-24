#include <arm_neon.h> // For SIMD intrinsics on ARM
// #include <cstddef>
#include <cstdint>
// #include <time.h>
// #include <cstdio>
// #include <cstdlib>
// #include <cstring>

static const uint8_t LUT[17][16] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF},
    {0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF},
    {0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF},
    {0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF},
    {0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF},
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF},
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF},
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF},
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF},
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF},
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF},
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF},
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
     0xFF, 0xFF, 0xFF, 0xFF},
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
     0x7F, 0xFF, 0xFF, 0xFF},
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
     0x7F, 0x7F, 0xFF, 0xFF},
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
     0x7F, 0x7F, 0x7F, 0xFF},
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
     0x7F, 0x7F, 0x7F, 0x7F},
};

int hamming_simd(const char *q_wrd, const char *d_wrd, int len) {

  const uint8_t *q_ptr = (const uint8_t *)q_wrd;
  const uint8_t *d_ptr = (const uint8_t *)d_wrd;
  int dist = 0;

  while (len >= 16) {
    // Load mask and input data
    uint8x16_t mask = vld1q_u8(LUT[16]); // Use LUT[16] for chunks of 16 bytes
    uint8x16_t q = vld1q_u8(q_ptr);
    uint8x16_t d = vld1q_u8(d_ptr);

    // Compare vectors, apply mask, and compute the bitwise NOT
    uint8x16_t cmp = vceqq_u8(q, d);
    uint8x16_t not_cmp_mask_or = vmvnq_u8(vorrq_u8(cmp, mask));

    // Count set bits (popcount) and sum them
    uint8x16_t popcount_result = vcntq_u8(not_cmp_mask_or);
    dist += vaddvq_u8(popcount_result); // Horizontal sum of all 16 bytes

    // Advance pointers and reduce remaining length
    q_ptr += 16;
    d_ptr += 16;
    len -= 16;
  }

  // Handle remaining bytes (if len < 16)
  if (len > 0) {
    uint8x16_t mask = vld1q_u8(LUT[len]); // Use LUT[len] for remaining bytes
    uint8x16_t q = vld1q_u8(q_ptr);
    uint8x16_t d = vld1q_u8(d_ptr);

    uint8x16_t cmp = vceqq_u8(q, d);
    uint8x16_t not_cmp_mask_or = vmvnq_u8(vorrq_u8(cmp, mask));

    uint8x16_t popcount_result = vcntq_u8(not_cmp_mask_or);
    dist += vaddvq_u8(popcount_result);
  }

  return dist;
}

// int hamming_distance(const char *q_wrd, const char *d_wrd, int len) {
//     int distance = 0;

//     // Iterate over the length of the strings
//     for (int i = 0; i < len; i++) {
//         if (q_wrd[i] != d_wrd[i]) {
//             distance++;
//         }
//     }

//     return distance;
// }

// // Function to generate random strings of length len
// void generate_random_string(char *str, int len) {
//     static const char charset[] =
//     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"; for
//     (int i = 0; i < len; i++) {
//         str[i] = charset[rand() % (sizeof(charset) - 1)];
//     }
//     str[len] = '\0';  // Null-terminate the string
// }

// // Function to get time in nanoseconds using clock_gettime
// long long get_time_ns() {
//     struct timespec ts;
//     clock_gettime(CLOCK_MONOTONIC, &ts);
//     return ts.tv_sec * 1000000000LL + ts.tv_nsec;
// }

// int main() {
//     srand(time(NULL));  // Seed for random number generation

//     const int len = 16; // Maximum length (16 bytes)
//     const size_t num_iterations = 100000000;  // Number of iterations for
//     performance testing (10 million) char str1[len + 1], str2[len + 1];  //
//     Strings to compare

//     // Generate random strings of length 16
//     generate_random_string(str1, len);
//     generate_random_string(str2, len);

//     printf("Hamming: %d \n", hamming_distance(str1, str2, len));
//     printf("SIMD: %d \n", hamming_simd(str1, str2, len));

//     // Measure total time for hamming_distance function over many iterations
//     long long start_time = get_time_ns();
//     int result_hd = 0;
//     for (volatile size_t i = 0; i < num_iterations; i++) {
//         result_hd += hamming_distance(str1, str2, len);
//     }
//     long long end_time = get_time_ns();
//     double time_hd = (end_time - start_time) / 1000000000.0;  // Convert to
//     seconds

//     // Measure total time for hamming_simd_32 function over many iterations
//     start_time = get_time_ns();
//     int result_simd = 0;
//     for (volatile size_t i = 0; i < num_iterations; i++) {
//         result_simd += hamming_simd(str1, str2, len);
//     }
//     end_time = get_time_ns();
//     double time_simd = (end_time - start_time) / 1000000000.0;  // Convert to
//     seconds

//     printf("Results: hamming: %d, %d \n", result_hd, result_simd);
//     // Print results
//     printf("Total time for hamming_distance over %lu iterations: %f s\n",
//     num_iterations, time_hd); printf("Total time for hamming_simd_32 over %lu
//     iterations: %f s\n", num_iterations, time_simd);

//     return 0;
// }
