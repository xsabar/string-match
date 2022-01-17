#ifndef _SMIO_H
#define _SMIO_H

#define CHARSET_SIZE 128

/**
 * @brief 单个匹配项
 */
typedef struct {
    int pos; // 匹配位置
    int len; // 匹配模式串长度
    const char* pattern; // 匹配模式串
} match_item_t;

/**
 * @brief 多模式串匹配结果
 */
typedef struct {
    int size; // 最大匹配数量
    int num;  // 匹配数
    match_item_t* items; // 匹配集合
} match_result_t;

/**
 * @brief 创建匹配结果数据结构
 * 
 * @param size 匹配项数量
 * @return match_result_t* 
 */
match_result_t* match_result_create(int size);


/**
 * @brief 销毁匹配结果
 * 
 * @param result 
 */
void match_result_destroy(match_result_t *result);

/**
 * @brief 初始化匹配输出数据结构
 * 
 * @param result 匹配结果指针
 * @param items  匹配项指针
 * @param size   匹配项大小
 */
void match_result_init(match_result_t *result, match_item_t* items, int size);

/**
 * @brief 添加匹配项
 * 
 * @param result 匹配结果指针
 * @param p      模式串指针
 * @param plen   模式串长度
 * @param pos    最终匹配位置
 * @return int   0:成功 -1:失败
 */
int match_result_append(match_result_t *result, const char *p, int plen, int pos);

#endif