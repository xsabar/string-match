#include "trie.h"

/**
 * @brief 获取转移字符集合
 * 
 * @param list   转移字符集合
 * @param states 状态数组
 * @param id     源状态id
 * @return int   转移字符数
 */
static int sttable_dbarr_list_child(char *list, TrieState *states, int id) {
    int num = 0;
    int child = states[id].first;
    TrieState *state = NULL;
    while (child != 0) {
        list[num++] = states[child].c;
        child = states[child].next;
    }
    return num;
}

/**
 * @brief 内存扩展
 * 
 * @param tbl  表指针
 * @return int 0:成功 -1:失败
 */
static int sttable_dbarr_extend(struct _sttable_dbarr_s *tbl) {
    int size = tbl->tsize * 2;
    int *target = (int *)realloc(tbl->target, sizeof(int) * size);
    if (target == NULL) {
        return -1;
    }
    memset(target + tbl->tsize, 0, sizeof(int) * tbl->tsize);
    tbl->tsize = size;
    tbl->target = target;
    return 0;
}

/**
 * @brief 计算合法的基数值
 * 
 * @param tbl  表指针
 * @param base 基数初始值
 * @param c    转移字符
 * @return int 基数值
 */
static int sttable_dbarr_find_base(struct _sttable_dbarr_s *tbl, int base, char c) {
    int nid = base + c;
    while (nid < tbl->tsize && tbl->target[nid] != 0) {
        ++nid;
    }
    if (nid >= tbl->tsize) {
        if (sttable_dbarr_extend(tbl) != 0) {
            return -1;
        }
    }
    return nid - c;
}

/**
 * @brief 找到合适的base值
 * 
 * @param tbl  表指针
 * @param base 初始base值
 * @param str  字符数组
 * @param len  字符数
 * @return int -1:失败（内存不足）0-n:base值
 */
static int sttable_dbarr_find_base_ex(struct _sttable_dbarr_s *tbl, int base, char *str, int len) {
    int valid_base = -1;
    while (base != valid_base) {
        valid_base = base;
        for (int i = 0; i < len; i++) {
            base = sttable_dbarr_find_base(tbl, base, str[i]);
            if (base < 0) {
                return -1;
            }
        }
    }
    return base;
}

/**
 * @brief 改变节点base值
 * 
 * @param tbl  表指针
 * @param fid  源节点
 * @param old_base 旧基数
 * @param new_base 新基数
 * @param list 转移字符集合
 * @param num  转移字符数
 */
static void sttable_dbarr_change_base(struct _sttable_dbarr_s *tbl, 
    int fid, int old_base, int new_base, char* list, int num) {
    for (int i = 0; i < num; i++) {
        int old_tid = old_base + list[i];
        int new_tid = new_base + list[i];
        tbl->target[new_tid] = tbl->target[old_tid];
        tbl->target[old_tid] = 0;
    }
    tbl->base[fid] = new_base;
}

/**
 * @brief 解决状态转移冲突
 * 
 * @param tbl 表指针
 * @param fid 来源状态id
 * @param cid 冲突状态id
 * @return int 0:成功 -1:失败
 */
static int sttable_dbarr_handle_crash(struct _sttable_dbarr_s *tbl, int fid, int cid) {
    char flist[CHARSET_SIZE];
    int fnum = sttable_dbarr_list_child(flist, tbl->trie->states, fid);
    if (fnum == 1) {
        int base = sttable_dbarr_find_base(tbl, 0, flist[0]);
        tbl->base[fid] = base;
        tbl->target[base + flist[0]] = tbl->trie->states[fid].first;
        return 0;
    }
    char clist[CHARSET_SIZE];
    int cnum = sttable_dbarr_list_child(clist, tbl->trie->states, cid);
    if (fnum <= cnum) {
        int old_base = tbl->base[fid];
        int new_base = sttable_dbarr_find_base_ex(tbl, old_base, flist, fnum);
        sttable_dbarr_change_base(tbl, fid, old_base, new_base, flist + 1, fnum - 1);
    } else {
        int old_base = tbl->base[cid];
        int new_base = sttable_dbarr_find_base_ex(tbl, old_base, clist, cnum);
        sttable_dbarr_change_base(tbl, cid, old_base, new_base, clist, cnum);
    }
    int tid = tbl->base[fid] + flist[0];
    tbl->target[tid] = tbl->trie->states[fid].first;
    return 0;
}

/**
 * @brief 设置状态转移
 * 
 * @param tbl 表指针
 * @param fid 源状态id
 * @param c   转移字符
 * @param tid 目标状态id
 * @return int 0:成功 -1:失败
 */
static int sttable_dbarr_set(struct _sttable_dbarr_s *tbl, int fid, char c, int tid) {
    if (tid >= tbl->bsize) {
        int size = tbl->bsize * 2;
        int *base = (int *)realloc(tbl->base, sizeof(int) * size);
        if (base == NULL) {
            return -1;
        }
        memset(base + tbl->bsize, 0, sizeof(int) * tbl->bsize);
        tbl->bsize = size;
        tbl->base = base;
    }
    int pos = tbl->base[fid] + c;
    if (pos >= tbl->tsize) {
        sttable_dbarr_extend(tbl);
    }
    int id = tbl->target[pos];
    if (id == 0) {
        tbl->target[pos] = tid;
    } else if (id != tid) {
        sttable_dbarr_handle_crash(tbl, fid, tbl->trie->states[id].parent);
    } else {}
    return 0;
}

/**
 * @brief 获取转移状态
 * 
 * @param tbl 表指针
 * @param id  源状态id
 * @param c   转移字符
 * @return int 目标状态id
 */
static int sttable_dbarr_get(struct _sttable_dbarr_s *tbl, int id, char c) {
    int pos = tbl->base[id] + c;
    if (pos >= tbl->tsize) {
        return -1;
    }
    int tid = tbl->target[pos];
    if (tid > 0 && tbl->trie->states[tid].parent == id) {
        return tid;
    }
    return -1;
}