#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_OF_MAX_CHARACTERS 256
#define MAX_BUFFER_SIZE 256
#define SMALLER_NOT_FOUND -55

#define SUCCESS 1
#define ERROR -1
#define LETTER_NOT_FOUND -1

struct node {
    int freq;
    int letter;
    struct node *left;
    struct node *right;
};

typedef struct node Node;
Node *tree;
int *frequency;
int code_table[NUM_OF_MAX_CHARACTERS];

Node *tree_pointer;

unsigned char buffer[MAX_BUFFER_SIZE];
int bits_in_buffer = 0;
int current_bit = 0;
int end_of_file = 0;
unsigned int original_size = 0;

void find_frequencies(FILE *f);

int find_smaller(Node *node_array[], int from);

void build_tree(Node **tree);

void fill_binary_table(Node *tree, int code);

int flush_buffer(FILE *f);

int write_legend(FILE *outfile);

int write_bit(FILE *f, int bit);

int encode_char(int character, FILE *outfile);

int encode_chars_to_file(FILE *infile, FILE *outfile);

int encode(const char *infile, const char *outfile);

int decode(const char *infile, const char *outfile);

void decode_text(FILE *infile, FILE *outfile);

void find_frequencies(FILE *f) {
    frequency = (int *) calloc(NUM_OF_MAX_CHARACTERS, sizeof(int));
    int c;
    while ((c = fgetc(f)) != EOF) {
        ++frequency[c];
        ++original_size;
    }
}

int find_smaller(Node *node_array[], int from) {
    int smaller;
    int i = 0;
    while (node_array[i]->freq == -1 || node_array[i]->freq == 0) {
        i++;
    }
    smaller = i;
    if (i == from) {
        i++;
        while (node_array[i]->freq == -1 || node_array[i]->freq == 0) {
            i++;
            if (i == NUM_OF_MAX_CHARACTERS) {
                break;
            }
        }
        smaller = i;
    }
    if (smaller == NUM_OF_MAX_CHARACTERS) {
        return SMALLER_NOT_FOUND;
    }
    for (i = 1; i < NUM_OF_MAX_CHARACTERS; i++) {
        if (node_array[i]->freq == -1 || node_array[i]->freq == 0 || i == from)
            continue;
        if (node_array[i]->freq < node_array[smaller]->freq)
            smaller = i;
    }
    if (smaller >= NUM_OF_MAX_CHARACTERS) {
        smaller = NUM_OF_MAX_CHARACTERS - 1;
    }
    return smaller;
}

