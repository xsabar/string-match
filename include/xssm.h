#ifndef _XSSM_H
#define _XSSM_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHARSET_SIZE 128

/**
 * 单字符串匹配算法
 * 1.基于前缀的匹配算法
 * 2.基于后缀的匹配算法
 * 3.基于子串的匹配算法
 * 4.其他算法
 */

// 基于前缀的匹配算法

/**
 * @brief kmp算法
 * 
 * @param s 文本串
 * @param p 模式串
 * @param slen 文本串长度
 * @param plen 模式串长度
 */
void kmp_search(const char *s, const char *p, int slen, int plen);

/**
 * @brief 前向自动机
 * 
 * @param s 文本串
 * @param p 模式串
 * @param slen 文本串长度
 * @param plen 模式串长度
 */
void fam_search(const char *s, const char *p, int slen, int plen);

/**
 * @brief shift and算法
 * 
 * @param s 文本串
 * @param p 模式串
 * @param slen 文本串长度
 * @param plen 模式串长度
 */
void shift_and_search(const char *s, const char *p, int slen, int plen);

/**
 * @brief shift or算法
 * 
 * @param s 文本串
 * @param p 模式串
 * @param slen 文本串长度
 * @param plen 模式串长度
 */
void shift_or_search(const char *s, const char *p, int slen, int plen);

// 基于后缀的匹配算法

/**
 * @brief bm算法
 * 
 * @param s 文本串
 * @param p 模式串
 * @param slen 文本串长度
 * @param plen 模式串长度
 */
void bm_search(const char *s, const char *p, int slen, int plen);

/**
 * @brief horspool算法
 * 
 * @param s 文本串
 * @param p 模式串
 * @param slen 文本串长度
 * @param plen 模式串长度
 */
void horspool_search(const char *s, const char *p, int slen, int plen);

/**
 * @brief sunday算法
 * 
 * @param s 文本串
 * @param p 模式串
 * @param slen 文本串长度
 * @param plen 模式串长度
 */
void sunday_search(const char *s, const char *p, int slen, int plen);

// 基于子串的匹配算法

/**
 * @brief bndm算法
 * 
 * @param s 文本串
 * @param p 模式串
 * @param slen 文本串长度
 * @param plen 模式串长度
 */
void bndm_search(const char *s, const char *p, int slen, int plen);

/**
 * @brief bom算法
 * 
 * @param s 文本串
 * @param p 模式串
 * @param slen 文本串长度
 * @param plen 模式串长度
 */
void bom_search(const char *s, const char *p, int slen, int plen);

// 其他算法

/**
 * @brief karp-rabin算法
 * 
 * @param s 文本串
 * @param p 模式串
 * @param slen 文本串长度
 * @param plen 模式串长度
 */
void kr_search(const char *s, const char *p, int slen, int plen);

#endif