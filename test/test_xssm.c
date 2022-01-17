#include <string.h>
#include "xssm.h"

typedef struct {
	char name[16];
	void (*func) (const char *, const char *, int, int);
} ssm_func_t;

int main() {
	ssm_func_t funcs[] = {
		{"kmp", kmp_search},
		{"fam", fam_search},
		{"shift and", shift_and_search},
		{"shift or", shift_or_search},
		{"bm", bm_search},
		{"horspool", horspool_search},
		{"sunday", sunday_search},
		{"bndm", bndm_search},
		{"bom", bom_search},
		{"karp rabin", kr_search},
	};
	const char *s = "abdkababcdabcdckdha";
	const char *p = "abcd";
	//const char *s = "cpm annual conference announce";
	//const char *p = "announce";
	//const char *s = "AGATACGATATATAC";
	//const char *p = "ATATA";
	int slen = strlen(s);
	int plen = strlen(p);
	for (int i = 0; i < sizeof(funcs) / sizeof(ssm_func_t); i++) {
		printf("%10s: ", funcs[i].name);
		funcs[i].func(s, p, slen, plen);
		printf("\n");
	}
	return 0;
}