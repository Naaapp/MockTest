#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "merkle_tree.h"

#include "md5.c"


static int hash_node(merkle_tree *mt, size_t i);
// static void print_tree(merkle_tree *mt);

//build a merkle tree with settings in 'mt'
//use data blocks
int build_tree_from_data(merkle_tree *mt, unsigned char *data[]) {

    if (mt->data_blocks > (unsigned int)(1 << (mt->tree_height - 1)))
        return -1;
    unsigned int i, leaf_start;
    leaf_start = (1 << (mt->tree_height - 1));
    mt->n = leaf_start + mt->data_blocks - 1;
    mt->nodes = (merkle_tree_node *)malloc(sizeof(merkle_tree_node) * (mt->n + 1));
    for (i = leaf_start; i <= mt->n; i++) {
        mt->nodes[i].data = data[i-leaf_start];
        mt->nodes[i].hash = NULL;
        if (hash_node(mt, i) == -1)
            return -1;
    }
    for (i = leaf_start - 1; i > 0; i--) {
        mt->nodes[i].hash = NULL;
        if (hash_node(mt, i) == -1)
            return -1;
    }
    return 0;
}

//build a merkle tree with settings in 'mt'
//use input File
int build_tree_from_file(merkle_tree *mt, FILE *fp){

    size_t len = 0;
    char* line = NULL;

    if(getline(&line, &len, fp) != -1){
        strtok(line, "\n");
        mt->data_blocks = atoi(line);
        mt->tree_height = abs(log2(mt->data_blocks)) + 1 + (int)(log2(mt->data_blocks) / abs(log2(mt->data_blocks)) != 1);
    }

    if (mt->data_blocks > (unsigned int)(1 << (mt->tree_height - 1)))
        return -1;
    unsigned int i, leaf_start;
    leaf_start = (1 << (mt->tree_height - 1));
    mt->n = leaf_start + mt->data_blocks - 1;
    mt->nodes = (merkle_tree_node *)malloc(sizeof(merkle_tree_node) * (mt->n + 1));
    for (i = 1; i<=mt->n; i++){
        if(getline(&line, &len, fp) != -1){
            strtok(line, "\n");
            if(strcmp(line,"<empty hash>")==0)
                mt->nodes[i].hash = NULL;
            else {
                mt->nodes[i].hash = (unsigned char *)malloc(sizeof(char *) * mt->hash_size);
                memcpy(mt->nodes[i].hash, (unsigned char *)line, mt->hash_size);
            }
        }
    }
    return 0;
}

//compare two merkle trees from node i
//make sure the two trees in same height
//return different data block number
//if no differnece return 0
void tree_cmp(merkle_tree *a, merkle_tree *b, size_t i, BlockList* bl) {

    int number;
    if(a->nodes[i].hash == NULL)
        return;
    if (i > (unsigned int)(1<<a->tree_height)-1)
        return ;
    if (memcmp(a->nodes[i].hash, b->nodes[i].hash, a->hash_size) != 0) {
        if (i<<1 > (unsigned int)(1<<a->tree_height)-1){
            number = i - (unsigned int)(1 << (a->tree_height - 1)) + 1;
            bl_insert(bl,number);
        }
        else {
            tree_cmp(a, b, i<<1, bl);
            tree_cmp(a, b, (i<<1)+1, bl);
        }
    }
    else
        return ;
}

// set tree data with specific block number
//
int set_tree_data(merkle_tree *mt, size_t block_num, unsigned char *data) {

    if (block_num > mt->data_blocks)
        return -1;
    size_t i = (1 << (mt->tree_height - 1)) + block_num - 1;
    if (mt->nodes[i].data)
        free(mt->nodes[i].data);
    mt->nodes[i].data = data;
    if (hash_node(mt, i) == -1)
        return -1;
    for (i>>=1; i>0; i>>=1)
        if (hash_node(mt, i) == -1)
            return -1;
    return 0;
}

//free the Merkle Tree Object...
//
void freeMerkleTree(merkle_tree *mt) {

    unsigned int i;
    if (!mt)
        return;
    if (mt->nodes) {
        for (i=1; i<=mt->n; i++)
            if(mt->nodes[i].hash)
                free(mt->nodes[i].hash);
        free(mt->nodes);
    }
    return;
}


//update a tree node hash
//leaf or inside nodes ...
//
static int hash_node(merkle_tree *mt, size_t i) {

    if (i > (unsigned int)(1<<mt->tree_height)-1)
        return -1;
    if (i < (unsigned int)(1<<(mt->tree_height-1))){
        if (2*i+1 <= mt->n && mt->nodes[2*i].hash && mt->nodes[2*i+1].hash) {
            unsigned char *buffer = (unsigned char *)malloc(sizeof(char *) * (2 * mt->hash_size + 1));
            memcpy(buffer, mt->nodes[2*i].hash, mt->hash_size);
            memcpy(buffer+mt->hash_size, mt->nodes[2*i+1].hash, mt->hash_size);
            if (!mt->nodes[i].hash)
                mt->nodes[i].hash = (unsigned char *)malloc(sizeof(char *) * mt->hash_size);
            mt->hash_function(buffer, 2*mt->hash_size, mt->nodes[i].hash);
            free(buffer);
        }
        else if (2*i <= mt->n && mt->nodes[2*i].hash) {
            if (!mt->nodes[i].hash)
                mt->nodes[i].hash = (unsigned char *)malloc(sizeof(char *) * mt->hash_size);
            memcpy(mt->nodes[i].hash, mt->nodes[2*i].hash, mt->hash_size);
        }
    }
    else {
        if (mt->nodes[i].data) {
            if (!mt->nodes[i].hash)
                mt->nodes[i].hash = (unsigned char *)malloc(sizeof(char *) * mt->hash_size);

            mt->hash_function(mt->nodes[i].data, mt->data_block_size, mt->nodes[i].hash);
        }
        else
            return -1;
    }
    return 0;
}

// for test use
// print a merkle tree nodes' hash
// as a list with node order
// static void print_tree(merkle_tree *mt, FILE* fp) {
//     unsigned int i;
//     printf("--------------------------------\n");
//     for(i=1; i<=mt->n; i++){
//         fprintf(fp, "%s\n",mt->nodes[i].hash);
//         printf("_____\n" );
//     }
//     printf("--------------------------------\n");
//     return;
// }

static void save_tree(merkle_tree *mt, FILE* fp){

    unsigned int i;
    fprintf(fp, "%lu\n", mt->data_blocks );
    for(i=1; i<=mt->n; i++){
        MD5Print(mt->nodes[i].hash, fp);
    }
    return;
    fclose(fp);
}

BlockList * bl_init()
{
    BlockList *list = malloc(sizeof(*list));
    Block *block = malloc(sizeof(*block));

    if (list == NULL || block == NULL)
    {
        exit(EXIT_FAILURE);
    }

    block->number = -1;
    block->next = NULL;
    list->first = block;

    return list;
}


void bl_insert(BlockList *list, int newnumber)
{
    /* Création du nouvel élément */
    Block *new = malloc(sizeof(*new));
    if (list == NULL || new == NULL)
    {
        exit(EXIT_FAILURE);
    }
    new->number = newnumber;
    new->next = NULL;

    Block* current = list->first;
    while(current->next != NULL)
        current = current->next;

    current->next = new;
}



void bl_read(BlockList *list)
{
    if (list == NULL)
    {
        exit(EXIT_FAILURE);
    }

    Block *current = list->first;

    while (current != NULL)
    {
        printf("%d -> ", current->number);
        current = current->next;
    }
    printf("NULL\n");
}


