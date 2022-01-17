#include <stdio.h>
#include <string.h>

#include "trie.h"
#include "ac.h"
#include "oracle.h"
#include "shift.h"
#include "bndm.h"
#include "horspool.h"
#include "wum.h"

#define MAX_MATCH_NUM 16

static STTableType type = STTABLE_TYPE_ARRAY;

static void print_result(const char *name, const match_result_t* result) {
    printf("%16s: ", name);
    for (int i = 0; i < result->num; i++) {
        printf("(%d, %s) ", result->items[i].pos, result->items[i].pattern);
    }
    printf("\n");
}

static void print_state(Trie *trie) {
    trie_make_bfs(trie);
    for (int i = 0; i < trie->state_num; i++) {
        TrieState *state = trie->bfs_states[i];
        printf("state: id=%d depth=%d char=%c\n", state->id, state->depth, state->c);
    }
}

static void trie_search_test(const char *s, int slen, const char **patterns, int num) {
    Trie* trie = trie_create_ex(patterns, num, type);
    match_result_t *result = match_result_create(MAX_MATCH_NUM);
    trie_search(trie, s, slen, result);
    print_result("trie search", result);
    match_result_destroy(result);
    trie_destroy(trie);
}

static void ac_full_search_test(const char *s, int slen, const char **patterns, int num) {
    AC* ac = ac_create_ex(patterns, num, AC_LEVEL_FULL);
    match_result_t *result = match_result_create(MAX_MATCH_NUM);
    ac_search(ac, s, slen, result);
    print_result("ac_full search", result);
    match_result_destroy(result);
    ac_destroy(ac);
}

static void ac_part_search_test(const char *s, int slen, const char **patterns, int num) {
    AC* ac = ac_create_ex(patterns, num, AC_LEVEL_PART);
    match_result_t *result = match_result_create(MAX_MATCH_NUM);
    ac_search(ac, s, slen, result);
    print_result("ac_part search", result);
    match_result_destroy(result);
    ac_destroy(ac);
}

static void sbom_search_test(const char *s, int slen, const char **patterns, int num) {
    Oracle *orc = oracle_create_ex(patterns, num, type);
    match_result_t *result = match_result_create(MAX_MATCH_NUM);
    oracle_search(orc, s, slen, result);
    print_result("sbom search", result);
    match_result_destroy(result);
    oracle_destroy(orc);
}

static void shift_search_test(const char *s, int slen, const char **patterns, int num) {
    ShiftNFA *nfa = shift_nfa_create_ex(patterns, num);
    match_result_t *result = match_result_create(MAX_MATCH_NUM);
    shift_nfa_search(nfa, s, slen, result);
    print_result("shift search", result);
    match_result_destroy(result);
    shift_nfa_destroy(nfa);
}

static void bndm_search_test(const char *s, int slen, const char **patterns, int num) {
    BndmNFA *nfa = bndm_nfa_create_ex(patterns, num);
    match_result_t *result = match_result_create(MAX_MATCH_NUM);
    bndm_nfa_search(nfa, s, slen, result);
    print_result("bndm search", result);
    match_result_destroy(result);
    bndm_nfa_destroy(nfa);
}

static void horspool_search_test(const char *s, int slen, const char **patterns, int num) {
    Horspool *hsp = horspool_create_ex(patterns, num, 1);
    match_result_t *result = match_result_create(MAX_MATCH_NUM);
    horspool_trie_search(hsp, s, slen, result);
    print_result("horspool search", result);
    match_result_destroy(result);
    horspool_destroy(hsp);
}

static void wum_search_test(const char *s, int slen, const char **patterns, int num) {
    Wum *wum = wum_create_ex(patterns, num, 1);
    match_result_t *result = match_result_create(MAX_MATCH_NUM);
    wum_search(wum, s, slen, result);
    print_result("wum search", result);
    match_result_destroy(result);
    wum_destroy(wum);
}

int main() {
    //const char *s = "abdkababcdabcdckdhaxhxhhab";
	//const char *p[] = {"abcd", "bcd", "cda", "xhh"};
    //const char *s = "annual conference announce";
    //const char *p[] = {"announce", "annual", "annually"};
    const char *s = "AGATACGATATATAC";
    const char *p[] = {"ATATATA", "TATAT", "ACGATAT", "AT", "CGA"};
	int slen = strlen(s);
    int pnum = sizeof(p) / sizeof(p[0]);
    trie_search_test(s, slen, p, pnum);
    ac_full_search_test(s, slen, p, pnum);
    ac_part_search_test(s, slen, p, pnum);
    sbom_search_test(s, slen, p, pnum);
    shift_search_test(s, slen, p, pnum);
    bndm_search_test(s, slen, p, pnum);
    horspool_search_test(s, slen, p, pnum);
    wum_search_test(s, slen, p, pnum);
    return 0;
}