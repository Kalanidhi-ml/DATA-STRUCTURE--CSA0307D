#include <stdio.h>
#include <stdlib.h>

typedef struct SplayNode {
    int key;
    struct SplayNode *left, *right;
} SplayNode;

typedef struct AvlNode {
    int key, h;
    struct AvlNode *left, *right;
} AvlNode;

static SplayNode *new_splay_node(int key) {
    SplayNode *n = (SplayNode *)malloc(sizeof(SplayNode));
    if (!n) exit(1);
    n->key = key;
    n->left = n->right = NULL;
    return n;
}

/* Right rotation (Zig / Zig-Zig / Zig-Zag helper) */
static SplayNode *rotate_right_splay(SplayNode *x) {
    SplayNode *y = x->left;
    x->left = y->right;
    y->right = x;
    return y;
}

/* Left rotation */
static SplayNode *rotate_left_splay(SplayNode *x) {
    SplayNode *y = x->right;
    x->right = y->left;
    y->left = x;
    return y;
}

/* Splays key to root if present (or last accessed node if absent); counts visited nodes */
static SplayNode *splay(SplayNode *root, int key, long long *visited) {
    if (!root) return NULL;

    SplayNode dummy;
    dummy.left = dummy.right = NULL;
    SplayNode *left_max = &dummy;
    SplayNode *right_min = &dummy;

    while (1) {
        if (visited) (*visited)++;
        if (key < root->key) {
            if (!root->left) break;
            if (visited) (*visited)++;
            if (key < root->left->key) {
                /* Zig-Zig (left-left) */
                root = rotate_right_splay(root);
                if (!root->left) break;
            }
            /* Link root to right_min */
            right_min->left = root;
            right_min = root;
            root = root->left;
        } else if (key > root->key) {
            if (!root->right) break;
            if (visited) (*visited)++;
            if (key > root->right->key) {
                /* Zig-Zig (right-right) */
                root = rotate_left_splay(root);
                if (!root->right) break;
            }
            /* Link root to left_max */
            left_max->right = root;
            left_max = root;
            root = root->right;
        } else {
            break;
        }
    }

    /* Reassemble */
    left_max->right = root->left;
    right_min->left = root->right;
    root->left = dummy.right;
    root->right = dummy.left;
    return root;
}

static SplayNode *splay_insert(SplayNode *root, int key) {
    long long v = 0;
    if (!root) return new_splay_node(key);
    root = splay(root, key, &v);
    if (root->key == key) return root;

    SplayNode *n = new_splay_node(key);
    if (key < root->key) {
        n->right = root;
        n->left = root->left;
        root->left = NULL;
    } else {
        n->left = root;
        n->right = root->right;
        root->right = NULL;
    }
    return n;
}

/* Splay search: splays accessed (or closest) node to root; returns 1 if found */
static int splay_search(SplayNode **root, int key, long long *nodes_visited) {
    *nodes_visited = 0;
    if (!*root) return 0;
    *root = splay(*root, key, nodes_visited);
    return ((*root)->key == key) ? 1 : 0;
}

static void free_splay(SplayNode *root) {
    if (!root) return;
    free_splay(root->left);
    free_splay(root->right);
    free(root);
}

/* AVL tree for comparison */
static int avl_h(const AvlNode *n) { return n ? n->h : 0; }
static int max2(int a, int b) { return a > b ? a : b; }
static void avl_fix(AvlNode *n) { n->h = 1 + max2(avl_h(n->left), avl_h(n->right)); }

static AvlNode *avl_rot_right(AvlNode *y) {
    AvlNode *x = y->left, *t2 = x->right;
    x->right = y;
    y->left = t2;
    avl_fix(y);
    avl_fix(x);
    return x;
}

static AvlNode *avl_rot_left(AvlNode *x) {
    AvlNode *y = x->right, *t2 = y->left;
    y->left = x;
    x->right = t2;
    avl_fix(x);
    avl_fix(y);
    return y;
}

static AvlNode *avl_insert(AvlNode *n, int key) {
    if (!n) {
        n = (AvlNode *)malloc(sizeof(AvlNode));
        if (!n) exit(1);
        n->key = key;
        n->h = 1;
        n->left = n->right = NULL;
        return n;
    }
    if (key < n->key) n->left = avl_insert(n->left, key);
    else if (key > n->key) n->right = avl_insert(n->right, key);
    else return n;

    avl_fix(n);
    int bf = avl_h(n->left) - avl_h(n->right);
    if (bf > 1 && key < n->left->key) return avl_rot_right(n);
    if (bf < -1 && key > n->right->key) return avl_rot_left(n);
    if (bf > 1) {
        n->left = avl_rot_left(n->left);
        return avl_rot_right(n);
    }
    if (bf < -1) {
        n->right = avl_rot_right(n->right);
        return avl_rot_left(n);
    }
    return n;
}

static int avl_search(const AvlNode *n, int key, long long *visited) {
    *visited = 0;
    while (n) {
        (*visited)++;
        if (key == n->key) return 1;
        n = (key < n->key) ? n->left : n->right;
    }
    return 0;
}

