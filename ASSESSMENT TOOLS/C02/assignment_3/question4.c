#include <stdio.h>
#include <stdlib.h>

typedef enum { RED = 0, BLACK = 1 } Color;

typedef struct RbNode {
    int key;
    Color color;
    struct RbNode *left, *right, *parent;
} RbNode;

typedef struct RbTree {
    RbNode *root;
    RbNode *leftmost;
    long long rotations;
} RbTree;

typedef struct AvlNode {
    int key, h;
    struct AvlNode *left, *right;
} AvlNode;

static long long avl_rotations = 0;

static RbNode *make_rb_node(int key, RbNode *parent) {
    RbNode *n = (RbNode *)malloc(sizeof(RbNode));
    if (!n) exit(1);
    n->key = key;
    n->color = RED;
    n->left = n->right = NULL;
    n->parent = parent;
    return n;
}

static RbTree *init_rb_tree(void) {
    RbTree *t = (RbTree *)malloc(sizeof(RbTree));
    if (!t) exit(1);
    t->root = NULL;
    t->leftmost = NULL;
    t->rotations = 0;
    return t;
}

static void sync_leftmost(RbTree *t) {
    RbNode *p = t->root;
    while (p && p->left) p = p->left;
    t->leftmost = p;
}

static int get_min_key(const RbTree *t, int *out) {
    if (!t || !t->leftmost) return 0;
    if (out) *out = t->leftmost->key;
    return 1;
}

static void rot_left_rb(RbTree *t, RbNode *x) {
    RbNode *y = x->right;
    if (!y) return;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) t->root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
    t->rotations++;
}

static void rot_right_rb(RbTree *t, RbNode *y) {
    RbNode *x = y->left;
    if (!x) return;
    y->left = x->right;
    if (x->right) x->right->parent = y;
    x->parent = y->parent;
    if (!y->parent) t->root = x;
    else if (y == y->parent->right) y->parent->right = x;
    else y->parent->left = x;
    x->right = y;
    y->parent = x;
    t->rotations++;
}

static void fix_insert_rb(RbTree *t, RbNode *z) {
    while (z->parent && z->parent->color == RED) {
        RbNode *gp = z->parent->parent;
        if (!gp) break;
        if (z->parent == gp->left) {
            RbNode *u = gp->right;
            if (u && u->color == RED) {
                z->parent->color = BLACK;
                u->color = BLACK;
                gp->color = RED;
                z = gp;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rot_left_rb(t, z);
                    gp = z->parent ? z->parent->parent : NULL;
                }
                if (z->parent && gp) {
                    z->parent->color = BLACK;
                    gp->color = RED;
                    rot_right_rb(t, gp);
                }
            }
        } else {
            RbNode *u = gp->left;
            if (u && u->color == RED) {
                z->parent->color = BLACK;
                u->color = BLACK;
                gp->color = RED;
                z = gp;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rot_right_rb(t, z);
                    gp = z->parent ? z->parent->parent : NULL;
                }
                if (z->parent && gp) {
                    z->parent->color = BLACK;
                    gp->color = RED;
                    rot_left_rb(t, gp);
                }
            }
        }
    }
    if (t->root) t->root->color = BLACK;
}

static void insert_rb(RbTree *t, int key) {
    RbNode *parent = NULL, *cur = t->root;
    while (cur) {
        parent = cur;
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return;
    }
    RbNode *z = make_rb_node(key, parent);
    if (!parent) t->root = z;
    else if (key < parent->key) parent->left = z;
    else parent->right = z;

    fix_insert_rb(t, z);
    sync_leftmost(t);
}

static RbNode *find_rb_node(RbNode *n, int key) {
    while (n && key != n->key) n = (key < n->key) ? n->left : n->right;
    return n;
}

