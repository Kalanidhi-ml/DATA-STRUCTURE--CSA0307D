#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* B-Tree of minimum degree t = 2 (max keys = 2t - 1 = 3, max children = 2t = 4) */
#define T 2
#define MAX_KEYS (2 * T - 1)
#define MAX_CHILDREN (2 * T)

typedef struct BTreeNode {
    int n;
    int leaf;
    int keys[MAX_KEYS];
    struct BTreeNode *c[MAX_CHILDREN];
} BTreeNode;

BTreeNode *btree_create_node(int leaf) {
    BTreeNode *node = (BTreeNode *)malloc(sizeof(BTreeNode));
    if (!node) exit(1);
    node->n = 0;
    node->leaf = leaf;
    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->c[i] = NULL;
    }
    return node;
}

/* Splits full child y (at index i) of non-full parent x */
void btree_split_child(BTreeNode *x, int i, BTreeNode *y) {
    BTreeNode *z = btree_create_node(y->leaf);
    z->n = T - 1;

    for (int j = 0; j < T - 1; j++) {
        z->keys[j] = y->keys[j + T];
    }

    if (!y->leaf) {
        for (int j = 0; j < T; j++) {
            z->c[j] = y->c[j + T];
        }
    }

    y->n = T - 1;

    for (int j = x->n; j >= i + 1; j--) {
        x->c[j + 1] = x->c[j];
    }
    x->c[i + 1] = z;

    for (int j = x->n - 1; j >= i; j--) {
        x->keys[j + 1] = x->keys[j];
    }
    x->keys[i] = y->keys[T - 1];
    x->n++;
}

void btree_insert_nonfull(BTreeNode *x, int k) {
    int i = x->n - 1;
    if (x->leaf) {
        while (i >= 0 && k < x->keys[i]) {
            x->keys[i + 1] = x->keys[i];
            i--;
        }
        x->keys[i + 1] = k;
        x->n++;
    } else {
        while (i >= 0 && k < x->keys[i]) {
            i--;
        }
        i++;
        if (x->c[i]->n == MAX_KEYS) {
            btree_split_child(x, i, x->c[i]);
            if (k > x->keys[i]) {
                i++;
            }
        }
        btree_insert_nonfull(x->c[i], k);
    }
}

BTreeNode *btree_insert(BTreeNode *root, int k) {
    if (!root) {
        root = btree_create_node(1);
        root->keys[0] = k;
        root->n = 1;
        return root;
    }
    if (root->n == MAX_KEYS) {
        BTreeNode *s = btree_create_node(0);
        s->c[0] = root;
        btree_split_child(s, 0, root);
        int i = (s->keys[0] < k) ? 1 : 0;
        btree_insert_nonfull(s->c[i], k);
        return s;
    } else {
        btree_insert_nonfull(root, k);
        return root;
    }
}

/* B-Tree Search returning 1 if found, 0 otherwise, and counting disk reads (node visits) */
int btree_search(const BTreeNode *root, int k, int *disk_reads) {
    *disk_reads = 0;
    const BTreeNode *curr = root;
    while (curr != NULL) {
        (*disk_reads)++;
        int i = 0;
        while (i < curr->n && k > curr->keys[i]) {
            i++;
        }
        if (i < curr->n && k == curr->keys[i]) {
            return 1;
        }
        if (curr->leaf) {
            return 0;
        }
        curr = curr->c[i];
    }
    return 0;
}

void print_level(const BTreeNode *node, int level, int current) {
    if (!node) return;
    if (level == current) {
        printf("[");
        for (int i = 0; i < node->n; i++) {
            printf("%d%s", node->keys[i], (i < node->n - 1) ? " " : "");
        }
        printf("] ");
    } else if (!node->leaf) {
        for (int i = 0; i <= node->n; i++) {
            print_level(node->c[i], level, current + 1);
        }
    }
}

int btree_height_levels(const BTreeNode *node) {
    int h = 0;
    const BTreeNode *curr = node;
    while (curr) {
        h++;
        curr = curr->leaf ? NULL : curr->c[0];
    }
    return h;
}

void print_btree(const BTreeNode *root) {
    int levels = btree_height_levels(root);
    for (int l = 0; l < levels; l++) {
        printf("L%d: ", l);
        print_level(root, l, 0);
    }
    printf("\n");
}

void btree_free(BTreeNode *node) {
    if (!node) return;
    if (!node->leaf) {
        for (int i = 0; i <= node->n; i++) {
            btree_free(node->c[i]);
        }
    }
    free(node);
}

/* Analytical comparison for 5,000,000 keys and 10 ms per disk read */
void analyze_5m_records(const BTreeNode *sample_tree) {
    const double N = 5000000.0;
    const double read_ms = 10.0;

    /* Worst-case degenerate BST height (nodes visited ~ N) */
    double bst_reads = N;

    /* Balanced AVL tree height: ~1.44 * log2(N + 1) */
    double avl_reads = ceil(1.44 * (log(N + 1.0) / log(2.0)));

    /* Order-100 B-tree (max children m = 100): height ~ ceil(log_m(N)) or using degree t=50 */
    double btree100_height_log100 = ceil(log(N) / log(100.0));
    double btree100_reads = btree100_height_log100;

    printf("\n=== 5,000,000 PATIENT RECORDS: DISK READS & SEARCH TIME (10 ms / read) ===\n");
    printf("Skewed BST (worst-case) : ~%.0f reads -> %.2f ms (%.2f s)\n",
           bst_reads, bst_reads * read_ms, (bst_reads * read_ms) / 1000.0);
    printf("AVL Tree (1 node/block) : ~%.0f reads -> %.2f ms\n",
           avl_reads, avl_reads * read_ms);
    printf("B-Tree (order 100)      : ~%.0f reads (height ~%.0f) -> %.2f ms\n",
           btree100_reads, btree100_height_log100, btree100_reads * read_ms);

    int reads = 0;
    btree_search(sample_tree, 5, &reads);
    printf("\nSample B-tree (t=2) search for smallest (5): %d disk reads (%.2f ms)\n",
           reads, reads * read_ms);
    btree_search(sample_tree, 30, &reads);
    printf("Sample B-tree (t=2) search for largest (30): %d disk reads (%.2f ms)\n",
           reads, reads * read_ms);
    btree_search(sample_tree, 99, &reads);
    printf("Sample B-tree (t=2) search for missing (99): %d disk reads (%.2f ms)\n",
           reads, reads * read_ms);
}

int main(void) {
    int keys[] = {10, 20, 5, 6, 12, 30, 7, 17};
    int count = (int)(sizeof(keys) / sizeof(keys[0]));
    BTreeNode *root = NULL;

    printf("=== B-TREE INSERTIONS (t = 2, order 4) ===\n");
    for (int i = 0; i < count; i++) {
        root = btree_insert(root, keys[i]);
        printf("After insert %2d -> ", keys[i]);
        print_btree(root);
    }

    int reads = 0;
    int test_keys[] = {5, 30, 17, 99};
    int test_count = (int)(sizeof(test_keys) / sizeof(test_keys[0]));

    printf("\n=== B-TREE SEARCH SIMULATION ===\n");
    for (int i = 0; i < test_count; i++) {
        int found = btree_search(root, test_keys[i], &reads);
        printf("Search %2d: %s | Disk reads = %d | Est. time (10ms/read) = %d ms\n",
               test_keys[i],
               found ? "FOUND" : "NOT FOUND",
               reads,
               reads * 10);
    }

    analyze_5m_records(root);

    btree_free(root);
    return 0;
}