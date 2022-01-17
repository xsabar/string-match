#ifndef _BIT_ARRAY_H
#define _BIT_ARRAY_H

#include <stdint.h>

#define BIT_ARRAY_MAX_BITS 1024
#define BIT_ARRAY_BUCKET_BITS 64
#define BIT_ARRAY_BUCKET_SIZE BIT_ARRAY_MAX_BITS / BIT_ARRAY_BUCKET_BITS

/**
 * @brief 位数组
 * 
 */
typedef struct {
    int min_bucket_id; // 非0最小桶ID
    int max_bucket_id; // 非0最大桶ID
    uint64_t bits[BIT_ARRAY_BUCKET_SIZE];
} bit_array_t;

/**
 * @brief 初始化为0
 * 
 * @param bar 位数组指针
 */
void bit_array_reset(bit_array_t *bar);

/**
 * @brief 左移一位
 * 
 * @param bar 位数组指针
 */
void bit_array_lshift(bit_array_t *bar);

/**
 * @brief 右移一位
 * 
 * @param bar 位数组指针
 */
void bit_array_rshift(bit_array_t *bar);

/**
 * @brief 设置对应的位
 * 
 * @param bar 位数组指针
 * @param pos 位置
 */
void bit_array_set(bit_array_t *bar, int pos);

/**
 * @brief 获取对应的位
 * 
 * @param bar 位数组指针
 * @param pos 位置
 * @return int 0｜1
 */
int bit_array_get(const bit_array_t *bar, int pos);

/**
 * @brief 按位与
 * 
 * @param lb 左操作数，结果存入左操作数
 * @param rb 右操作数
 */
void bit_array_and(bit_array_t *lb, const bit_array_t *rb);

/**
 * @brief 按位或
 * 
 * @param lb 左操作数，结果存入左操作数
 * @param rb 右操作数
 */
void bit_array_or(bit_array_t *lb, const bit_array_t *rb);

/**
 * @brief 取出并删除第一个1，返回1的位置
 * 
 * @param bar  指针
 * @return int 位置
 */
int bit_array_pop(bit_array_t *bar);

/**
 * @brief 复制位数组
 * 
 * @param lb 
 * @param rb 
 */
void bit_array_copy(bit_array_t *dest, const bit_array_t *src);

/**
 * @brief 测试位数组是否位空（全是0）
 * 
 * @param bar 
 * @return bool true:empty false:not empty 
 */
int bit_array_empty(bit_array_t *bar);

#endif