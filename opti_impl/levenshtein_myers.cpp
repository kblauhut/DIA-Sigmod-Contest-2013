
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
