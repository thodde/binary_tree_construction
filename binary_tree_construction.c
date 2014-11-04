#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node node;

struct node {
    int data;
    struct node* left;
    struct node* right;
    struct node* parent;
};

// used printing tree in ascii
typedef struct asciinode_struct asciinode;

struct asciinode_struct {
    asciinode* left;
    asciinode* right;

    // length of the edge from this node to its children
    int edge_length;
    int height;
    int lablen;

    // -1 = left, 0 = root, 1 = right
    int parent_dir;

    // max supported uint32 in dec, 10 digits max
    char label[11];
};

// vars for printing the tree
#define MAX_HEIGHT 1000
int lprofile[MAX_HEIGHT];
int rprofile[MAX_HEIGHT];
#define INFINITY (1<<20)

// adjust gap between left and right nodes
int gap = 3;

// used for printing siblings,
// this is the x coordinate of the next char printed
int print_next;

// prototypes for tree construction functions
node* build_tree(int[], int, int, int[], int, int);
int search(int[], int, int, int);
node* new_node(int);
void print_inorder(node*);
void print_postorder(node*);

// prototypes for ascii tree functions
int MIN (int, int);
int MAX (int, int);
asciinode* build_ascii_tree_recursive(node*);
asciinode* build_ascii_tree(node*);
void free_ascii_tree(asciinode*);
void compute_lprofile(asciinode*, int, int);
void compute_rprofile(asciinode*, int, int);
void compute_edge_lengths(asciinode*);
void print_level(asciinode*, int, int);
void print_ascii_tree(node*);

// Driver program to test functions
int main() {
    int in[] = { 9, 3, 1, 0, 4, 2, 7, 6, 8, 5 };
    int post[] = { 9, 1, 4, 0, 3, 6, 7, 5, 8, 2 };

    int len = sizeof(in)/sizeof(in[0]);
    node* root = build_tree(in, 0, len - 1, post, 0, len - 1);

    // test the built tree by printing In order traversal
    printf("\nInorder traversal of the constructed tree is: \n");
    print_inorder(root);
    printf("\n");
    printf("\nPostorder traversal of the constructed tree is: \n");
    print_postorder(root);
    printf("\n");
    printf("\n");
    printf("Ascii Representation of Tree:\n");

    print_ascii_tree(root);

    return 0;
}

/* Recursive function to construct binary tree of size n from
 *  in-order traversal in[] and post-order traversal post[].  Initial values
 *  of inStart and inEnd should be 0 and n - 1.  The function doesn't
 *  do any error checking for cases where in-order and post-order
 *  do not form a tree
 */
node* build_tree(int in[], int inStart, int inEnd,
                 int post[], int postStart, int postEnd) {
    if(inStart > inEnd || postStart > postEnd)
        return NULL;

    int rootValue = post[postEnd];
    node *tNode = new_node(rootValue);

    // find the index of this node in in-order traversal
    int inIndex = search(in, inStart, inEnd, rootValue);

    // Using index in in-order traversal, construct left and right subtrees
    tNode->left = build_tree(in, inStart, inIndex-1, post, postStart, postStart + inIndex - (inStart + 1));
    tNode->right = build_tree(in, inIndex+1, inEnd, post, postStart + inIndex - inStart, postEnd - 1);

    return tNode;
}

// Function to find index of value in arr[start...end]
// The function assumes that value is present in in[]
int search(int arr[], int start, int end, int value) {
    int i;
    for(i = start; i < end; i++) {
        if(arr[i] == value)
            return i;
    }

    return i;
}

// function that allocates a new node with the
// given data and NULL left and right pointers
node* new_node(int data) {
    node* n = (node*)malloc(sizeof(node));
    n->data = data;
    n->left = NULL;
    n->right = NULL;

    return n;
}

// This function is here just to test build_tree()
void print_inorder(node* n) {
    if (n == NULL)
        return;

    // first print left child
    print_inorder(n->left);

    // then print the data of node
    printf("%d ", n->data);

    // now print right child
    print_inorder(n->right);
}

// This function is here just to test build_tree()
void print_postorder(node* n) {
    if (n == NULL)
        return;

    // first print left child
    print_postorder(n->left);

    // now print right child
    print_postorder(n->right);

    // then print the data of node
    printf("%d ", n->data);
}

// ******************************************
// All the functions below are related to
// printing the tree in ascii form to prove
// its correctness.
//
// Some of the ascii code borrowed from:
// http://ideone.com/wrY8Vo
// ******************************************
int MIN (int X, int Y) {
    return ((X) < (Y)) ? (X) : (Y);
}

int MAX (int X, int Y) {
    return ((X) > (Y)) ? (X) : (Y);
}

// given a node, recursively build the subtree on the
// left and right of the node.
asciinode * build_ascii_tree_recursive(node* t) {
    asciinode* node;

    if (t == NULL) return NULL;

    node = malloc(sizeof(asciinode));
    node->left = build_ascii_tree_recursive(t->left);
    node->right = build_ascii_tree_recursive(t->right);

    if (node->left != NULL) {
        node->left->parent_dir = -1;
    }

    if (node->right != NULL) {
        node->right->parent_dir = 1;
    }

    sprintf(node->label, "%d", t->data);
    node->lablen = strlen(node->label);

    return node;
}

// copy the tree into the ascii node structure
asciinode* build_ascii_tree(node* t) {
    asciinode* node;
    if (t == NULL) return NULL;
    node = build_ascii_tree_recursive(t);
    node->parent_dir = 0;
    return node;
}

