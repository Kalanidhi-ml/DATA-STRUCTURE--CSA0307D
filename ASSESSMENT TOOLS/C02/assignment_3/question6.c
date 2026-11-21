#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_WORD_LEN 64

typedef struct Trie {
    struct Trie *child[26];
    int words;
    int end;
} Trie;

typedef struct {
    char **items;
    size_t count;
    size_t cap;
} StringList;

static size_t trie_alloc_bytes = 0;
static size_t trie_node_count = 0;

static Trie *trie_new_node(void) {
    Trie *n = (Trie *)calloc(1, sizeof(Trie));
    if (!n) exit(1);
    trie_alloc_bytes += sizeof(Trie);
    trie_node_count++;
    return n;
}

static int is_lower_alpha(char c) {
    return (c >= 'a' && c <= 'z');
}

static void trie_insert(Trie *root, const char *word) {
    if (!root || !word || word[0] == '\0') return;
    for (int i = 0; word[i] != '\0'; i++) {
        if (!is_lower_alpha(word[i])) return;
    }

    Trie *curr = root;
    for (int i = 0; word[i] != '\0'; i++) {
        int idx = word[i] - 'a';
        if (!curr->child[idx]) {
            curr->child[idx] = trie_new_node();
        }
        curr = curr->child[idx];
        curr->words++;
    }
    curr->end++;
}

static int trie_search(const Trie *root, const char *word) {
    if (!root || !word || word[0] == '\0') return 0;
    const Trie *curr = root;
    for (int i = 0; word[i] != '\0'; i++) {
        if (!is_lower_alpha(word[i])) return 0;
        int idx = word[i] - 'a';
        if (!curr->child[idx]) return 0;
        curr = curr->child[idx];
    }
    return curr->end > 0 ? 1 : 0;
}

static const Trie *trie_walk_prefix(const Trie *root, const char *prefix) {
    if (!root || !prefix) return NULL;
    const Trie *curr = root;
    for (int i = 0; prefix[i] != '\0'; i++) {
        if (!is_lower_alpha(prefix[i])) return NULL;
        int idx = prefix[i] - 'a';
        if (!curr->child[idx]) return NULL;
        curr = curr->child[idx];
    }
    return curr;
}

static int trie_count_prefix(const Trie *root, const char *prefix) {
    if (!root || !prefix) return 0;
    if (prefix[0] == '\0') return root->words;
    const Trie *node = trie_walk_prefix(root, prefix);
    return node ? node->words : 0;
}

static void collect_words(const Trie *curr, char *buf, int depth, StringList *out) {
    if (!curr) return;
    if (curr->end > 0) {
        buf[depth] = '\0';
        for (int k = 0; k < curr->end; k++) {
            if (out->count == out->cap) {
                out->cap = out->cap ? out->cap * 2 : 64;
                out->items = (char **)realloc(out->items, out->cap * sizeof(char *));
                if (!out->items) exit(1);
            }
            out->items[out->count] = (char *)malloc((size_t)depth + 1);
            if (!out->items[out->count]) exit(1);
            memcpy(out->items[out->count], buf, (size_t)depth + 1);
            out->count++;
        }
    }
    if (depth >= MAX_WORD_LEN - 1) return;
    for (int i = 0; i < 26; i++) {
        if (curr->child[i]) {
            buf[depth] = (char)('a' + i);
            collect_words(curr->child[i], buf, depth + 1, out);
        }
    }
}

static void trie_autocomplete(const Trie *root, const char *prefix, StringList *out) {
    out->items = NULL;
    out->count = 0;
    out->cap = 0;
    if (!root || !prefix) return;

    const Trie *start = (prefix[0] == '\0') ? root : trie_walk_prefix(root, prefix);
    if (!start) return;

    char buf[MAX_WORD_LEN];
    size_t plen = strlen(prefix);
    if (plen >= MAX_WORD_LEN) return;
    memcpy(buf, prefix, plen);
    collect_words(start, buf, (int)plen, out);
}

static void free_string_list(StringList *list) {
    if (!list) return;
    for (size_t i = 0; i < list->count; i++) free(list->items[i]);
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->cap = 0;
}

static int trie_delete_rec(Trie *curr, const char *word, int depth) {
    if (!curr) return 0;
    if (word[depth] == '\0') {
        if (curr->end <= 0) return 0;
        curr->end--;
        return 1;
    }
    int idx = word[depth] - 'a';
    if (!curr->child[idx]) return 0;

    int deleted = trie_delete_rec(curr->child[idx], word, depth + 1);
    if (deleted) {
        curr->child[idx]->words--;
        if (curr->child[idx]->words == 0 && curr->child[idx]->end == 0) {
            free(curr->child[idx]);
            curr->child[idx] = NULL;
            trie_node_count--;
            trie_alloc_bytes -= sizeof(Trie);
        }
    }
    return deleted;
}

