#include "rbtree.h"
#include <stdlib.h>

enum
{
    L = 0,
    R = 1
};

/* ==========아래 만든 함수 선언========== */

static void RB_insert_fixup(rbtree *t, node_t *z);
static void RB_rotate(rbtree *t, node_t *x, int dir);
static void RB_Transplant(rbtree *t, node_t *u, node_t *v);
static node_t *TreeMin(rbtree *t, node_t *x);
static void RB_delete_fixup(rbtree *t, node_t *x);

/* ==================================== */
rbtree *new_rbtree(void) // 임시완료
{
    // TODO: initialize struct if needed

    // printf("DEBUG: enter new_rbtree()\n");
    rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
    if (!p)
        return NULL;

    /* nil을 만들어야함*/
    node_t *NIL = calloc(1, sizeof(node_t));
    if (!NIL)
    {
        free(p);
        return NULL;
    }
    /* 초기화 */
    NIL->color = RBTREE_BLACK;
    NIL->ch[L] = NIL;
    NIL->ch[R] = NIL;
    NIL->parent = NIL;

    p->nil = NIL;
    p->root = NIL;

    return p;
}
/* ===================내가 추가한 함수====================*/
static void free_node(rbtree *t, node_t *x) // 재귀적으로 노드들 순회하면서 free 하는 함수, 임시완료
{
    if (x == t->nil)
        return;

    free_node(t, x->ch[L]);
    free_node(t, x->ch[R]);
    free(x);
}
/* =====================================================*/
void delete_rbtree(rbtree *t) // 임시완료
{
    // TODO: reclaim the tree nodes's memory
    /* 모든 노드 순회 하면서 free 하기*/
    free_node(t, t->root);
    free(t->nil);

    free(t);
}

node_t *rbtree_insert(rbtree *t, const key_t key) // 임시완료
{
    // TODO: implement insert

    node_t *z = calloc(1, sizeof(node_t));
    if (!z)
        return NULL;
    z->key = key;

    node_t *x = t->root;
    node_t *y = t->nil;

    while (x != t->nil) // 경계까지 내려가기
    {
        y = x; // y가 경계까지 내려가지면 끝
        if (z->key < x->key)
            x = x->ch[L];
        else
            x = x->ch[R];
    }

    z->parent = y;   // z를 y아래 삽입
    if (y == t->nil) // 루트이면
        t->root = z;
    else if (z->key < y->key)
        y->ch[L] = z;
    else
        y->ch[R] = z;

    z->ch[L] = t->nil;
    z->ch[R] = t->nil;
    z->color = RBTREE_RED;
    RB_insert_fixup(t, z);

    return z;
}

node_t *rbtree_find(const rbtree *t, const key_t key) // 임시완료
{
    // TODO: implement find
    node_t *x = t->root;

    while (x != t->nil)
    {
        if (key == x->key)
            return x;
        else if (key < x->key)
            x = x->ch[L];
        else
            x = x->ch[R];
    }

    return NULL;
}

node_t *rbtree_min(const rbtree *t)
{
    // TODO: implement find
    node_t *x = t->root;

    while (x->ch[L] != t->nil)
        x = x->ch[L];

    return x;
}

node_t *rbtree_max(const rbtree *t)
{
    // TODO: implement find
    node_t *x = t->root;
    while (x->ch[R] != t->nil)
        x = x->ch[R];

    return x;
}

