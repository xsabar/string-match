#include "xssm.h"

/* 模余运算性质
 * 1. (a + b) % m = (a % m + b % m) % m
 * 2. (a * b) % m = ((a % m) * (b % m)) % m
*/

#define MULTIPLY_FACTOR 31

void kr_search(const char *s, const char *p, int slen, int plen) {
    if (slen < plen) {
        return;
    }
    uint64_t patt_hash = 0;
    uint64_t text_hash = 0;
    uint64_t minus_factor = 1;
    for (int i = 0; i < plen; i++) {
        patt_hash = patt_hash * MULTIPLY_FACTOR + p[i];
        text_hash = text_hash * MULTIPLY_FACTOR + s[i];
    }
    if (patt_hash == text_hash && memcmp(s, p, plen) == 0) {
        printf("%d ", 0);
    }
    for (int i = 1; i < plen; i++) {
        minus_factor *= MULTIPLY_FACTOR;
    }
    for (int i = plen; i < slen; i++) {
        text_hash = (text_hash - s[i - plen] * minus_factor) * MULTIPLY_FACTOR + s[i];
        if (patt_hash == text_hash && memcmp(s + i - plen + 1, p, plen) == 0) {
            printf("%d ", i - plen + 1);
        }
    }
}