static void splice_rb(RbTree *t, RbNode *u, RbNode *v) {
    if (!u->parent) t->root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

static RbNode *subtree_min_rb(RbNode *n) {
    while (n && n->left) n = n->left;
    return n;
}

static void fix_delete_rb(RbTree *t, RbNode *x, RbNode *xp) {
    while (x != t->root && (!x || x->color == BLACK)) {
        if (!xp) break;
        if (x == xp->left) {
            RbNode *w = xp->right;
            if (w && w->color == RED) {
                w->color = BLACK;
                xp->color = RED;
                rot_left_rb(t, xp);
                w = xp->right;
            }
            int lb = (!w || !w->left || w->left->color == BLACK);
            int rb = (!w || !w->right || w->right->color == BLACK);
            if (lb && rb) {
                if (w) w->color = RED;
                x = xp;
                xp = x->parent;
            } else {
                if (rb) {
                    if (w && w->left) w->left->color = BLACK;
                    if (w) {
                        w->color = RED;
                        rot_right_rb(t, w);
                        w = xp->right;
                    }
                }
                if (w) w->color = xp->color;
                xp->color = BLACK;
                if (w && w->right) w->right->color = BLACK;
                rot_left_rb(t, xp);
                x = t->root;
                xp = NULL;
            }
        } else {
            RbNode *w = xp->left;
            if (w && w->color == RED) {
                w->color = BLACK;
                xp->color = RED;
                rot_right_rb(t, xp);
                w = xp->left;
            }
            int lb = (!w || !w->left || w->left->color == BLACK);
            int rb = (!w || !w->right || w->right->color == BLACK);
            if (lb && rb) {
                if (w) w->color = RED;
                x = xp;
                xp = x->parent;
            } else {
                if (lb) {
                    if (w && w->right) w->right->color = BLACK;
                    if (w) {
                        w->color = RED;
                        rot_left_rb(t, w);
                        w = xp->left;
                    }
                }
                if (w) w->color = xp->color;
                xp->color = BLACK;
                if (w && w->left) w->left->color = BLACK;
                rot_right_rb(t, xp);
                x = t->root;
                xp = NULL;
            }
        }
    }
    if (x) x->color = BLACK;
}

static void delete_rb(RbTree *t, int key) {
    RbNode *z = find_rb_node(t->root, key);
    if (!z) return;

    RbNode *y = z;
    Color orig = y->color;
    RbNode *x = NULL, *xp = NULL;

    if (!z->left) {
        x = z->right;
        xp = z->parent;
        splice_rb(t, z, z->right);
    } else if (!z->right) {
        x = z->left;
        xp = z->parent;
        splice_rb(t, z, z->left);
    } else {
        y = subtree_min_rb(z->right);
        orig = y->color;
        x = y->right;
        if (y->parent == z) {
            xp = y;
        } else {
            xp = y->parent;
            splice_rb(t, y, y->right);
            y->right = z->right;
            if (y->right) y->right->parent = y;
        }
        splice_rb(t, z, y);
        y->left = z->left;
        if (y->left) y->left->parent = y;
        y->color = z->color;
    }

    free(z);
    if (orig == BLACK) fix_delete_rb(t, x, xp);
    sync_leftmost(t);
}

static int check_bh(RbNode *n, RbNode *exp_parent, int *ok) {
    if (!*ok) return -1;
    if (!n) return 1;
    if (n->parent != exp_parent) { *ok = 0; return -1; }
    if (n->color == RED) {
        if ((n->left && n->left->color == RED) || (n->right && n->right->color == RED)) {
            *ok = 0;
            return -1;
        }
    }
    int lbh = check_bh(n->left, n, ok);
    int rbh = check_bh(n->right, n, ok);
    if (!*ok || lbh != rbh) { *ok = 0; return -1; }
    return lbh + (n->color == BLACK ? 1 : 0);
}

static int valid_rb(const RbTree *t) {
    if (!t) return 0;
    if (!t->root) return 1;
    if (t->root->color != BLACK) return 0;
    int ok = 1;
    check_bh(t->root, NULL, &ok);
    return ok;
}

static int height_rb(const RbNode *n) {
    if (!n) return 0;
    int lh = height_rb(n->left), rh = height_rb(n->right);
    return 1 + (lh > rh ? lh : rh);
}

static void print_pre_rb(const RbNode *n) {
    if (!n) return;
    printf("%d(%c) ", n->key, n->color == RED ? 'R' : 'B');
    print_pre_rb(n->left);
    print_pre_rb(n->right);
}

static void free_rb_nodes(RbNode *n) {
    if (!n) return;
    free_rb_nodes(n->left);
    free_rb_nodes(n->right);
    free(n);
}

static void destroy_rb(RbTree *t) {
    if (!t) return;
    free_rb_nodes(t->root);
    free(t);
}

static int h_avl(const AvlNode *n) { return n ? n->h : 0; }
static int max2(int a, int b) { return a > b ? a : b; }
static void refresh_h(AvlNode *n) { n->h = 1 + max2(h_avl(n->left), h_avl(n->right)); }

static AvlNode *rot_right_avl(AvlNode *y) {
    AvlNode *x = y->left, *t2 = x->right;
    x->right = y;
    y->left = t2;
    refresh_h(y);
    refresh_h(x);
    avl_rotations++;
    return x;
}

static AvlNode *rot_left_avl(AvlNode *x) {
    AvlNode *y = x->right, *t2 = y->left;
    y->left = x;
    x->right = t2;
    refresh_h(x);
    refresh_h(y);
    avl_rotations++;
    return y;
}

static AvlNode *insert_avl(AvlNode *n, int key) {
    if (!n) {
        n = (AvlNode *)malloc(sizeof(AvlNode));
        if (!n) exit(1);
        n->key = key;
        n->h = 1;
        n->left = n->right = NULL;
        return n;
    }
    if (key < n->key) n->left = insert_avl(n->left, key);
    else if (key > n->key) n->right = insert_avl(n->right, key);
    else return n;

    refresh_h(n);
    int bf = h_avl(n->left) - h_avl(n->right);
    if (bf > 1 && key < n->left->key) return rot_right_avl(n);
    if (bf < -1 && key > n->right->key) return rot_left_avl(n);
    if (bf > 1) {
        n->left = rot_left_avl(n->left);
        return rot_right_avl(n);
    }
    if (bf < -1) {
        n->right = rot_right_avl(n->right);
        return rot_left_avl(n);
    }
    return n;
}

static AvlNode *min_avl(AvlNode *n) {
    while (n && n->left) n = n->left;
    return n;
}

static AvlNode *delete_avl(AvlNode *n, int key) {
    if (!n) return NULL;
    if (key < n->key) n->left = delete_avl(n->left, key);
    else if (key > n->key) n->right = delete_avl(n->right, key);
    else {
        if (!n->left || !n->right) {
            AvlNode *tmp = n->left ? n->left : n->right;
            free(n);
            return tmp;
        }
        AvlNode *succ = min_avl(n->right);
        n->key = succ->key;
        n->right = delete_avl(n->right, succ->key);
    }
    if (!n) return NULL;
    refresh_h(n);
    int bf = h_avl(n->left) - h_avl(n->right);
    if (bf > 1 && h_avl(n->left->left) >= h_avl(n->left->right)) return rot_right_avl(n);
    if (bf > 1) {
        n->left = rot_left_avl(n->left);
        return rot_right_avl(n);
    }
    if (bf < -1 && h_avl(n->right->right) >= h_avl(n->right->left)) return rot_left_avl(n);
    if (bf < -1) {
        n->right = rot_right_avl(n->right);
        return rot_left_avl(n);
    }
    return n;
}

static void free_avl(AvlNode *n) {
    if (!n) return;
    free_avl(n->left);
    free_avl(n->right);
    free(n);
}

static void permute(int *arr, int len) {
    for (int i = len - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

int main(void) {
    int seq[] = {10, 20, 30, 15, 25, 5};
    RbTree *sample = init_rb_tree();
    printf("--- RB Trace (10, 20, 30, 15, 25, 5) ---\n");
    for (int i = 0; i < 6; i++) {
        insert_rb(sample, seq[i]);
        int min_k = -1;
        get_min_key(sample, &min_k);
        printf("After %2d | OK=%d | Min=%2d | Pre: ", seq[i], valid_rb(sample), min_k);
        print_pre_rb(sample->root);
        printf("\n");
    }
    printf("Trace rotations: %lld\n\n", sample->rotations);
    destroy_rb(sample);

    const int N = 10000;
    int *ins_keys = (int *)malloc(sizeof(int) * N);
    int *del_keys = (int *)malloc(sizeof(int) * N);
    if (!ins_keys || !del_keys) return 1;

    srand(2026);
    for (int i = 0; i < N; i++) {
        ins_keys[i] = i + 1;
        del_keys[i] = i + 1;
    }
    permute(ins_keys, N);
    permute(del_keys, N);

    RbTree *rb = init_rb_tree();
    AvlNode *avl = NULL;
    avl_rotations = 0;
    int ok_all = 1;

    for (int i = 0; i < N; i++) {
        insert_rb(rb, ins_keys[i]);
        if (!valid_rb(rb)) ok_all = 0;
        avl = insert_avl(avl, ins_keys[i]);
    }
    long long rb_ins_r = rb->rotations;
    long long avl_ins_r = avl_rotations;
    int rb_h = height_rb(rb->root);
    int avl_h = h_avl(avl);

    for (int i = 0; i < N; i++) {
        delete_rb(rb, del_keys[i]);
        if (!valid_rb(rb)) ok_all = 0;
        avl = delete_avl(avl, del_keys[i]);
    }

    printf("--- 10,000 Random Insert + Delete ---\n");
    printf("RB  : H=%d | InsRot=%lld | TotalRot=%lld | Valid=%d\n",
           rb_h, rb_ins_r, rb->rotations, ok_all);
    printf("AVL : H=%d | InsRot=%lld | TotalRot=%lld\n",
           avl_h, avl_ins_r, avl_rotations);

    RbTree *rb_inc = init_rb_tree();
    AvlNode *avl_inc = NULL;
    avl_rotations = 0;
    int ok_inc = 1;

    for (int i = 1; i <= N; i++) {
        insert_rb(rb_inc, i);
        if (!valid_rb(rb_inc)) ok_inc = 0;
        avl_inc = insert_avl(avl_inc, i);
    }
    int inc_min = -1;
    get_min_key(rb_inc, &inc_min);

    printf("\n--- 10,000 Ascending Insert ---\n");
    printf("RB  : H=%d | Min=%d | Rot=%lld | Valid=%d\n",
           height_rb(rb_inc->root), inc_min, rb_inc->rotations, ok_inc);
    printf("AVL : H=%d | Rot=%lld\n", h_avl(avl_inc), avl_rotations);

    destroy_rb(rb);
    destroy_rb(rb_inc);
    free_avl(avl);
    free_avl(avl_inc);
    free(ins_keys);
    free(del_keys);
    return 0;
}