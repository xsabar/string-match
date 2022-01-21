#include <string.h>
#include "bit_array.h"

void bit_array_reset(bit_array_t *bar) {
    memset(bar, 0, sizeof(bit_array_t));
    bar->min_bucket_id = INT32_MAX;
    bar->max_bucket_id = INT32_MIN;
}

void bit_array_lshift(bit_array_t *bar) {
    int min_bucket_id = bar->min_bucket_id;
    int max_bucket_id = bar->max_bucket_id;
    if (min_bucket_id > max_bucket_id) {
        return;
    }
    if (max_bucket_id < BIT_ARRAY_BUCKET_SIZE - 1) {
        if ((bar->bits[max_bucket_id + 1] = bar->bits[max_bucket_id] >> (BIT_ARRAY_BUCKET_BITS - 1)) != 0) {
            ++bar->max_bucket_id;
        }
    }
    for (int i = max_bucket_id; i > min_bucket_id; i--) {
        bar->bits[i] = (bar->bits[i] << 1) | (bar->bits[i - 1] >> (BIT_ARRAY_BUCKET_BITS - 1));
    }
    bar->bits[min_bucket_id] <<= 1;
    if (bar->bits[min_bucket_id] == 0) {
        ++bar->min_bucket_id;
    }
    while (bar->max_bucket_id >= bar->min_bucket_id && bar->bits[bar->max_bucket_id] == 0) {
        --bar->max_bucket_id;
    }
}

void bit_array_rshift(bit_array_t *bar) {
    int min_bucket_id = bar->min_bucket_id;
    int max_bucket_id = bar->max_bucket_id;
    if (min_bucket_id > max_bucket_id) {
        return;
    }
    if (min_bucket_id > 0) {
        if ((bar->bits[min_bucket_id - 1] = bar->bits[min_bucket_id] << (BIT_ARRAY_BUCKET_BITS - 1)) != 0) {
            --bar->min_bucket_id;
        }
    }
    for (int i = min_bucket_id; i < max_bucket_id - 1; i++) {
        bar->bits[i] = (bar->bits[i] >> 1) | (bar->bits[i + 1] << (BIT_ARRAY_BUCKET_BITS - 1));
    }
    bar->bits[max_bucket_id] >>= 1;
    if (bar->bits[max_bucket_id] == 0) {
        --bar->max_bucket_id;
    }
    while (bar->min_bucket_id <= bar->max_bucket_id && bar->bits[bar->min_bucket_id] == 0) {
        ++bar->min_bucket_id;
    }
}

void bit_array_set(bit_array_t *bar, int pos) {
    int bucket_id = pos / BIT_ARRAY_BUCKET_BITS;
    int bit_id = pos - bucket_id * BIT_ARRAY_BUCKET_BITS;
    bar->bits[bucket_id] |= (uint64_t)1 << bit_id;
    if (bar->max_bucket_id < bucket_id) {
        bar->max_bucket_id = bucket_id;
    }
    if (bar->min_bucket_id > bucket_id) {
        bar->min_bucket_id = bucket_id;
    }
}

int bit_array_get(const bit_array_t *bar, int pos) {
    int bucket_id = pos / BIT_ARRAY_BUCKET_BITS;
    int bit_id = pos - bucket_id * BIT_ARRAY_BUCKET_BITS;
    return (bar->bits[bucket_id] & (1 << bit_id)) >> bit_id;
}

void bit_array_and(bit_array_t *lbar, const bit_array_t *rbar) {
    int min_bucket_id = INT32_MAX;
    int max_bucket_id = INT32_MIN;
    for (int i = lbar->min_bucket_id; i <= lbar->max_bucket_id ; i++) {
        lbar->bits[i] &= rbar->bits[i];
        if (lbar->bits[i] != 0) {
            if (min_bucket_id > i) {
                min_bucket_id = i;
            }
            max_bucket_id = i;
        }
    }
    lbar->min_bucket_id = min_bucket_id;
    lbar->max_bucket_id = max_bucket_id;
}

void bit_array_or(bit_array_t *lbar, const bit_array_t *rbar) {
    if (rbar->min_bucket_id > rbar->max_bucket_id) {
        return;
    }
    if (lbar->min_bucket_id > rbar->min_bucket_id) {
        lbar->min_bucket_id = rbar->min_bucket_id;
    }
    if (lbar->max_bucket_id < rbar->max_bucket_id) {
        lbar->max_bucket_id = rbar->max_bucket_id;
    }
    for (int i = lbar->min_bucket_id; i <= lbar->max_bucket_id; i++) {
        lbar->bits[i] |= rbar->bits[i];
    }
}

int bit_array_pop(bit_array_t *bar) {
    int min_bucket_id = bar->min_bucket_id;
    int max_bucket_id = bar->max_bucket_id;
    if (min_bucket_id > max_bucket_id) {
        return -1;
    }
    uint64_t x = bar->bits[min_bucket_id];
    uint64_t y = x ^ (x - 1);
    bar->bits[min_bucket_id] = x & (x - 1);
    while (bar->min_bucket_id <= max_bucket_id && bar->bits[bar->min_bucket_id] == 0) {
        ++bar->min_bucket_id;
    }
    x = (y & 0x5555555555555555UL) + ((y >> 1) & 0x5555555555555555UL);
    y = (x & 0x3333333333333333UL) + ((x >> 2) & 0x3333333333333333UL);
    x = (y & 0x0f0f0f0f0f0f0f0fUL) + ((y >> 4) & 0x0f0f0f0f0f0f0f0fUL);
    y = (x & 0x00ff00ff00ff00ffUL) + ((x >> 8) & 0x00ff00ff00ff00ffUL);
    x = (y & 0x0000ffff0000ffffUL) + ((y >> 16) & 0x0000ffff0000ffffUL);
    y = (x & 0x00000000ffffffffUL) + (x >> 32);
    return min_bucket_id * BIT_ARRAY_BUCKET_BITS + y - 1;
}

void bit_array_copy(bit_array_t *dest, const bit_array_t *src) {
    memcpy(dest, src, sizeof(bit_array_t));
}

int bit_array_empty(bit_array_t *bar) {
    return bar->min_bucket_id > bar->max_bucket_id;
}