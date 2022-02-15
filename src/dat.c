#include <stdlib.h>
#include <string.h>
#include "dat.h"

DATrie* dat_create() {
    DATrie *dat = (DATrie *)malloc(sizeof(DATrie));
    dat->cap = DAT_NODE_DEFAULT_NUM;
    dat->nodes = (dat_node_t *)calloc(dat->cap, sizeof(dat_node_t));
    memset(dat->nodes, 0, dat->cap * sizeof(dat_node_t));
    dat->tail.len = DAT_TAIL_DEFAULT_LEN;
    dat->tail.pos = 1;
    dat->tail.str = (char *)malloc(dat->tail.len);
    return dat;
}

DATrie* dat_create_ex(const char **patterns, int pnum) {
    DATrie *dat = dat_create();
    for (int i = 0; i < pnum; i++) {
        dat_insert(dat, patterns[i], strlen(patterns[i]));
    }
    return dat;
}

void dat_destroy(DATrie *dat) {
    if (dat != NULL) {
        free(dat->nodes);
        free(dat->tail.str);
    }
}

/**
 * @brief 插入分裂出来的字符串后缀
 * 
 * @param tail 后缀存储对象指针
 * @param s    字符串后缀
 * @return int 0:成功 -1:失败（内存不足）
 */
static int dat_tail_insert(dat_tail_t *tail, const char *s) {
    int len = strlen(s);
    if (tail->pos + len > tail->len) {
        char *str = (char *)realloc(tail->str, tail->len + DAT_TAIL_INCREMT_LEN);
        if (str == NULL) {
            return -1;
        }
        memset(str + tail->len, 0, DAT_TAIL_INCREMT_LEN);
        tail->len += DAT_TAIL_INCREMT_LEN;
        tail->str = str;
    }
    for (int i = 0; i < len; i++) {
        tail->str[tail->pos++] = s[i];
    }
    tail->str[tail->pos++] = '\0';
    return 0;
}

/**
 * @brief 扩展节点数组
 * 
 * @param dat 双数组trie树指针
 * @return int 0:成功 -1:失败（内存不足）
 */
static int dat_node_extend(DATrie *dat) {
    int cap = dat->cap + DAT_NODE_INCREMT_NUM;
    dat_node_t *nodes = (dat_node_t *)realloc(dat->nodes, cap * sizeof(dat_node_t));
    if (nodes == NULL) {
        return -1;
    }
    memset(nodes + dat->cap, 0, DAT_NODE_INCREMT_NUM * sizeof(dat_node_t));
    dat->cap = cap;
    dat->nodes = nodes;
    return 0;
}

/**
 * @brief 模式串尾部添加结束字符
 * 避免一个模式串是另一个模式串的子串
 * 
 * @param dst 目标字符串
 * @param src 来源字符串
 * @param len 字符串长度
 * @return char* 目标字符串
 */
static char* dat_add_stop_char(char *dst, const char *src, int len) {
    strncpy(dst, src, len);
    dst[len - 1] = DAT_STOP_CHAR;
    dst[len] = '\0';
    return dst;
}

/**
 * @brief 字符串比较
 * 
 * @param s1 
 * @param s2 
 * @return int 0:相同 n:第n个字符开始不同
 */
static int dat_strcmp(const char *s1, const char *s2) {
    int i = 1;
    while (*s1 == *s2) {
        if (*s1 == '\0') {
            return 0;
        }
        ++i;
        ++s1;
        ++s2;
    }
    return i;
}

/**
 * @brief 找到合适的base值
 * 
 * @param dat  双数组trie树指针
 * @param base 初始base值
 * @param c    转移字符
 * @return int -1:失败（内存不足）0-n:base值
 */
static int dat_find_base(DATrie *dat, int base, char c) {
    int nid = base + c;
    while (nid < dat->cap && dat->nodes[nid].check != 0) {
        ++nid;
    }
    if (nid >= dat->cap) {
        if (dat_node_extend(dat) != 0) {
            return -1;
        }
    }
    return nid - c;
}