// free all the nodes of the given tree
void free_ascii_tree(asciinode *node) {
    if (node == NULL) return;
    free_ascii_tree(node->left);
    free_ascii_tree(node->right);
    free(node);
}

// The following function fills in the lprofile array for the given tree.
// It assumes that the center of the label of the root of this tree
// is located at a position (x,y).  It assumes that the edge_length
// fields have been computed for this tree.
void compute_lprofile(asciinode *node, int x, int y) {
    int i, isleft;
    if (node == NULL) return;

    isleft = (node->parent_dir == -1);
    lprofile[y] = MIN(lprofile[y], x-((node->lablen-isleft)/2));

    if (node->left != NULL) {
        for (i=1; i <= node->edge_length && y+i < MAX_HEIGHT; i++) {
            lprofile[y+i] = MIN(lprofile[y+i], x-i);
        }
    }

    compute_lprofile(node->left, x-node->edge_length-1, y+node->edge_length+1);
    compute_lprofile(node->right, x+node->edge_length+1, y+node->edge_length+1);
}

void compute_rprofile(asciinode *node, int x, int y) {
    int i;
    int notleft;
    if (node == NULL) return;

    notleft = (node->parent_dir != -1);
    rprofile[y] = MAX(rprofile[y], x+((node->lablen-notleft)/2));

    if (node->right != NULL) {
        for (i=1; i <= node->edge_length && y+i < MAX_HEIGHT; i++) {
            rprofile[y+i] = MAX(rprofile[y+i], x+i);
        }
    }

    compute_rprofile(node->left, x-node->edge_length-1, y+node->edge_length+1);
    compute_rprofile(node->right, x+node->edge_length+1, y+node->edge_length+1);
}

// This function fills in the edge_length and
// height fields of the specified tree
void compute_edge_lengths(asciinode *node) {
    int h, hmin, i, delta;
    if (node == NULL) return;
    compute_edge_lengths(node->left);
    compute_edge_lengths(node->right);

    /* first fill in the edge_length of node */
    if (node->right == NULL && node->left == NULL) {
        node->edge_length = 0;
    }
    else {
        if (node->left != NULL) {
            for (i=0; i<node->left->height && i < MAX_HEIGHT; i++) {
                rprofile[i] = -INFINITY;
            }
            compute_rprofile(node->left, 0, 0);
            hmin = node->left->height;
        }
        else {
            hmin = 0;
        }
        if (node->right != NULL) {
            for (i=0; i<node->right->height && i < MAX_HEIGHT; i++) {
                lprofile[i] = INFINITY;
            }
            compute_lprofile(node->right, 0, 0);
            hmin = MIN(node->right->height, hmin);
        }
        else {
            hmin = 0;
        }

        delta = 4;
        for (i=0; i<hmin; i++) {
            delta = MAX(delta, gap + 1 + rprofile[i] - lprofile[i]);
        }

        // If the node has two children of height 1, then we allow the
        // two leaves to be within 1, instead of 2
        if (((node->left != NULL && node->left->height == 1) ||
                (node->right != NULL && node->right->height == 1))&&delta>4) {
            delta--;
        }

        node->edge_length = ((delta+1)/2) - 1;
    }

    //now fill in the height of node
    h = 1;
    if (node->left != NULL) {
        h = MAX(node->left->height + node->edge_length + 1, h);
    }

    if (node->right != NULL) {
        h = MAX(node->right->height + node->edge_length + 1, h);
    }
    node->height = h;
}

// This function prints the given level of the given tree, assuming
// that the node has the given x cordinate.
void print_level(asciinode *node, int x, int level) {
    int i, isleft;
    if (node == NULL) return;
    isleft = (node->parent_dir == -1);

    if (level == 0) {
        for (i = 0; i < (x-print_next-((node->lablen-isleft)/2)); i++) {
            printf(" ");
        }

        print_next += i;
        printf("%s", node->label);
        print_next += node->lablen;
    }
    else if (node->edge_length >= level) {
        if (node->left != NULL) {
            for (i = 0; i < (x-print_next-(level)); i++) {
                printf(" ");
            }

            print_next += i;
            printf("/");
            print_next++;
        }

        if (node->right != NULL) {
            for (i = 0; i < (x-print_next+(level)); i++) {
                printf(" ");
            }

            print_next += i;
            printf("\\");
            print_next++;
        }
    }
    else {
        print_level(node->left,
                x-node->edge_length-1,
                level-node->edge_length-1);
        print_level(node->right,
                x+node->edge_length+1,
                level-node->edge_length-1);
    }
}

// prints ascii tree for given tree structure
void print_ascii_tree(node* t) {
    asciinode* proot;
    int xmin, i;

    if (t == NULL) return;

    proot = build_ascii_tree(t);
    compute_edge_lengths(proot);

    for (i = 0; i < proot->height && i < MAX_HEIGHT; i++) {
        lprofile[i] = INFINITY;
    }

    compute_lprofile(proot, 0, 0);
    xmin = 0;

    for (i = 0; i < proot->height && i < MAX_HEIGHT; i++) {
        xmin = MIN(xmin, lprofile[i]);
    }

    for (i = 0; i < proot->height; i++) {
        print_next = 0;
        print_level(proot, -xmin, i);
        printf("\n");
    }

    if (proot->height >= MAX_HEIGHT) {
        printf("(This tree is taller than %d, and may be drawn incorrectly.)\n", MAX_HEIGHT);
    }
    free_ascii_tree(proot);
}