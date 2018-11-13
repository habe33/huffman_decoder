#include <stdio.h>
#include <stdlib.h>

#define NUM_OF_MAX_CHARACTERS 256
#define NUM_OF_TREE_BITS 30

/* Node structure */
struct node {
    int freq;
    int letter;
    struct Node *left;
    struct Node *right;
};

typedef struct node Node;

Node* tree;
int* frequency;
int code_table[NUM_OF_MAX_CHARACTERS][NUM_OF_TREE_BITS];

void find_frequencies(FILE *f) {
    frequency = (int *) calloc(NUM_OF_MAX_CHARACTERS, sizeof(int));
    int c;
    while ((c = fgetc(f)) != EOF) {
        ++frequency[c];
    }
}

int find_smaller(Node* node_array[], int from) {
    int smaller;
    int i = 0;
    while (node_array[i]->freq == -1 || node_array[i]->freq == 0)
        i++;
    
    smaller = i;
    if (i == from){
        i++;
        while (node_array[i]->freq == -1 || node_array[i]->freq == 0)
            i++;
        smaller=i;
    }

    for (i = 1; i < NUM_OF_MAX_CHARACTERS; i++){
        if (node_array[i]->freq == -1 || node_array[i]->freq == 0 || i == from)
            continue;
        if (node_array[i]->freq < node_array[smaller]->freq)
            smaller = i;
    }
    if (smaller >= NUM_OF_MAX_CHARACTERS)  {
        smaller = NUM_OF_MAX_CHARACTERS - 1;
    }
    return smaller;
}


void build_tree(Node** tree) {
    Node* node_array[NUM_OF_MAX_CHARACTERS];
    for (int i = 0; i < NUM_OF_MAX_CHARACTERS; i++) {
        if (frequency[i] > 0) {
            printf("Frequency: %i, char: %c\n", frequency[i], i, i);
        }
        node_array[i] = malloc(sizeof(Node));
        node_array[i]->freq = frequency[i];
        node_array[i]->letter = i;
        node_array[i]->left = NULL;
        node_array[i]->right = NULL;

    }
    int small_one, small_two;
    int subtrees = NUM_OF_MAX_CHARACTERS;
    Node* temp;
    while (subtrees > 1) {
        small_one = find_smaller(node_array, -1);
        small_two = find_smaller(node_array, small_one);
        if (node_array[small_one]->freq > 0 && node_array[small_two]->freq > 0) {
            printf("Small freq node one: %c\n", small_one);
            printf("Small freq node two: %c\n", small_two);
            printf("Creating new nodes... \n");
            temp = node_array[small_one];
            node_array[small_one] = malloc(sizeof(Node));
            node_array[small_one]->freq = temp->freq + node_array[small_two]->freq;
            node_array[small_one]->letter = -2;
            node_array[small_one]->left = node_array[small_two];
            node_array[small_one]->right = temp;
            node_array[small_two]->freq = -1;
        }
        subtrees--;
    }
    printf("Tree successfully created...\n");
    *tree = node_array[small_one];
}

void fill_binary_table(char code_table[NUM_OF_MAX_CHARACTERS][NUM_OF_TREE_BITS], Node *tree, char code[]) {
    if (tree->letter >= 0) {
        printf("\nTree letter: %c\n", tree->letter);
        for (int y = 0; y < NUM_OF_TREE_BITS; y++) {
            printf("%c", code[y]);
            code_table[(int) tree->letter][y] = code[y];
        }
    } else {
        char new_code_left[NUM_OF_TREE_BITS];
        char new_code_right[NUM_OF_TREE_BITS];
        for (int k = 0; k < NUM_OF_TREE_BITS; k++){
            if (k == NUM_OF_TREE_BITS - 1) {
                new_code_left[k] = '1';
                new_code_right[k] = '2';
            } else {
                new_code_left[k] = code[k + 1];
                new_code_right[k] = code[k + 1];
            }
        }
        fill_binary_table(code_table, tree->left, new_code_left);
        fill_binary_table(code_table, tree->right, new_code_right);
    }
}

int encode(const char* infile, const char* outfile) {
    FILE *filein, *fileout;
    filein = fopen(infile, "rb");
    if (filein == NULL) {
        printf("Failed to open input file.");
        return 0;
    }

    find_frequencies(filein);
    build_tree(&tree);
    char code[NUM_OF_TREE_BITS] = {'0'};
    fill_binary_table(code_table, tree, code);




    fileout = fopen(outfile, "wb");
    if (fileout == NULL) {
        printf("Failed to open output file.");
        return 0;
    }
    //encode_chars_to_file(fileout, filein, code_table);
    fclose(filein);
    fclose(fileout);
    return 0;
}

int main(int argc, char const *argv[]) {
    if (argc != 4) {
        return 0;
    }
    if (argv[1][0] == '-' || argv[1][1] == 'e') {
        encode(argv[2], argv[3]);
    }
}