/**
 * @brief 找到合适的base值
 * 
 * @param dat  双数组trie树指针
 * @param base 初始base值
 * @param str  字符数组
 * @param len  字符数
 * @return int -1:失败（内存不足）0-n:base值
 */
static int dat_find_base_ex(DATrie *dat, int base, char *str, int len) {
    int valid_base = -1;
    while (base != valid_base) {
        valid_base = base;
        for (int i = 0; i < len; i++) {
            base = dat_find_base(dat, base, str[i]);
            if (base < 0) {
                return -1;
            }
        }
    }
    return base;
}

/**
 * @brief 含共同前缀的模式串插入
 * 先插入共同前缀，再插入分裂点，再处理后缀
 * 
 * @param dat 双数组trie树指针
 * @param fid 起始节点
 * @param p   待插入的模式串后缀
 * @return int 0:成功 -1:失败（内存不足）
 */
static int dat_insert_joint(DATrie *dat, int fid, const char *p) {
    int offset = -dat->nodes[fid].base;
    int dpos = dat_strcmp(dat->tail.str + offset, p);
    if (dpos == 0) {
        return 0; // 模式串重复，直接退出
    }
    --dpos;
    // 共同子串插入trie树
    for (int i = 0; i < dpos; i++) {
        int base = dat_find_base(dat, 0, p[i]);
        if (base < 0) {
            return -1;
        }
        int tid = base + p[i];
        dat->nodes[fid].base = base;
        dat->nodes[tid].check = fid;
        fid = tid;
    }
    // 找到两个分裂点的base值
    char temp[2];
    temp[0] = dat->tail.str[offset + dpos];
    temp[1] = p[dpos];
    int base = dat_find_base_ex(dat, 0, temp, 2);
    if (base < 0) {
        return -1;
    }
    dat->nodes[fid].base = base;
    // 设置老模式串分裂点
    int tid = base + temp[0];
    dat->nodes[tid].check = fid;
    dat->nodes[tid].base = -(offset + dpos + 1);
    // 设置新模式串分裂点
    tid = base + temp[1];
    dat->nodes[tid].check = fid;
    dat->nodes[tid].base = -dat->tail.pos;
    dat_tail_insert(&dat->tail, p + dpos + 1);
    return 0;
}

static int dat_find_nodes(DATrie *dat, char *list, int fid) {
    int num = 0;
    int base = dat->nodes[fid].base;
    int maxid = base + CHARSET_SIZE;
    if (maxid > dat->cap) {
        maxid = dat->cap;
    }
    for (int tid = base; tid < maxid; tid++) {
        if (dat->nodes[tid].check == fid) {
            list[num++] = tid - base;
        }
    }
    return num;
}

/**
 * @brief 改变节点base值
 * 
 * @param dat  双数组trie树指针
 * @param fid  源节点
 * @param old_base
 * @param new_base
 * @param list 转移字符集合
 * @param num  转移字符数
 */
static void dat_change_base(DATrie *dat, int fid, int old_base, int new_base, char* list, int num) {
    for (int i = 0; i < num; i++) {
        int old_tid = old_base + list[i];
        int new_tid = new_base + list[i];
        dat_node_t *old_node = &dat->nodes[old_tid];
        dat_node_t *new_node = &dat->nodes[new_tid];
        new_node->base = old_node->base;
        new_node->check = fid;
        for (int c = 0; c < CHARSET_SIZE; c++) {
            int ttid = old_node->base + c;
            if (ttid >= dat->cap) {
                break;
            }
            if (dat->nodes[ttid].check == old_tid) {
                dat->nodes[ttid].check = new_tid;
            }
        }
        old_node->base = 0;
        old_node->check = 0;
    }
    dat->nodes[fid].base = new_base;
}

/**
 * @brief 模式串冲突时的插入方法
 * 
 * @param dat 双数组trie树指针
 * @param fid 源节点
 * @param cid 冲突节点
 * @param c   冲突字符
 * @param p 
 * @return int 0:成功 -1:失败
 */