static void free_avl(AvlNode *n) {
    if (!n) return;
    free_avl(n->left);
    free_avl(n->right);
    free(n);
}

static void shuffle(int *arr, int len) {
    for (int i = len - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

int main(void) {
    const int N = 1000;
    const int ACCESSES = 10000;
    int hot_count = N / 20; /* 5% of products */
    if (hot_count < 1) hot_count = 1;

    int *keys = (int *)malloc(sizeof(int) * N);
    int *skewed_queries = (int *)malloc(sizeof(int) * ACCESSES);
    int *uniform_queries = (int *)malloc(sizeof(int) * ACCESSES);
    if (!keys || !skewed_queries || !uniform_queries) return 1;

    srand(2026);
    for (int i = 0; i < N; i++) keys[i] = i + 1;

    int *shuffled = (int *)malloc(sizeof(int) * N);
    if (!shuffled) return 1;
    for (int i = 0; i < N; i++) shuffled[i] = keys[i];
    shuffle(shuffled, N);

    SplayNode *sp_skew = NULL, *sp_unif = NULL, *sp_asc = NULL;
    AvlNode *avl = NULL;
    for (int i = 0; i < N; i++) {
        sp_skew = splay_insert(sp_skew, shuffled[i]);
        sp_unif = splay_insert(sp_unif, shuffled[i]);
        sp_asc  = splay_insert(sp_asc, keys[i]);
        avl     = avl_insert(avl, keys[i]);
    }
    free(shuffled);

    /* Generate 10,000 skewed queries: 80% to top 5% hot items, 20% to remaining 95% */
    for (int i = 0; i < ACCESSES; i++) {
        if ((rand() % 100) < 80) {
            skewed_queries[i] = keys[rand() % hot_count];
        } else {
            int rem = N - hot_count;
            skewed_queries[i] = keys[hot_count + (rem > 0 ? (rand() % rem) : 0)];
        }
        uniform_queries[i] = keys[rand() % N];
    }

    /* Skewed workload simulation + record root for first 10 accesses */
    long long sp_skew_total = 0, sp_skew_max = 0;
    long long avl_skew_total = 0, avl_skew_max = 0;

    printf("=== FIRST 10 ACCESSES (SKEWED WORKLOAD) ===\n");
    for (int i = 0; i < ACCESSES; i++) {
        long long v_sp = 0, v_avl = 0;
        splay_search(&sp_skew, skewed_queries[i], &v_sp);
        avl_search(avl, skewed_queries[i], &v_avl);

        sp_skew_total += v_sp;
        if (v_sp > sp_skew_max) sp_skew_max = v_sp;

        avl_skew_total += v_avl;
        if (v_avl > avl_skew_max) avl_skew_max = v_avl;

        if (i < 10) {
            printf("Access %2d: key=%4d | Splay visited=%2lld | New Root=%4d | AVL visited=%2lld\n",
                   i + 1, skewed_queries[i], v_sp, sp_skew ? sp_skew->key : -1, v_avl);
        }
    }

    /* Uniform workload simulation */
    long long sp_unif_total = 0, sp_unif_max = 0;
    long long avl_unif_total = 0, avl_unif_max = 0;
    for (int i = 0; i < ACCESSES; i++) {
        long long v_sp = 0, v_avl = 0;
        splay_search(&sp_unif, uniform_queries[i], &v_sp);
        avl_search(avl, uniform_queries[i], &v_avl);

        sp_unif_total += v_sp;
        if (v_sp > sp_unif_max) sp_unif_max = v_sp;

        avl_unif_total += v_avl;
        if (v_avl > avl_unif_max) avl_unif_max = v_avl;
    }

    /* Sorted order access test */
    long long sp_sorted_total = 0, sp_sorted_max = 0;
    for (int i = 0; i < N; i++) {
        long long v_sp = 0;
        splay_search(&sp_asc, keys[i], &v_sp);
        sp_sorted_total += v_sp;
        if (v_sp > sp_sorted_max) sp_sorted_max = v_sp;
    }

    printf("\n=== WORKLOAD SUMMARY (N=%d, Accesses=%d) ===\n", N, ACCESSES);
    printf("Skewed (80/5)  -> Splay Avg: %.2f (Max: %lld) | AVL Avg: %.2f (Max: %lld)\n",
           (double)sp_skew_total / ACCESSES, sp_skew_max,
           (double)avl_skew_total / ACCESSES, avl_skew_max);
    printf("Uniform        -> Splay Avg: %.2f (Max: %lld) | AVL Avg: %.2f (Max: %lld)\n",
           (double)sp_unif_total / ACCESSES, sp_unif_max,
           (double)avl_unif_total / ACCESSES, avl_unif_max);
    printf("Sorted Access  -> Splay Avg: %.2f (Max: %lld) over %d sequential lookups\n",
           (double)sp_sorted_total / N, sp_sorted_max, N);

    free_splay(sp_skew);
    free_splay(sp_unif);
    free_splay(sp_asc);
    free_avl(avl);
    free(keys);
    free(skewed_queries);
    free(uniform_queries);
    return 0;
}