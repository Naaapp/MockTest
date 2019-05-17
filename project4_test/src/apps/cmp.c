//Uliege INFO0940-1: Operating Systems
//Project 4 : Théo Stassen and Ludovic Sangiovanni


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "merkle_tree.c"

//The block size is fixed to 1024. 
#define BLOCK_SIZE 1024

int convertDecimalToOctal(int decimalNumber)
{
    int octalNumber = 0, i = 1;

    while (decimalNumber != 0)
    {
        octalNumber += (decimalNumber % 8) * i;
        decimalNumber /= 8;
        i *= 10;
    }

    return octalNumber;
}

int main (int argc, char *argv[])
{
    /**
     Part 1 : write the input files, create corresponding trees, save the trees in .mktree files
    */

    //Check the arguments
    if (argc != 4 || strcmp(argv[1], "-l") != 0){
        printf("Wrong arguments\n");
        return (-1);
    }

    //Open the files

    char filepath1[20]; 
    char filepath2[20]; 
    strcpy(filepath1, argv[2]);
    strcpy(filepath2, argv[3]);

    FILE *fp1 = fopen(filepath1, "r");
    FILE *fp2 = fopen(filepath2, "r");
    if(fp1 == NULL || fp2 == NULL) {
      perror("Error in opening file");
      return(-1);
    }

    //Variable definition

    int c1;
    int c2;
    int i;
    int j;
    int k;
    int file_size = 0;
    int endoffile = 0;

    //Check the size of the files (must be the same)

    while(1){
        fgetc(fp1);
        if( feof(fp1) ) {
            break;
        }
        file_size++;
    }
    fseek(fp1, 0, SEEK_SET );

    //Find the number of data_blocks and the three height corresponding to the file size

    int data_blocks =  file_size / BLOCK_SIZE + (int)(file_size % BLOCK_SIZE != 0);

    int three_height = log2(data_blocks) + 2;

    //data blocks and merkle tree initialization

    unsigned char *data1[data_blocks], *data2[data_blocks], buffer1[BLOCK_SIZE], buffer2[BLOCK_SIZE];
    merkle_tree mt_a = {0, three_height, MD5_DIGEST_LENGTH, BLOCK_SIZE, data_blocks, MD5One, NULL};
    merkle_tree mt_b = {0, three_height, MD5_DIGEST_LENGTH, BLOCK_SIZE, data_blocks, MD5One, NULL};

    //Read the files and put in blocks of data

    for (i=0; i<data_blocks; i++) {
        for (j=0; j<BLOCK_SIZE; j++){

            if(endoffile){
                buffer1[j] = 97;
                buffer2[j] = 97;
            }
            else{
                c1 = fgetc(fp1);
                c2 = fgetc(fp2);
                if( feof(fp1) ) {
                    endoffile = 1;
                }
                buffer1[j] = c1;
                buffer2[j] = c2;
            }
        }
        data1[i] = (unsigned char *)malloc(sizeof(char) * BLOCK_SIZE);
        data2[i] = (unsigned char *)malloc(sizeof(char) * BLOCK_SIZE);
        memcpy(data1[i], buffer1, BLOCK_SIZE);
        memcpy(data2[i], buffer2, BLOCK_SIZE);
    }

    //Build trees with data

    build_tree_from_data(&mt_a, data1);
    build_tree_from_data(&mt_b, data2);

    //You can print the resulting trees

    // print_tree(&mt_a, stdout);
    // print_tree(&mt_b, stdout);

    //Save the trees in .mktree files

    strcat(filepath1,".mktree");
    strcat(filepath2,".mktree");
    FILE * fp3 = fopen(filepath1, "w+");
    FILE * fp4 = fopen(filepath2, "w+");

    save_tree(&mt_a,fp3);
    save_tree(&mt_b,fp4);
    fseek(fp3, 0, SEEK_SET );
    fseek(fp4, 0, SEEK_SET );

    /**
    Part 2 : extract the trees from .mktree files, build the trees object from that
    */

    //Build the trees from the .mktree files

    merkle_tree mt_c = {0, three_height, MD5_DIGEST_LENGTH*2, BLOCK_SIZE, data_blocks, MD5One, NULL};
    merkle_tree mt_d = {0, three_height, MD5_DIGEST_LENGTH*2, BLOCK_SIZE, data_blocks, MD5One, NULL};

    build_tree_from_file(&mt_c,fp3);
    build_tree_from_file(&mt_d,fp4);

    //You can print the resulting trees

    // print_tree(&mt_c, stdout);
    // print_tree(&mt_d, stdout);

    //Create a linked list to put the number of block whith differences

    BlockList * bl = bl_init();

    //Compare two merkle trees

    tree_cmp(&mt_c, &mt_d, 1, bl);

    //Read the corresponding blocks

    fseek(fp1, 0, SEEK_SET );
    fseek(fp2, 0, SEEK_SET );
    int prev_block_number = 0; 
    int block_number = 0;

    Block *current = bl->first->next;
    while(current != NULL){
        block_number = current->number;
        //We shift the position to the next interesting block (the -1 comes from we had shift one block by reading)
        fseek(fp1, (block_number-prev_block_number-1)*BLOCK_SIZE, SEEK_CUR );
        fseek(fp2, (block_number-prev_block_number-1)*BLOCK_SIZE, SEEK_CUR );
        for (k=0; k<BLOCK_SIZE; k++){
            c1 = fgetc(fp1);
            c2 = fgetc(fp2);
            if (c1 != c2)
                printf(" %d\t %d\t%d\n", (block_number-1)*BLOCK_SIZE+k+1, convertDecimalToOctal(c1), convertDecimalToOctal(c2));
        }
        prev_block_number = block_number;
        current = current->next;
    }

    //Free merkle tree objects

    freeMerkleTree(&mt_a);
    freeMerkleTree(&mt_b);
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);
    fclose(fp4);

    return 0;
}