static int dat_insert_crash(DATrie *dat, int fid, int cid, int c, const char *p) {
    // 找出源节点和冲突节点的子节点集合
    char flist[CHARSET_SIZE];
    char clist[CHARSET_SIZE];
    int fnum = dat_find_nodes(dat, flist, fid);
    int cnum = dat_find_nodes(dat, clist, cid);
    flist[fnum++] = c;
    if (fnum <= cnum) {
        int old_base = dat->nodes[fid].base;
        int new_base = dat_find_base_ex(dat, old_base, flist, fnum);
        dat_change_base(dat, fid, old_base, new_base, flist, fnum - 1);
    } else {
        int old_base = dat->nodes[cid].base;
        int new_base = dat_find_base_ex(dat, old_base, clist, cnum);
        dat_change_base(dat, cid, old_base, new_base, clist, cnum);
    }
    int tid = dat->nodes[fid].base + c;
    dat->nodes[tid].base = -dat->tail.pos;
    dat->nodes[tid].check = fid;
    dat_tail_insert(&dat->tail, p);
    return 0;
}

void dat_insert(DATrie *dat, const char *p, int plen) {
    if (plen <= 0) {
        return;
    }
    // 模式串添加结尾字符，避免一个模式串是另一个模式串的前缀
    char pattern[plen + 2]; // '#' + '\0'
    p = dat_add_stop_char(pattern, p, ++plen);
    dat_node_t *nodes = dat->nodes;
    dat_tail_t *tail = &dat->tail;
    int check = 0;
    for (int i = 0, fid = 1, tid = 1; i < plen; i++, fid = tid) {
        tid = nodes[fid].base + p[i];
        if (tid >= dat->cap) {
            dat_node_extend(dat);
        }
        check = nodes[tid].check;
        if (check == 0) {
            // 非冲突失配，插入当前转移并设置分裂点
            nodes[tid].check = fid;
            nodes[tid].base = -tail->pos;
            dat_tail_insert(tail, p + i + 1);
            break;
        }
        if (check == fid) {
            if (nodes[tid].base < 0) {
                // 匹配分裂点，先插入共同前缀，再重新设置各自的分裂点
                dat_insert_joint(dat, tid, p + i + 1);
                break;
            }
        }
        if (check != fid) {
            // 冲突型失配，解决冲突后插入当前转移并设置分裂点
            dat_insert_crash(dat, fid, check, p[i], p + i + 1);
            break;
        }
    }
}

void dat_delete(DATrie *dat, const char *p, int plen) {
    if (plen <= 0) {
        return;
    }
    // 模式串添加结尾字符，避免一个模式串是另一个模式串的前缀
    char pattern[plen + 2]; // '#' + '\0'
    p = dat_add_stop_char(pattern, p, ++plen);
    for (int i, fid = 1, tid = 1, check = 0, base = 0; i < plen; i++, fid = tid) {
        tid = dat->nodes[fid].base + p[i];
        check = tid < dat->cap ? dat->nodes[tid].check : 0;
        if (check != fid) {
            break;
        }
        base = dat->nodes[tid].base;
        if (base < 0) {
            int pos = dat_strcmp(p + i + 1, &dat->tail.str[-base]);
            if (pos == 0) {
                dat->nodes[tid].base = 0;
                dat->nodes[tid].check = 0;
            }
            break;
        }
    }
}

void dat_search(const DATrie *dat, const char *s, int slen, match_result_t *result) {
    for (int i = 0; i < slen; i++) {
        for (int j = i, fid = 1, tid = 1, check = 0, base = 0; j < slen; j++, fid = tid) {
            tid = dat->nodes[fid].base + s[j];
            check = tid < dat->cap ? dat->nodes[tid].check : 0;
            if (check != fid) {
                break;
            }
            base = dat->nodes[tid].base;
            if (base < 0) {
                int pos = dat_strcmp(s + j + 1, &dat->tail.str[-base]) - 1;
                if (dat->tail.str[-base + pos] == '#') {
                    match_result_append(result, j + pos - i + 1, i);
                }
                break;
            }
            if (base + DAT_STOP_CHAR < dat->cap && dat->nodes[base + DAT_STOP_CHAR].check == tid) {
                match_result_append(result, j - i + 1, i);
            }
        }
    }
}