int rbtree_erase(rbtree *t, node_t *z)
{
    // TODO: implement erase
    node_t *x;     //   삭제한 자리에 오는 노드가 되며, 이를 기준으로 Fixup 할거임
    node_t *y = z; //   후계자로 사용할 노드

    color_t y_original_color = y->color;
    if (z->ch[L] == t->nil) // (a)의 경우, 오른자식 승격
    {
        x = z->ch[R];
        RB_Transplant(t, z, z->ch[R]);
    }
    else if (z->ch[R] == t->nil) // (b)의 경우, 왼자식 승격
    {
        x = z->ch[L];
        RB_Transplant(t, z, z->ch[L]);
    }
    else
    {
        y = TreeMin(t, z->ch[R]);    // z의 후계자 확인
        y_original_color = y->color; // 삭제되는 색 업데이트(후계자의 색으로)
        x = y->ch[R];

        if (y != z->ch[R]) // (d)의 경우 y가 더 아래
        {
            RB_Transplant(t, y, y->ch[R]);
            y->ch[R] = z->ch[R];
            y->ch[R]->parent = y;
        }
        else // y가 z의 직계 자식인 경우
            x->parent = y;
        RB_Transplant(t, z, y);
        y->ch[L] = z->ch[L];
        y->ch[L]->parent = y;
        y->color = z->color;
    }
    if (y_original_color == RBTREE_BLACK)
        RB_delete_fixup(t, x);
    free(z);
    return 0;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n)
{
    // TODO: implement to_array

    /* arr을 트리 중위순회 한 값으로 채우기*/
    /* 재귀로 구현*/
    size_t index = 0;
    inorder(t, t->root, arr, &index, n);

    return 0;
}

/* ===================내가 추가한 함수들====================*/
void inorder(const rbtree *t, node_t *x, key_t *arr, size_t *idx, size_t n)
{
    if ((x == t->nil) || (*idx >= n))
        return;

    inorder(t, x->ch[L], arr, idx, n);
    arr[*idx] = x->key;
    (*idx)++;
    inorder(t, x->ch[R], arr, idx, n);
}
/* ===================내가 추가한 함수들====================*/

/* ===================내가 추가한 함수들====================*/

// dir은 x 기준 회전 방향
// dir = L 이면 left rotate, dir = R 이면 right rotate
static void RB_rotate(rbtree *t, node_t *x, int dir)
{
    int opp = 1 - dir;
    node_t *y = x->ch[opp];

    x->ch[opp] = y->ch[dir];
    if (y->ch[dir] != t->nil)
        y->ch[dir]->parent = x;

    y->parent = x->parent;
    if (x->parent == t->nil)
        t->root = y;
    else if (x == x->parent->ch[L])
        x->parent->ch[L] = y;
    else
        x->parent->ch[R] = y;
    y->ch[dir] = x;
    x->parent = y;
}

// 삽입 시 fixup하는 함수
static void RB_insert_fixup(rbtree *t, node_t *z) // 노드 색은 RED고 리프임
{
    while ((z->parent->color == RBTREE_RED) && (z != t->root))
    {
        int dir = (z->parent->parent->ch[L] == z->parent) ? L : R; // z의 부모의 방향 확인
        int opp = 1 - dir;
        node_t *w = z->parent->parent->ch[opp]; // 부모 반대가 삼촌이 된다

        if (w->color == RBTREE_RED) // CASE1
        {
            z->parent->color = RBTREE_BLACK;
            w->color = RBTREE_BLACK;
            z->parent->parent->color = RBTREE_RED;
            z = z->parent->parent;
        }
        else
        {
            if (z->parent->ch[opp] == z) // CASE2
            {
                z = z->parent;
                RB_rotate(t, z, dir);
            }
            z->parent->color = RBTREE_BLACK; // CASE3
            z->parent->parent->color = RBTREE_RED;
            RB_rotate(t, z->parent->parent, opp);
        }
    }
    t->root->color = RBTREE_BLACK;
}

// 삭제하려는 노드 u의 위치에 v를 대신 붙이는 함수
static void RB_Transplant(rbtree *t, node_t *u, node_t *v)
{
    if (u->parent == t->nil) // u가 루트 노드인 경우
        t->root = v;
    else if (u == u->parent->ch[L]) // u가 왼쪽 자식인 경우
        u->parent->ch[L] = v;
    else // u가 오른쪽 자식인 경우
        u->parent->ch[R] = v;
    v->parent = u->parent; // v의 부모를 u의 부모로 설정
}

node_t *TreeMin(rbtree *t, node_t *x)
{
    while (x->ch[L] != t->nil)
        x = x->ch[L];

    return x;
}