static int trie_delete(Trie *root, const char *word) {
    if (!root || !word || word[0] == '\0') return 0;
    for (int i = 0; word[i] != '\0'; i++) {
        if (!is_lower_alpha(word[i])) return 0;
    }
    return trie_delete_rec(root, word, 0);
}

static void trie_free(Trie *curr) {
    if (!curr) return;
    for (int i = 0; i < 26; i++) {
        if (curr->child[i]) trie_free(curr->child[i]);
    }
    free(curr);
}

static void generate_names(char **arr, int n) {
    const char *first[] = {"arun", "anand", "anitha", "anu", "bala", "bhavana",
                           "deepak", "divya", "gokul", "harish", "kavya", "kiran",
                           "manoj", "meena", "nithin", "pooja", "rahul", "ramya",
                           "sanjay", "sneha", "vignesh", "vikram"};
    const char *last[] = {"kumar", "sharma", "patel", "reddy", "nair", "rao",
                          "iyer", "menon", "das", "pillai", "verma", "singh"};
    int f_cnt = (int)(sizeof(first) / sizeof(first[0]));
    int l_cnt = (int)(sizeof(last) / sizeof(last[0]));

    for (int i = 0; i < n; i++) {
        char buf[MAX_WORD_LEN];
        snprintf(buf, sizeof(buf), "%s%s%d",
                 first[i % f_cnt],
                 last[(i / f_cnt) % l_cnt],
                 i);
        for (int j = 0; buf[j] != '\0'; j++) {
            if (buf[j] >= '0' && buf[j] <= '9') {
                buf[j] = (char)('a' + (buf[j] - '0'));
            }
        }
        arr[i] = (char *)malloc(strlen(buf) + 1);
        if (!arr[i]) exit(1);
        strcpy(arr[i], buf);
    }
}

static double elapsed_ns_per_op(clock_t start, clock_t end, long ops) {
    double sec = (double)(end - start) / (double)CLOCKS_PER_SEC;
    return (sec * 1e9) / (double)ops;
}

static void run_scale_benchmark(int n) {
    char **names = (char **)malloc((size_t)n * sizeof(char *));
    if (!names) exit(1);
    generate_names(names, n);

    trie_alloc_bytes = 0;
    trie_node_count = 0;
    Trie *root = trie_new_node();

    for (int i = 0; i < n; i++) {
        trie_insert(root, names[i]);
    }

    size_t array_bytes = (size_t)n * sizeof(char *);
    for (int i = 0; i < n; i++) {
        array_bytes += strlen(names[i]) + 1;
    }

    const char *queries[] = {"a", "an", "arun", "san", "zzz"};
    int q_count = (int)(sizeof(queries) / sizeof(queries[0]));
    long total_ops = 20000L;
    volatile int sink = 0;

    clock_t t0 = clock();
    for (long op = 0; op < total_ops; op++) {
        sink += trie_count_prefix(root, queries[op % q_count]);
    }
    clock_t t1 = clock();

    printf("N=%5d | Nodes=%6zu | TrieMem=%8.2f KB | ArrayMem=%7.2f KB | AvgPrefixSearch=%7.1f ns\n",
           n,
           trie_node_count,
           (double)trie_alloc_bytes / 1024.0,
           (double)array_bytes / 1024.0,
           elapsed_ns_per_op(t0, t1, total_ops));

    trie_free(root);
    for (int i = 0; i < n; i++) free(names[i]);
    free(names);
}

int main(void) {
    const char *contacts[] = {"anand", "anitha", "anu", "arun", "bala", "anusuya"};
    Trie *starter = trie_new_node();
    for (int i = 0; i < 6; i++) trie_insert(starter, contacts[i]);

    const char *typed[] = {"a", "an", "anu", "b", "z"};
    for (int i = 0; i < 5; i++)
        printf("%s:%d ", typed[i], trie_count_prefix(starter, typed[i]));
    printf("\n\n");

    const char *tests[] = {"", "anu", "an", "b", "zzz"};
    for (int i = 0; i < 5; i++) {
        StringList matches;
        trie_autocomplete(starter, tests[i], &matches);
        printf("Prefix \"%s\" -> count=%d | matches=[", tests[i], trie_count_prefix(starter, tests[i]));
        for (size_t j = 0; j < matches.count; j++) {
            printf("%s%s", matches.items[j], (j + 1 < matches.count) ? ", " : "");
        }
        printf("]\n");
        free_string_list(&matches);
    }

    printf("\nSearch \"anu\" before delete: %d | After delete \"anu\": ", trie_search(starter, "anu"));
    trie_delete(starter, "anu");
    printf("%d | count(\"anu\")=%d\n\n", trie_search(starter, "anu"), trie_count_prefix(starter, "anu"));
    trie_free(starter);

    run_scale_benchmark(500);
    run_scale_benchmark(5000);
    run_scale_benchmark(50000);
    return 0;
}