void build_tree(Node **tree) {
    Node *node_array[NUM_OF_MAX_CHARACTERS];
    for (int i = 0; i < NUM_OF_MAX_CHARACTERS; i++) {
        if (frequency[i] > 0) {
            printf("Frequency: %i, char: %c\n", frequency[i], i);
        }
        node_array[i] = malloc(sizeof(Node));
        node_array[i]->freq = frequency[i];
        node_array[i]->letter = i;
        node_array[i]->left = NULL;
        node_array[i]->right = NULL;
    }
    int small_one, small_two;
    int subtrees = NUM_OF_MAX_CHARACTERS;
    Node *temp;
    while (subtrees > 1) {
        small_one = find_smaller(node_array, -1);
        small_two = find_smaller(node_array, small_one);
        if (small_one == SMALLER_NOT_FOUND || small_two == SMALLER_NOT_FOUND) {
            break;
        }
        if (node_array[small_one]->freq > 0 && node_array[small_two]->freq > 0) {
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

int find_letter(Node *local_tree, const int bit) {
    if (bit) {
        //printf("Going right...\n");
        if (local_tree->right->letter >= 0) {
            //printf("Found letter: %i\n", local_tree->right->letter);
            return local_tree->right->letter;
        } else {
            tree_pointer = local_tree->right;
            return LETTER_NOT_FOUND;
        }
    } else {
        //printf("Going left...\n");
        if (local_tree->left->letter >= 0) {
            //printf("Found letter: %i\n", local_tree->left->letter);
            return local_tree->left->letter;
        } else {
            tree_pointer = local_tree->left;
            return LETTER_NOT_FOUND;
        }
    }
}

//void fill_binary_table(Node *tree, const char code[]) {
//    char new_code_left[NUM_OF_TREE_BITS];
//    char new_code_right[NUM_OF_TREE_BITS];
//    if (tree->letter >= 0) {
//        printf("\nChar: %c\n", tree->letter);
//        for (int y = NUM_OF_TREE_BITS - 1; y >= 0; y--) {
//            code_table[tree->letter][y] = code[y];
//            printf("%c", code_table[tree->letter][y]);
//        }
//    } else {
//        for (int k = NUM_OF_TREE_BITS - 1; k >= 0; k--) {
//            if (k == NUM_OF_TREE_BITS - 1) {
//                new_code_left[k] = '1';
//                new_code_right[k] = '2';
//            } else {
//                new_code_left[k] = code[k + 1];
//                new_code_right[k] = code[k + 1];
//            }
//        }
//        fill_binary_table(tree->left, new_code_left);
//        fill_binary_table(tree->right, new_code_right);
//    }
//}

void fill_binary_table(Node *tree, int code) {
    if (tree->letter >= 0) {
        code_table[tree->letter] = code;
        //printf("\nChar: %c %i, code %i\n", tree->letter, tree->letter, code);
        char buffer[65];
        itoa(code, buffer, 2);
        //printf("Binary value = %s\n", buffer);
    } else {
        fill_binary_table(tree->left, code * 2);
        fill_binary_table(tree->right, code * 2 + 1);
    }
}

int flush_buffer(FILE *f) {
    if (bits_in_buffer) {
        size_t bytes_written = fwrite(buffer, 1, (size_t) ((bits_in_buffer + 7) >> 3), f);
        if (bytes_written < MAX_BUFFER_SIZE && ferror(f)) {
            return ERROR;
        }
        bits_in_buffer = 0;
    }
    return SUCCESS;
}

int encode_char(int character, FILE *outfile) {
    int res;
    int code;
    char c;
    int found_first_bit = 0;
    code = code_table[character];
    //printf("Char %c; Code %i\n", character, code);
    char buffer[65];
    for (int i = 0; i < 65; i++){
        buffer[i] = -1;
    }
    itoa(code, buffer, 2);
    //printf("Binary value = %s\n", buffer);
    for (int k = 0; k < 65; k++) {
        c = buffer[k];
        //printf("Read from buffer %c \n", c);
        if (c == '0') {
            //printf("Writing bit 0\n");
            res = write_bit(outfile, 0);
        } else if (c == '1') {
            if (found_first_bit) {
                //printf("Writing bit 1\n");
                res = write_bit(outfile, 1);
            } else {
                found_first_bit = 1;
            }
        }
        if (res < 0) {
            return ERROR;
        }
    }
    return SUCCESS;
}

int encode_chars_to_file(FILE *infile, FILE *outfile) {
    int c;
    fseek(infile, 0, SEEK_SET);
    int res;
    printf("Writing encoded chars to file...");
    while ((c = fgetc(infile)) != EOF) {
        res = encode_char(c, outfile);
        if (res < 0) {
            return ERROR;
        }
    }
    return flush_buffer(outfile);
}

int encode(const char *infile, const char *outfile) {
    FILE *filein, *fileout;
    filein = fopen(infile, "rb");
    if (filein == NULL) {
        printf("Failed to open input file.");
        return ERROR;
    }

    find_frequencies(filein);
    build_tree(&tree);
    int code = 1;
    fill_binary_table(tree, code);
    fileout = fopen(outfile, "wb");
    if (fileout == NULL) {
        printf("Failed to open output file.");
        return ERROR;
    }
    if (write_legend(fileout) != 0) {
        return ERROR;
    }
    int res = encode_chars_to_file(filein, fileout);
    fclose(filein);
    fclose(fileout);
    return res;
}

int write_legend(FILE *outfile) {
    int i, j, byte = 0;
    size_t size = sizeof(unsigned int) + NUM_OF_MAX_CHARACTERS * (1 + sizeof(int));
    char *buffer = (char *) calloc(size, 1);
    if (buffer == NULL) {
        return ERROR;
    }
    j = sizeof(int);
    while (j--) {
        buffer[byte++] = (original_size >> (j << 3)) & 0xff;
    }
    int freq;
    for (i = 0; i < NUM_OF_MAX_CHARACTERS; ++i) {
        freq = frequency[i];
        j = sizeof(int);
        while (j--) {
            buffer[byte++] = (freq >> (j << 3)) & 0xff;
        }
    }
    printf("\nWriting header...\n");
    fwrite(buffer, 1, size, outfile);
    free(buffer);
    return 0;
}

int write_bit(FILE *f, int bit) {
    if (bits_in_buffer == MAX_BUFFER_SIZE << 3) {
        size_t bytes_written = fwrite(buffer, 1, MAX_BUFFER_SIZE, f);
        if (bytes_written < MAX_BUFFER_SIZE && ferror(f)) {
            return ERROR;
        }
        bits_in_buffer = 0;
        memset(buffer, 0, MAX_BUFFER_SIZE);
    }
    if (bit) {
        buffer[bits_in_buffer >> 3] |= (0x1 << (7 - bits_in_buffer % 8));
    }
    ++bits_in_buffer;
    return SUCCESS;
}

int read_legend(FILE *f) {
    size_t bytes_read;
    int byte = 0;
    unsigned char buff[4];

    //read original size
    bytes_read = fread(&buff, 1, sizeof(int), f);
    if (bytes_read < 1)
        return ERROR;
    byte = 0;
    original_size = buff[byte];
    while (byte < sizeof(int)) {
        original_size = (original_size << (1 << 3)) | buff[byte++];
    }
    printf("ORIG_SIZE: %i", original_size);

    //read frequencies
    int num = NUM_OF_MAX_CHARACTERS;
    bytes_read = fread(&num, 1, 1, f);
    frequency = (int *) calloc(NUM_OF_MAX_CHARACTERS, sizeof(int));
    if (bytes_read < 1) {
        return ERROR;
    }
    size_t size = NUM_OF_MAX_CHARACTERS * (1 + sizeof(int));
    char *buffer = (char *) calloc(size, 1);
    if (buffer == NULL) {
        return ERROR;
    }
    fread(buffer, 1, size, f);
    byte = 0;
    int j;
    unsigned int freq;
    for (int i = 0; i < NUM_OF_MAX_CHARACTERS; i++) {
        j = 0;
        freq = (unsigned char) buffer[byte++];
        while (++j < sizeof(int)) {
            freq = (freq << (1 << 3)) | (unsigned char) buffer[byte++];
        }
        frequency[i] = freq;
    }
    free(buffer);
    return SUCCESS;
}

int read_bit(FILE *f) {
    if (current_bit == bits_in_buffer) {
        if (end_of_file) {
            return -1;
        } else {
            size_t bytes_read = fread(buffer, 1, MAX_BUFFER_SIZE, f);
            if (bytes_read < MAX_BUFFER_SIZE) {
                if (feof(f)) {
                    end_of_file = 1;
                }
            }
            bits_in_buffer = bytes_read << 3;
            current_bit = 0;
        }
    }

    if (bits_in_buffer == 0) {
        return -1;
    }
    int bit = (buffer[current_bit >> 3] >> (7 - current_bit % 8)) & 0x1;
    ++current_bit;
    return bit;
}

void decode_text(FILE *infile, FILE *outfile) {
    int bit;
    tree_pointer = tree;
    fseek(infile, -1, SEEK_CUR);
    int char_count = 0;
    while (1) {
        bit = read_bit(infile);
        if (bit == -1) {
            break;
        }
        int letter = find_letter(tree_pointer, bit);
        if (letter != LETTER_NOT_FOUND) {
            ++char_count;
            if (char_count <= original_size) {
                char c = (char) letter;
                fwrite(&c, 1, 1, outfile);
                tree_pointer = tree;
            } else {
                break;
            }
        }
    }
}


int decode(const char *infile, const char *outfile) {
    FILE *filein, *fileout;
    filein = fopen(infile, "rb");
    if (filein == NULL) {
        printf("Failed to open input file.");
        return ERROR;
    }
    if (read_legend(filein) > 0) {
        build_tree(&tree);
    }
    int code = 1;
    fill_binary_table(tree, code);
    fileout = fopen(outfile, "wb");
    if (fileout == NULL) {
        printf("Failed to open output file.");
        return ERROR;
    }
    decode_text(filein, fileout);
    return SUCCESS;
}

int main(int argc, char const *argv[]) {
    if (argc != 4) {
        return ERROR;
    }
    if (argv[1][0] == '-' && argv[1][1] == 'e') {
        return encode(argv[2], argv[3]);
    } else if (argv[1][0] == '-' && argv[1][1] == 'd') {
        return decode(argv[2], argv[3]);
    }
}