static void RB_delete_fixup(rbtree *t, node_t *x)
{
    while ((x != t->root) && (x->color == RBTREE_BLACK))
    {
        int dir = (x->parent->ch[L] == x) ? L : R; // x의 방향 확인
        int opp = 1 - dir;
        node_t *w = x->parent->ch[opp]; // 형제 노드

        if (w->color == RBTREE_RED) // CASE1
        {
            w->color = RBTREE_BLACK;
            x->parent->color = RBTREE_RED;
            RB_rotate(t, x->parent, dir);
            w = x->parent->ch[opp];
        }

        if ((w->ch[L]->color == RBTREE_BLACK) && (w->ch[R]->color == RBTREE_BLACK)) // CASE2
        {
            w->color = RBTREE_RED;
            x = x->parent;
        }
        else
        {
            if (w->ch[opp]->color == RBTREE_BLACK) // CASE3
            {
                w->ch[dir]->color = RBTREE_BLACK;
                w->color = RBTREE_RED;
                RB_rotate(t, w, opp);
                w = x->parent->ch[opp];
            }
            // CASE4
            w->color = x->parent->color;
            x->parent->color = RBTREE_BLACK;
            w->ch[opp]->color = RBTREE_BLACK;
            RB_rotate(t, x->parent, dir);
            x = t->root;
        }
    }
    x->color = RBTREE_BLACK;
}

// /* =====================================================*/

/* 리뷰한 코드 */

// #include "rbtree.h"

// #include <stdlib.h>

// node_t *new_node(color_t color, const key_t key, node_t *parent, node_t *left, node_t *right)
// {
//     node_t *node;

//     node = (node_t *)calloc(1, sizeof(node_t));

//     if (!node)
//         return (NULL);

//     node->color = color;
//     node->key = key;
//     node->parent = parent;
//     node->left = left;
//     node->right = right;
//     return (node);
// }

// rbtree *new_rbtree(void)
// {
//     // init할 때 nilNode를 할당해서 이게 root, nil 모두 되게 구현
//     rbtree *newTree;
//     node_t *nilNode;

//     newTree = (rbtree *)calloc(1, sizeof(rbtree));
//     if (!newTree)
//         return (NULL);

//     nilNode = new_node(RBTREE_BLACK, -1, NULL, NULL, NULL);
//     if (!nilNode)
//     {
//         free(newTree);
//         return (NULL);
//     }
//     nilNode->parent = nilNode; // 다른 연산에서 세그폴트 방지를 위해
//     nilNode->left = nilNode;
//     nilNode->right = nilNode;

//     newTree->nil = nilNode;
//     newTree->root = nilNode;

//     return (newTree);
// }

// void recursiveDelete(node_t **node, node_t *nil)
// {
//     if (!node || !(*node) || *node == nil)
//         return;

//     recursiveDelete(&(*node)->left, nil);
//     recursiveDelete(&(*node)->right, nil);
//     free(*node);
//     *node = NULL;
// }

// void delete_rbtree(rbtree *t)
// {
//     if (!t)
//         return;

//     if (t->root && t->nil)
//     {
//         recursiveDelete(&t->root, t->nil);
//         free(t->nil);
//         t->nil = NULL;
//     }
//     free(t);
// }

// /*
// '####:'##::: ##::'######::'########:'########::'########:
// . ##:: ###:: ##:'##... ##: ##.....:: ##.... ##:... ##..::
// : ##:: ####: ##: ##:::..:: ##::::::: ##:::: ##:::: ##::::
// : ##:: ## ## ##:. ######:: ######::: ########::::: ##::::
// : ##:: ##. ####::..... ##: ##...:::: ##.. ##:::::: ##::::
// : ##:: ##:. ###:'##::: ##: ##::::::: ##::. ##::::: ##::::
// '####: ##::. ##:. ######:: ########: ##:::. ##:::: ##::::
// ....::..::::..:::......:::........::..:::::..:::::..:::::

// */

// node_t *bst_insert(rbtree *t, const key_t key)
// {
//     node_t *cur;
//     node_t *newNodeParent;

//     if (t->root == t->nil)
//     {
//         cur = new_node(RBTREE_BLACK, key, t->nil, t->nil, t->nil);
//         if (!cur)
//             return (NULL);
//         t->root = cur;
//         return (cur);
//     }

