#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct BstNode {
    int key;
    struct BstNode *l, *r;
} BstNode;

typedef struct AvlNode {
    int key, h;
    struct AvlNode *l, *r;
} AvlNode;

static long long rot_ll = 0, rot_rr = 0, rot_lr = 0, rot_rl = 0;

void reset_rotations(void) {
    rot_ll = rot_rr = rot_lr = rot_rl = 0;
}

long long total_rotations(void) {
    return rot_ll + rot_rr + rot_lr + rot_rl;
}

BstNode *bst_insert(BstNode *n, int key) {
    if (!n) {
        n = malloc(sizeof *n);
        if (!n) exit(1);
        n->key = key;
        n->l = n->r = NULL;
        return n;
    }
    if (key < n->key) n->l = bst_insert(n->l, key);
    else if (key > n->key) n->r = bst_insert(n->r, key);
    return n;
}

int bst_search(const BstNode *n, int key) {
    while (n) {
        if (key == n->key) return 1;
        n = (key < n->key) ? n->l : n->r;
    }
    return 0;
}

int bst_height(const BstNode *n) {
    if (!n) return 0;
    int lh = bst_height(n->l);
    int rh = bst_height(n->r);
    return 1 + (lh > rh ? lh : rh);
}

void bst_free(BstNode *n) {
    if (!n) return;
    bst_free(n->l);
    bst_free(n->r);
    free(n);
}

int avl_ht(const AvlNode *n) { return n ? n->h : 0; }
int max_int(int a, int b) { return a > b ? a : b; }
void avl_fix(AvlNode *n) { n->h = 1 + max_int(avl_ht(n->l), avl_ht(n->r)); }

AvlNode *avl_rot_right(AvlNode *y) {
    AvlNode *x = y->l;
    AvlNode *t2 = x->r;
    x->r = y;
    y->l = t2;
    avl_fix(y);
    avl_fix(x);
    return x;
}

AvlNode *avl_rot_left(AvlNode *x) {
    AvlNode *y = x->r;
    AvlNode *t2 = y->l;
    y->l = x;
    x->r = t2;
    avl_fix(x);
    avl_fix(y);
    return y;
}

AvlNode *avl_insert(AvlNode *n, int key) {
    if (!n) {
        n = malloc(sizeof *n);
        if (!n) exit(1);
        n->key = key;
        n->h = 1;
        n->l = n->r = NULL;
        return n;
    }
    if (key < n->key) n->l = avl_insert(n->l, key);
    else if (key > n->key) n->r = avl_insert(n->r, key);
    else return n;

    avl_fix(n);
    int bal = avl_ht(n->l) - avl_ht(n->r);

    if (bal > 1 && key < n->l->key) {
        rot_ll++;
        return avl_rot_right(n);
    }
    if (bal < -1 && key > n->r->key) {
        rot_rr++;
        return avl_rot_left(n);
    }
    if (bal > 1) {
        rot_lr++;
        n->l = avl_rot_left(n->l);
        return avl_rot_right(n);
    }
    if (bal < -1) {
        rot_rl++;
        n->r = avl_rot_right(n->r);
        return avl_rot_left(n);
    }
    return n;
}

int avl_search(const AvlNode *n, int key) {
    while (n) {
        if (key == n->key) return 1;
        n = (key < n->key) ? n->l : n->r;
    }
    return 0;
}

int avl_verify_balance(const AvlNode *n) {
    if (!n) return 1;
    int bal = avl_ht(n->l) - avl_ht(n->r);
    if (bal < -1 || bal > 1) return 0;
    return avl_verify_balance(n->l) && avl_verify_balance(n->r);
}

void avl_free(AvlNode *n) {
    if (!n) return;
    avl_free(n->l);
    avl_free(n->r);
    free(n);
}

void shuffle(int *a, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
    }
}

double elapsed_ms(clock_t start, clock_t end) {
    return ((double)(end - start) * 1000.0) / CLOCKS_PER_SEC;
}

int main(void) {
    const int N = 100000;
    int *sorted = malloc(sizeof(int) * N);
    int *rand_keys = malloc(sizeof(int) * N);
    if (!sorted || !rand_keys) return 1;

    srand(12345);
    for (int i = 0; i < N; i++) {
        sorted[i] = i + 1;
        rand_keys[i] = i + 1;
    }
    shuffle(rand_keys, N);

    /* Sorted insertions */
    BstNode *bst_s = NULL;
    clock_t t0 = clock();
    for (int i = 0; i < N; i++) bst_s = bst_insert(bst_s, sorted[i]);
    clock_t t1 = clock();

    volatile int found = 0;
    clock_t t2 = clock();
    for (int i = 0; i < N; i++) found += bst_search(bst_s, sorted[i]);
    clock_t t3 = clock();

    reset_rotations();
    AvlNode *avl_s = NULL;
    clock_t t4 = clock();
    for (int i = 0; i < N; i++) avl_s = avl_insert(avl_s, sorted[i]);
    clock_t t5 = clock();

    long long sorted_rots = total_rotations();
    int sorted_avl_valid = avl_verify_balance(avl_s);

    clock_t t6 = clock();
    for (int i = 0; i < N; i++) found += avl_search(avl_s, sorted[i]);
    clock_t t7 = clock();

    printf("=== 100,000 SORTED KEYS ===\n");
    printf("BST Height: %d | Insert: %.2f ms | Search All: %.2f ms\n",
           bst_height(bst_s), elapsed_ms(t0, t1), elapsed_ms(t2, t3));
    printf("AVL Height: %d | Insert: %.2f ms | Search All: %.2f ms | Rotations: %lld | Valid: %d\n",
           avl_ht(avl_s), elapsed_ms(t4, t5), elapsed_ms(t6, t7), sorted_rots, sorted_avl_valid);

    /* Random insertions */
    reset_rotations();
    AvlNode *avl_r = NULL;
    clock_t t8 = clock();
    for (int i = 0; i < N; i++) avl_r = avl_insert(avl_r, rand_keys[i]);
    clock_t t9 = clock();

    printf("\n=== 100,000 RANDOM KEYS ===\n");
    printf("AVL Height: %d | Insert: %.2f ms | Rotations: %lld (LL=%lld, RR=%lld, LR=%lld, RL=%lld) | Valid: %d\n",
           avl_ht(avl_r), elapsed_ms(t8, t9), total_rotations(),
           rot_ll, rot_rr, rot_lr, rot_rl, avl_verify_balance(avl_r));

    bst_free(bst_s);
    avl_free(avl_s);
    avl_free(avl_r);
    free(sorted);
    free(rand_keys);
    return 0;
}