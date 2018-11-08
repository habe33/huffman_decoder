#include <stdio.h>
#include <stdlib.h>

#define NUM_OF_MAX_CHARACTERS 256

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
int code_table[NUM_OF_MAX_CHARACTERS];

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
    //Node* node_array = (Node * ) calloc(NUM_OF_MAX_CHARACTERS, sizeof(Node));
    for (int i = 0; i < NUM_OF_MAX_CHARACTERS; i++) {
        if (frequency[i] > 0) {
            printf("Frequency: %i, int: %i, char: %c\n", frequency[i], i, i);
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

void fill_binary_table(int code_table[], Node *tree, int code) {
    printf("code for tree letter: %c, code: %i\n", tree->letter, code);
    printf("left: %i\n", code * 10 + 1);
    printf("right: %i\n", code * 10 + 2);
    if (tree->letter >= 0) {
        code_table[(int) tree->letter] = code;
    } else {
        fill_binary_table(code_table, tree->left, code * 10 + 1);
        fill_binary_table(code_table, tree->right, code * 10 + 2);
    }
}

void print_code_table() {
    printf("Binary tree filled...\n");
    for (int i = 0; i < NUM_OF_MAX_CHARACTERS; i++) {
        if (code_table[i] > 0) {
            printf("Binary code for %c: %i \n", i, code_table[i]);
        }
    }
}

// //this is from github. 
// void write_char_bit(FILE* fileout, char c) {
//     int code = code_table[(int) c];
//     if (bits_in_buffer == MAX_BUFFER_SIZE << 3) {
//         size_t bytes_written = fwrite(buffer, 1, MAX_BUFFER_SIZE, fileout);
//         if (bytes_written < MAX_BUFFER_SIZE && ferror(f)) {
//             return INVALID_BIT_WRITE;
//         }
//         bits_in_buffer = 0;
//         memset(buffer, 0, MAX_BUFFER_SIZE);
//     }
//     if (c) {
//         buffer[bits_in_buffer >> 3] |= (0x1 << (7 - bits_in_buffer % 8));
//     }
//     ++bits_in_buffer;
// }

// void encode_chars_to_file(FILE* filein, FILE* fileout, int code_table[]) {
//     while ((c = fgetc(filein)) != EOF) {
//         write_char_bit(c);
//     }
// }


int encode(const char* infile, const char* outfile) {
    FILE *filein, *fileout;
    filein = fopen(infile, "rb");
    if (filein == NULL) {
        printf("Failed to open input file.");
        return 0;
    }

    find_frequencies(filein);
    build_tree(&tree);
    fill_binary_table(code_table, tree, 0);

    print_code_table();

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