//     cur = t->root;
//     newNodeParent = NULL;
//     while (cur != t->nil) // 삽입할 리프노드 찾기 - 무조건 리프노드에 삽입
//     {
//         newNodeParent = cur;
//         if (key < cur->key)
//             cur = cur->left;
//         else
//             cur = cur->right;
//     }
//     // newNodeParent -> null이 될 수 없음. 위에서 한번 거름
//     cur = new_node(RBTREE_RED, key, newNodeParent, t->nil, t->nil);
//     if (!cur)
//         return (NULL);

//     if (cur->key < newNodeParent->key)
//         newNodeParent->left = cur;
//     else
//         newNodeParent->right = cur;

//     return (cur);
// }

// void tree_right_rotation(rbtree *t, node_t *root)
// { // 더블포인터 넘겨서 변수 자체를 바꿀 필요 X - 연결만 바꿔주면 됨
//     node_t *oldRoot;
//     node_t *pivot;
//     node_t *pivotRight;

//     if (!t || !root || root == t->nil || root->left == t->nil)
//         return;

//     oldRoot = root;
//     pivot = root->left;
//     pivotRight = pivot->right;

//     pivot->parent = oldRoot->parent;
//     if (root == t->root)
//         t->root = pivot;
//     else
//     {
//         if (oldRoot->parent->left == oldRoot)
//             oldRoot->parent->left = pivot;
//         else
//             oldRoot->parent->right = pivot;
//     }

//     pivot->right = oldRoot;
//     oldRoot->parent = pivot;

//     oldRoot->left = pivotRight;
//     if (pivotRight != t->nil)
//         pivotRight->parent = oldRoot;
// }

// void tree_left_rotation(rbtree *t, node_t *root)
// {
//     node_t *oldRoot;
//     node_t *pivot;
//     node_t *pivotLeft;

//     if (!t || !root || root == t->nil || root->right == t->nil)
//         return;

//     oldRoot = root;
//     pivot = root->right;
//     pivotLeft = pivot->left;

//     pivot->parent = oldRoot->parent;
//     if (root == t->root)
//         t->root = pivot;
//     else
//     {
//         if (oldRoot->parent->left == oldRoot)
//             oldRoot->parent->left = pivot;
//         else
//             oldRoot->parent->right = pivot;
//     }

//     pivot->left = oldRoot;
//     oldRoot->parent = pivot;

//     oldRoot->right = pivotLeft;
//     if (pivotLeft != t->nil)
//         pivotLeft->parent = oldRoot;
// }

// void insert_fixup(rbtree *t, node_t *node)
// {
//     node_t *cur;
//     node_t *parentSibling;

//     if (!t || !node)
//         return;

//     cur = node;
//     while (cur->parent->color == RBTREE_RED) // root->parent = nil이기 때문에 segfault 안남
//     {
//         if (cur->parent == cur->parent->parent->left) // 왼쪽 서브트리인 경우
//         {
//             parentSibling = cur->parent->parent->right;
//             if (parentSibling->color == RBTREE_RED)
//             { // 색을 바꿔도 블랙 깊이는 변하지 않기에 변경 가능 -> cur 갱신해서 계속 확인 & 색 변경
//                 cur->parent->color = RBTREE_BLACK;
//                 parentSibling->color = RBTREE_BLACK;
//                 cur->parent->parent->color = RBTREE_RED;
//                 cur = cur->parent->parent;
//             }
//             else
//             {
//                 if (cur == cur->parent->right)
//                 {
//                     cur = cur->parent;
//                     tree_left_rotation(t, cur);
//                 }
//                 cur->parent->color = RBTREE_BLACK;
//                 cur->parent->parent->color = RBTREE_RED;
//                 tree_right_rotation(t, cur->parent->parent);
//             }
//         }
//         else // 오른쪽 서브트리인 경우
//         {
//             parentSibling = cur->parent->parent->left;
//             if (parentSibling->color == RBTREE_RED)
//             {
//                 cur->parent->color = RBTREE_BLACK;
//                 parentSibling->color = RBTREE_BLACK;
//                 cur->parent->parent->color = RBTREE_RED;
//                 cur = cur->parent->parent;
//             }
//             else
//             {
//                 if (cur == cur->parent->left)
//                 {
//                     cur = cur->parent;
//                     tree_right_rotation(t, cur);
//                 }
//                 cur->parent->color = RBTREE_BLACK;
//                 cur->parent->parent->color = RBTREE_RED;
//                 tree_left_rotation(t, cur->parent->parent);
//             }
//         }
//     }

