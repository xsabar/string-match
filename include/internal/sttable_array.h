#include "trie.h"

/**
 * @brief 数组设置状态转移
 * 
 * @param tbl 表指针
 * @param fid 源状态ID
 * @param c   转移字符
 * @param tid 目标状态ID
 * @return int 0:成功 -1:失败
 */
static int sttable_array_set(struct _sttable_array_s *tbl, int fid, char c, int tid) {
    int index = fid * CHARSET_SIZE + c;
    if (tid * CHARSET_SIZE >= tbl->size) {
        int size = tbl->size * 2;
        int *stt = (int*)realloc(tbl->stt, size * sizeof(int));
        if (stt == NULL) {
            return -1;
        }
        memset(stt + tbl->size, -1, tbl->size * sizeof(int));
        tbl->size = size;
        tbl->stt = stt;
    }
    tbl->stt[index] = tid;
    return 0;
}

/**
 * @brief 数组获取状态转移
 * 
 * @param tbl 表指针
 * @param id  源状态id
 * @param c   转移字符
 * @return int 目标状态id
 */
static int sttable_array_get(struct _sttable_array_s *tbl, int id, char c) {
    return tbl->stt[id * CHARSET_SIZE + c];
}