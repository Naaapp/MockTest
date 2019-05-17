//Uliege INFO0940-1: Operating Systems
//Project 4 : Théo Stassen and Ludovic Sangiovanni

typedef int (*Hash_Function) (unsigned char *, unsigned int, unsigned char *);

typedef struct { 
    unsigned char *hash;
    unsigned char *data;
} merkle_tree_node;

typedef struct {
    size_t n;
    size_t tree_height;
    size_t hash_size;
    size_t data_block_size;
    size_t data_blocks;
    Hash_Function hash_function;
    merkle_tree_node *nodes;
} merkle_tree;

typedef struct Block Block;
struct Block
{
    int number;
    Block *next;
};

typedef struct BlockList BlockList;
struct BlockList
{
    Block *first;
};

BlockList * bl_init();
void bl_insert(BlockList *list, int new);
void bl_read(BlockList *list);

int build_tree(merkle_tree *mt, unsigned char **data);
void tree_cmp(merkle_tree *a, merkle_tree *b, size_t i, BlockList* bl);
int set_tree_data(merkle_tree *mt, size_t i, unsigned char *data);
void freeMerkleTree(merkle_tree *mt);