//     t->root->color = RBTREE_BLACK;
// }

// node_t *rbtree_insert(rbtree *t, const key_t key)
// {
//     // BST 삽입 -> fix
//     // left rotation, right rotation 함수 구현
//     // case study
//     node_t *newNode;

//     if (!t)
//         return (NULL);

//     newNode = bst_insert(t, key);
//     if (!newNode)
//         return (NULL);

//     insert_fixup(t, newNode);

//     return (newNode);
// }

// node_t *rbtree_find(const rbtree *t, const key_t key)
// { // 못찾으면 NULL 리턴, 찾으면 해당 노드 반환
//     node_t *cur;

//     if (!t || t->root == t->nil)
//         return (NULL);

//     cur = t->root;
//     while (cur != t->nil)
//     {
//         if (cur->key == key)
//             return (cur);
//         else if (cur->key < key)
//             cur = cur->right;
//         else
//             cur = cur->left;
//     }
//     return (NULL);
// }

// node_t *rbtree_min(const rbtree *t)
// {
//     node_t *minNode;

//     if (!t || t->root == t->nil)
//         return (NULL);

//     minNode = t->root;
//     while (minNode->left != t->nil)
//         minNode = minNode->left;

//     return (minNode);
// }

// node_t *rbtree_max(const rbtree *t)
// {
//     node_t *maxNode;

//     if (!t || t->root == t->nil)
//         return (NULL);

//     maxNode = t->root;
//     while (maxNode->right != t->nil)
//         maxNode = maxNode->right;

//     return (maxNode);
// }

// /*
// '########:'########:::::'###:::::'######::'########:
//  ##.....:: ##.... ##:::'## ##:::'##... ##: ##.....::
//  ##::::::: ##:::: ##::'##:. ##:: ##:::..:: ##:::::::
//  ######::: ########::'##:::. ##:. ######:: ######:::
//  ##...:::: ##.. ##::: #########::..... ##: ##...::::
//  ##::::::: ##::. ##:: ##.... ##:'##::: ##: ##:::::::
//  ########: ##:::. ##: ##:::: ##:. ######:: ########:
// ........::..:::::..::..:::::..:::......:::........::
// */

// void rbtree_transplant(rbtree *t, node_t *u, node_t *v)
// {
//     // 트리 t내에서 u의 자리를 v로 대체하는 함수
//     // 모든 상황에서 쓰이는 함수 X
//     // 자식 노드가 1개인 상황에서 교체하는 경우에만 사용되는 함수
//     if (!t || !u || !v)
//         return;

//     if (u->parent == t->nil) // root라는 의미
//         t->root = v;         // root
//     else if (u == u->parent->left)
//         u->parent->left = v;
//     else
//         u->parent->right = v;

//     v->parent = u->parent;
// }

// node_t *subtree_min(rbtree *t, node_t *subRoot)
// {
//     // util함수
//     node_t *minNode;

//     if (!t || !subRoot || subRoot == t->nil)
//         return (NULL);

//     minNode = subRoot;
//     while (minNode->left != t->nil)
//         minNode = minNode->left;

//     return (minNode);
// }

// void delete_fixup(rbtree *t, node_t *target)
// {
//     node_t *sibling;

//     if (!t || !target)
//         return;

//     while (target != t->root && target->color == RBTREE_BLACK)
//     {
//         if (target == target->parent->left)
//         {
//             sibling = target->parent->right;

//             if (sibling->color == RBTREE_RED)
//             {
//                 sibling->color = RBTREE_BLACK;
//                 target->parent->color = RBTREE_RED;
//                 tree_left_rotation(t, target->parent);
//                 sibling = target->parent->right;
//             }

//             if (sibling->left->color == RBTREE_BLACK && sibling->right->color == RBTREE_BLACK)
//             {
//                 if (sibling != t->nil)
//                     sibling->color = RBTREE_RED;
//                 target = target->parent;
//             }
//             else
//             {
//                 if (sibling->right->color == RBTREE_BLACK)
//                 {
//                     sibling->left->color = RBTREE_BLACK;
//                     sibling->color = RBTREE_RED;
//                     tree_right_rotation(t, sibling);
//                     sibling = target->parent->right;
//                 }
//                 sibling->color = target->parent->color;
//                 target->parent->color = RBTREE_BLACK;
//                 sibling->right->color = RBTREE_BLACK;
//                 tree_left_rotation(t, target->parent);
//                 target = t->root;
//             }
//         }
//         else
//         {
//             sibling = target->parent->left;
//             if (sibling->color == RBTREE_RED)
//             {
//                 sibling->color = RBTREE_BLACK;
//                 target->parent->color = RBTREE_RED;
//                 tree_right_rotation(t, target->parent);
//                 sibling = target->parent->left;
//             }

//             if (sibling->right->color == RBTREE_BLACK && sibling->left->color == RBTREE_BLACK)
//             {
//                 if (sibling != t->nil)
//                     sibling->color = RBTREE_RED;
//                 target = target->parent;
//             }
//             else
//             {
//                 if (sibling->left->color == RBTREE_BLACK)
//                 {
//                     sibling->right->color = RBTREE_BLACK;
//                     sibling->color = RBTREE_RED;
//                     tree_left_rotation(t, sibling);
//                     sibling = target->parent->left;
//                 }
//                 sibling->color = target->parent->color;
//                 target->parent->color = RBTREE_BLACK;
//                 sibling->left->color = RBTREE_BLACK;
//                 tree_right_rotation(t, target->parent);
//                 target = t->root;
//             }
//         }
//     }
//     target->color = RBTREE_BLACK;
// }

// int rbtree_erase(rbtree *t, node_t *p)
// {
//     node_t *nodeToReplace;
//     node_t *replacementChild;
//     color_t deletedNodeColor;

//     if (!t || !p)
//         return (-1); // -1 리턴이 맞는가?

//     nodeToReplace = p;
//     deletedNodeColor = nodeToReplace->color;

//     if (p->left == t->nil)
//     {
//         replacementChild = p->right;
//         rbtree_transplant(t, p, p->right);
//     }
//     else if (p->right == t->nil)
//     {
//         replacementChild = p->left;
//         rbtree_transplant(t, p, p->left);
//     }
//     else
//     {
//         nodeToReplace = subtree_min(t, nodeToReplace->right);
//         if (!nodeToReplace)
//             return (-1);
//         deletedNodeColor = nodeToReplace->color;
//         replacementChild = nodeToReplace->right;

//         if (nodeToReplace != p->right)
//         {
//             rbtree_transplant(t, nodeToReplace, nodeToReplace->right);
//             nodeToReplace->right = p->right;
//             nodeToReplace->right->parent = nodeToReplace;
//         }
//         else
//             replacementChild->parent = nodeToReplace;

//         rbtree_transplant(t, p, nodeToReplace);
//         nodeToReplace->left = p->left;
//         nodeToReplace->left->parent = nodeToReplace; // 통일성을 위해 nil 노드여도 함
//         nodeToReplace->color = p->color;
//     }

//     if (deletedNodeColor == RBTREE_BLACK)
//         delete_fixup(t, replacementChild);

//     free(p);

//     t->nil->parent = t->nil; // 전체 동작에 필요하진 않으나 논리구조 유지를 위해 원상복구
//     return (0);
// }

// void recursiveInorder(const rbtree *t, key_t *arr, const size_t n, const node_t *node, size_t *idx)
// {
//     if (!t || !arr || n == 0 || !node || node == t->nil || *idx >= n)
//         return;

//     recursiveInorder(t, arr, n, node->left, idx);
//     if (*idx < n)
//         arr[(*idx)++] = node->key;
//     else
//         return;
//     recursiveInorder(t, arr, n, node->right, idx);
// }

// int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n)
// { // 실패시 0 리턴, 아니면 삽입 개수 리턴
//     size_t idx;

//     if (!t || !arr || n == 0)
//         return 0;

//     idx = 0;
//     recursiveInorder(t, arr, n, t->root, &idx);
//     return (int)idx;
// }