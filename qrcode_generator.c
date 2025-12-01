#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define IMAGE_FILE_NAME "qrcode.ppm"

#define BYTE_SIZE 8

#define WHITE 0
#define BLACK 1

/* qrcode cell */
#define LOCKED 1
#define UNLOCKED 0
typedef struct cell {
    unsigned char value;
    bool locked;
} cell_t;

#define QRCODE_VERSIONS 40
#define CORRECTION_LEVELS 4
#define CORRECTION_LOW 0
#define CORRECTION_MEDIUM 1
#define CORRECTION_QUARTILE 2
#define CORRECTION_HIGH 3

#define PADDING 4

#define MASK_NUMBER 8
#define MASK_ANY -1

/* Table with the maximum data a qrcode of a specific version can hold.
 * To make more sense, qrcodes go from 1 to 40 and not from 0 to 39 (the first row is filled with invalid parameters).
 * I haven't found a formula to compute them, so I just keep them here */
typedef struct qrcode_information {
    int character_capacity;
    int error_correction_codewords_per_block;
    int blocks_in_group1;
    int data_codewords_per_block_in_group1;
    int blocks_in_group2;
    int data_codewords_per_block_in_group2;
} qrcode_information_t;
const qrcode_information_t qrcode_infos[QRCODE_VERSIONS + 1][CORRECTION_LEVELS] =  {
    { {-1, -1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1, -1} },
    { {17, 7, 1, 19, 0, 0}, {14, 10, 1, 16, 0, 0}, {11, 13, 1, 13, 0, 0}, {7, 17, 1, 9, 0, 0} },
    { {32, 10, 1, 34, 0, 0}, {26, 16, 1, 28, 0, 0}, {20, 22, 1, 22, 0, 0}, {14, 28, 1, 16, 0, 0} },
    { {53, 15, 1, 55, 0, 0}, {42, 26, 1, 44, 0, 0}, {32, 18, 2, 17, 0, 0}, {24, 22, 2, 13, 0, 0} },
    { {78, 20, 1, 80, 0, 0}, {62, 18, 2, 32, 0, 0}, {46, 26, 2, 24, 0, 0}, {34, 16, 4, 9, 0, 0} },
    { {106, 26, 1, 108, 0, 0}, {84, 24, 2, 43, 0, 0}, {60, 18, 2, 15, 2, 16}, {44, 22, 2, 11, 2, 12} },
    { {134, 18, 2, 68, 0, 0}, {106, 16, 4, 27, 0, 0}, {74, 24, 4, 19, 0, 0}, {58, 28, 4, 15, 0, 0} },
    { {154, 20, 2, 78, 0, 0}, {122, 18, 4, 31, 0, 0}, {86, 18, 2, 14, 4, 15}, {64, 26, 4, 13, 1, 14} },
    { {192, 24, 2, 97, 0, 0}, {152, 22, 2, 38, 2, 39}, {108, 22, 4, 18, 2, 19}, {84, 26, 4, 14, 2, 15} },
    { {230, 30, 2, 116, 0, 0}, {180, 22, 3, 36, 2, 37}, {130, 20, 4, 16, 4, 17}, {98, 24, 4, 12, 4, 13} },
    { {271, 18, 2, 68, 2, 69}, {213, 26, 4, 43, 1, 44}, {151, 24, 6, 19, 2, 20}, {119, 28, 6, 15, 2, 16} },
    { {321, 20, 4, 81, 0, 0}, {251, 30, 1, 50, 4, 51}, {177, 28, 4, 22, 4, 23}, {137, 24, 3, 12, 8, 13} },
    { {367, 24, 2, 92, 2, 93}, {287, 22, 6, 36, 2, 37}, {203, 26, 4, 20, 6, 21}, {155, 28, 7, 14, 4, 15} },
    { {425, 26, 4, 107, 0, 0}, {331, 22, 8, 37, 1, 38}, {241, 24, 8, 20, 4, 21}, {177, 22, 12, 11, 4, 12} },
    { {458, 30, 3, 115, 1, 116}, {362, 24, 4, 40, 5, 41}, {258, 20, 11, 16, 5, 17}, {194, 24, 11, 12, 5, 13} },
    { {520, 22, 5, 87, 1, 88}, {412, 24, 5, 41, 5, 42}, {292, 30, 5, 24, 7, 25}, {220, 24, 11, 12, 7, 13} },
    { {586, 24, 5, 98, 1, 99}, {450, 28, 7, 45, 3, 46}, {322, 24, 15, 19, 2, 20}, {250, 30, 3, 15, 13, 16} },
    { {644, 28, 1, 107, 5, 108}, {504, 28, 10, 46, 1, 47}, {364, 28, 1, 22, 15, 23}, {280, 28, 2, 14, 17, 15} },
    { {718, 30, 5, 120, 1, 121}, {560, 26, 9, 43, 4, 44}, {394, 28, 17, 22, 1, 23}, {310, 28, 2, 14, 19, 15} },
    { {792, 28, 3, 113, 4, 114}, {624, 26, 3, 44, 11, 45}, {442, 26, 17, 21, 4, 22}, {338, 26, 9, 13, 16, 14} },
    { {858, 28, 3, 107, 5, 108}, {666, 26, 3, 41, 13, 42}, {482, 30, 15, 24, 5, 25}, {382, 28, 15, 15, 10, 16} },
    { {929, 28, 4, 116, 4, 117}, {711, 26, 17, 42, 0, 0}, {509, 28, 17, 22, 6, 23}, {403, 30, 19, 16, 6, 17} },
    { {1003, 28, 2, 111, 7, 112}, {779, 28, 17, 46, 0, 0}, {565, 30, 7, 24, 16, 25}, {439, 24, 34, 13, 0, 0} },
    { {1091, 30, 4, 121, 5, 122}, {857, 28, 4, 47, 14, 48}, {611, 30, 11, 24, 14, 25}, {461, 30, 16, 15, 14, 16} },
    { {1171, 30, 6, 117, 4, 118}, {911, 28, 6, 45, 14, 46}, {661, 30, 11, 24, 16, 25}, {511, 30, 30, 16, 2, 17} },
    { {1273, 26, 8, 106, 4, 107}, {997, 28, 8, 47, 13, 48}, {715, 30, 7, 24, 22, 25}, {535, 30, 22, 15, 13, 16} },
    { {1367, 28, 10, 114, 2, 115}, {1059, 28, 19, 46, 4, 47}, {751, 28, 28, 22, 6, 23}, {593, 30, 33, 16, 4, 17} },
    { {1465, 30, 8, 122, 4, 123}, {1125, 28, 22, 45, 3, 46}, {805, 30, 8, 23, 26, 24}, {625, 30, 12, 15, 28, 16} },
    { {1528, 30, 3, 117, 10, 118}, {1190, 28, 3, 45, 23, 46}, {868, 30, 4, 24, 31, 25}, {658, 30, 11, 15, 31, 16} },
    { {1628, 30, 7, 116, 7, 117}, {1264, 28, 21, 45, 7, 46}, {908, 30, 1, 23, 37, 24}, {698, 30, 19, 15, 26, 16} },
    { {1732, 30, 5, 115, 10, 116}, {1370, 28, 19, 47, 10, 48}, {982, 30, 15, 24, 25, 25}, {742, 30, 23, 15, 25, 16} },
    { {1840, 30, 13, 115, 3, 116}, {1452, 28, 2, 46, 29, 47}, {1030, 30, 42, 24, 1, 25}, {790, 30, 23, 15, 28, 16} },
    { {1952, 30, 17, 115, 0, 0}, {1538, 28, 10, 46, 23, 47}, {1112, 30, 10, 24, 35, 25}, {842, 30, 19, 15, 35, 16} },
    { {2068, 30, 17, 115, 1, 116}, {1628, 28, 14, 46, 21, 47}, {1168, 30, 29, 24, 19, 25}, {898, 30, 11, 15, 46, 16} },
    { {2188, 30, 13, 115, 6, 116}, {1722, 28, 14, 46, 23, 47}, {1228, 30, 44, 24, 7, 25}, {958, 30, 59, 16, 1, 17} },
    { {2303, 30, 12, 121, 7, 122}, {1809, 28, 12, 47, 26, 48}, {1283, 30, 39, 24, 14, 25}, {983, 30, 22, 15, 41, 16} },
    { {2431, 30, 6, 121, 14, 122}, {1911, 28, 6, 47, 34, 48}, {1351, 30, 46, 24, 10, 25}, {1051, 30, 2, 15, 64, 16} },
    { {2563, 30, 17, 122, 4, 123}, {1989, 28, 29, 46, 14, 47}, {1423, 30, 49, 24, 10, 25}, {1093, 30, 24, 15, 46, 16} },
    { {2699, 30, 4, 122, 18, 123}, {2099, 28, 13, 46, 32, 47}, {1499, 30, 48, 24, 14, 25}, {1139, 30, 42, 15, 32, 16} },
    { {2809, 30, 20, 117, 4, 118}, {2213, 28, 40, 47, 7, 48}, {1579, 30, 43, 24, 22, 25}, {1219, 30, 10, 15, 67, 16} },
    { {2953, 30, 19, 118, 6, 119}, {2331, 28, 18, 47, 31, 48}, {1663, 30, 34, 24, 34, 25}, {1273, 30, 20, 15, 61, 16} }
};

/* Lookup tables for correction computation */
#define LOOKUPTABLE_SIZE 256
int log_lookup_table[LOOKUPTABLE_SIZE];
int log_reverse_lookup_table[LOOKUPTABLE_SIZE];

#define BYTE_MODE_INDICATOR_SIZE 4
const unsigned char BYTE_MODE_INDICATOR[BYTE_MODE_INDICATOR_SIZE] = {0, 1, 0, 0};

/* Character count indicator size for byte mode */
const int character_count_indicator_size[QRCODE_VERSIONS + 1] =  {
    -1, 8, 8, 8, 8, 8, 8, 8, 8, 8, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
};

/* Terminator 0's */
#define TERMINATOR_MAX_SIZE 4
/* Filler characters to add to data after the input if space is still not filled */
const int FILLER_CHARACTERS[] = {236, 17};

const int REMAINDER_BITS_NUMER[QRCODE_VERSIONS + 1] = {
    -1, 0, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0
};

/* Table with type information bits for every mask and correction level */
const int TYPE_INFORMATION_BITS[MASK_NUMBER][CORRECTION_LEVELS][15] = {
    { 
        {1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0},
        {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
        {0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1},
        {0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1} 
    },
    {
        {1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1},
        {1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
        {0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0} 
    },
    {
        {1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0},
        {1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0},
        {0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1},
        {0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1} 
    },
    {
        {1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1},
        {1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1},
        {0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0},
        {0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0} 
    },
    {
        {1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1},
        {1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1},
        {0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0} 
    },
    {
        {1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0},
        {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0},
        {0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1},
        {0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1} 
    },
    {
        {1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1},
        {0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0},
        {0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0} 
    },
    {
        {1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0},
        {1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1},
        {0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1} 
    }
};

/* Version information (qrcode versions from 1 to 6 do not have those) */
const unsigned char VERSION_INFORMATION_BITS[][18] = {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0},
    {0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1},
    {0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1},
    {0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1},
    {0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0},
    {0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1},
    {0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1},
    {0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0},
    {0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1},
    {0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1},
    {0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0},
    {0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1},
    {0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1},
    {0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0},
    {0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0},
    {0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1},
    {0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1},
    {0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0},
    {0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0},
    {0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1},
    {0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0},
    {1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0},
    {1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1},
    {1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0},
    {1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0},
    {1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1}
};

/* Positions of align patterns */
#define PATTERN_MAX_NUMBER 7
const int ALIGN_PATTERN_LOCATION_DISTANCES[QRCODE_VERSIONS+1][PATTERN_MAX_NUMBER] = {
    {-1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1},
    {6, 18, 6, 6, 6, 6, 6},
    {6, 22, 6, 6, 6, 6, 6},
    {6, 26, 6, 6, 6, 6, 6},
    {6, 30, 6, 6, 6, 6, 6},
    {6, 34, 6, 6, 6, 6, 6},
    {6, 22, 38, 6, 6, 6, 6},
    {6, 24, 42, 6, 6, 6, 6},
    {6, 26, 46, 6, 6, 6, 6},
    {6, 28, 50, 6, 6, 6, 6},
    {6, 30, 54, 6, 6, 6, 6},
    {6, 32, 58, 6, 6, 6, 6},
    {6, 34, 62, 6, 6, 6, 6},
    {6, 26, 46, 66, 6, 6, 6},
    {6, 26, 48, 70, 6, 6, 6},
    {6, 26, 50, 74, 6, 6, 6},
    {6, 30, 54, 78, 6, 6, 6},
    {6, 30, 56, 82, 6, 6, 6},
    {6, 30, 58, 86, 6, 6, 6},
    {6, 34, 62, 90, 6, 6, 6},
    {6, 28, 50, 72, 94, 6, 6},
    {6, 26, 50, 74, 98, 6, 6},
    {6, 30, 54, 78, 102, 6, 6},
    {6, 28, 54, 80, 106, 6, 6},
    {6, 32, 58, 84, 110, 6, 6},
    {6, 30, 58, 86, 114, 6, 6},
    {6, 34, 62, 90, 118, 6, 6},
    {6, 26, 50, 74, 98, 122, 6},
    {6, 30, 54, 78, 102, 126, 6},
    {6, 26, 52, 78, 104, 130, 6},
    {6, 30, 56, 82, 108, 134, 6},
    {6, 34, 60, 86, 112, 138, 6},
    {6, 30, 58, 86, 114, 142, 6},
    {6, 34, 62, 90, 118, 146, 6},
    {6, 30, 54, 78, 102, 126, 150},
    {6, 24, 50, 76, 102, 128, 154},
    {6, 28, 54, 80, 106, 132, 158},
    {6, 32, 58, 84, 110, 136, 162},
    {6, 26, 54, 82, 110, 138, 166},
    {6, 30, 58, 86, 114, 142, 170}
};

/* Converts n to binary and inserts it into destination 
 *  Es. n = 5, destination_size = 8;
 *  result = 0 0 0 0 0 1 0 1
 * */
void get_binary_from_integer(unsigned int n, unsigned char *destination, unsigned int destination_size) {
    unsigned int temp = n;
    int bit_count = 0;
    while (bit_count < destination_size) {
        if (temp > 0) {
            destination[destination_size - bit_count - 1] = temp % 2;
            temp /= 2;
        } else {
            destination[destination_size - bit_count - 1] = 0;
        }
        bit_count++;
    }
}

/* Gets an integer from a binary value of 1 Byte */
unsigned char get_integer_from_binary(unsigned char bytes[]) {
    int n = 0;
    for (int i = 0; i < BYTE_SIZE; i++ ) {
        n += pow(2, BYTE_SIZE - 1 - i) * bytes[i];
    }
    return n;
}

/* Gets the size of a qrcode from its version */
int get_qrcode_size(int version) {
    return (version * 4) + 17;
}

/* Gets the generator polynomial from the given error correction codeblocks */
void get_generator_polynomial(unsigned char destinantion[], int ec_codeblocks) {

    int generator_size = ec_codeblocks + 1;
    unsigned char temp1[generator_size];
    unsigned char temp2[generator_size];

    for (int i = 0 ; i < generator_size; i++) {
        destinantion[i] = 0;
        temp1[i] = 0;
        temp2[i] = 0;
    }

    /* Polynomial sum in 2 steps */
    for (int i = 0 ; i < ec_codeblocks; i++) {
        for (int j = generator_size - 2 - i; j < generator_size - 1; j++) {
            temp1[j] = log_lookup_table[(destinantion[j+1] + 0) % 255];
        }
        for(int j = generator_size - 1 - i; j < generator_size; j++) {
            temp2[j] = log_lookup_table[(destinantion[j] + i) % 255];
        }
        for (int j = 0; j < generator_size; j++) {
            destinantion[j] = log_reverse_lookup_table[temp1[j] ^ temp2[j]] % 255;
        }
    }
}

/* Computes the correction bits from the given message and generator polynomial */
void get_correction_words(unsigned char message_polynomial[], int message_size, unsigned char generator_polynomial[], int generator_polynomial_size, unsigned char destination[]) {

    int pol_dim = message_size + generator_polynomial_size - 1;
    unsigned char polynomial[pol_dim];
    unsigned char temp_polynomial[pol_dim];
    unsigned char temp_computation[pol_dim];

    /* Copy values */
    for (int i = 0; i < pol_dim; i++) {
        if (i <= generator_polynomial_size)
            temp_polynomial[i] = generator_polynomial[i];
        else
            temp_polynomial[i] = 0;
    }
    for (int i = 0; i < pol_dim; i++) {
        if (i < message_size)
            polynomial[i] = message_polynomial[i];
        else
            polynomial[i] = 0;
    }

    /* Compute correction bits */
    int generator_pos = 0;
    for (int i = 0; i < message_size; i++) {
        if (polynomial[i] != 0)  {
            for (int j = generator_pos; j < generator_pos + generator_polynomial_size; j++) {
                temp_computation[j] = (temp_polynomial[j] + log_reverse_lookup_table[polynomial[i]]) % 255;
            }
            for (int j = generator_pos; j < generator_pos + generator_polynomial_size; j++) {
                polynomial[j] = log_lookup_table[temp_computation[j]] ^ polynomial[j];
            }
        }
        generator_pos++;

        /* Shift temp polynomial */
        for (int j = pol_dim - 1; j > 0; j--) {
            temp_polynomial[j] = temp_polynomial[j-1];
        }
        temp_polynomial[0] = 0;
    }

    /* Fill destination with results */
    int count = 0;
    for (int i = generator_pos; i < generator_pos + generator_polynomial_size - 1; i++) {
        destination[count] = polynomial[i];
        count++;
    }

}

/* Fills the qrcode with patterns and data */
void populate_qrcode(cell_t qrcode[], unsigned char data[], int version, int correction_level, int mask) {

    int qrcode_size = get_qrcode_size(version);

    /* finder patterns */
    /* fill outer white; lock all for later */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            qrcode[qrcode_size*i + j].value = 0;
            qrcode[qrcode_size*i + j].locked = 1;
            qrcode[qrcode_size*(qrcode_size - 1 - i) + j].value = WHITE;
            qrcode[qrcode_size*(qrcode_size - 1 - i) + j].locked = LOCKED;
            qrcode[qrcode_size*(i) + qrcode_size - 1 - j].value = WHITE;
            qrcode[qrcode_size*(i) + qrcode_size - 1 - j].locked = LOCKED;
        }
    }
    /* fill outer black*/
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            qrcode[qrcode_size*i + j].value = BLACK;
            qrcode[qrcode_size*i + j].locked = LOCKED;
            qrcode[qrcode_size*(qrcode_size - 1 - i) + j].value = BLACK;
            qrcode[qrcode_size*(i) + qrcode_size - 1 - j].value = BLACK;
        }
    }
    /* fill inner white */
    for (int i = 1; i < 6; i++) {
        for (int j = 1; j < 6; j++) {
            qrcode[qrcode_size*i + j].value = WHITE;
            qrcode[qrcode_size*i + j].locked = LOCKED;
            qrcode[qrcode_size*(qrcode_size - 1 - i) + j].value = WHITE;
            qrcode[qrcode_size*(i) + qrcode_size - 1 - j].value = WHITE;
        }
    }
    /* fill inner black */
    for (int i = 2; i < 5; i++) {
        for (int j = 2; j < 5; j++) {
            qrcode[qrcode_size*i + j].value = BLACK;
            qrcode[qrcode_size*i + j].locked = LOCKED;
            qrcode[qrcode_size*(qrcode_size - 1 - i) + j].value = BLACK;
            qrcode[qrcode_size*(i) + qrcode_size - 1 - j].value = BLACK;
        }
    }

    /* Alignemnt patterns (for qrcodes with version 2 or later) */
    if (version > 1) {
    
        for (int i = 0; i < PATTERN_MAX_NUMBER; i++) {
            for (int j = 0; j < PATTERN_MAX_NUMBER; j++) {
                if (qrcode[qrcode_size*(ALIGN_PATTERN_LOCATION_DISTANCES[version][i]) + ALIGN_PATTERN_LOCATION_DISTANCES[version][j]].locked == UNLOCKED) {
                    for (int k = -2; k <= 2; k++) {
                        for (int h = -2; h <= 2; h++) {
                            qrcode[qrcode_size*(ALIGN_PATTERN_LOCATION_DISTANCES[version][i] + k) + ALIGN_PATTERN_LOCATION_DISTANCES[version][j] + h].value = BLACK;
                            qrcode[qrcode_size*(ALIGN_PATTERN_LOCATION_DISTANCES[version][i] + k) + ALIGN_PATTERN_LOCATION_DISTANCES[version][j] + h].locked = LOCKED;
                        }
                    }
                    for (int k = -1; k <= 1; k++) {
                        for (int h = -1; h <= 1; h++) {
                            qrcode[qrcode_size*(ALIGN_PATTERN_LOCATION_DISTANCES[version][i] + k) + ALIGN_PATTERN_LOCATION_DISTANCES[version][j] + h].value = WHITE;
                        }
                    }
                    qrcode[qrcode_size*(ALIGN_PATTERN_LOCATION_DISTANCES[version][i]) + ALIGN_PATTERN_LOCATION_DISTANCES[version][j]].value = BLACK;
                }
            }
        }
    }

    /* Timing patterns */
    const int alignment_pos = 6;
    for (int i = 0; i < qrcode_size; i++) {
        if (qrcode[qrcode_size*(i) + alignment_pos].locked == UNLOCKED) {
            qrcode[qrcode_size*(i) + alignment_pos].value = (i + 1) % 2;
            qrcode[qrcode_size*(i) + alignment_pos].locked = LOCKED;
        }
        if (qrcode[qrcode_size*(alignment_pos) + i].locked == UNLOCKED) {
            qrcode[qrcode_size*(alignment_pos) + i].value = (i + 1) % 2;
            qrcode[qrcode_size*(alignment_pos) + i].locked = LOCKED;
        }
    }
    
    /* Dark module and reserved areas */
    qrcode[qrcode_size*(qrcode_size - 1 - 7) + 8].value = BLACK;
    qrcode[qrcode_size*(qrcode_size - 1 - 7) + 8].locked = LOCKED;

    /* version 7 or above require a Version Information Area */
    if (version >= 7) {
        /* Lock the cells */
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 6; j++) {
                qrcode[qrcode_size*(qrcode_size - 11 + i) + j].locked = LOCKED;
                qrcode[qrcode_size*(j) + qrcode_size - 11 + i].locked = LOCKED;
            }
        }
        /* Manually put every bit in its place */
        qrcode[qrcode_size*(qrcode_size - 11 + 0) + 0].value = VERSION_INFORMATION_BITS[version][17];
        qrcode[qrcode_size*(qrcode_size - 11 + 1) + 0].value = VERSION_INFORMATION_BITS[version][16];
        qrcode[qrcode_size*(qrcode_size - 11 + 2) + 0].value = VERSION_INFORMATION_BITS[version][15];
        qrcode[qrcode_size*(qrcode_size - 11 + 0) + 1].value = VERSION_INFORMATION_BITS[version][14];
        qrcode[qrcode_size*(qrcode_size - 11 + 1) + 1].value = VERSION_INFORMATION_BITS[version][13];
        qrcode[qrcode_size*(qrcode_size - 11 + 2) + 1].value = VERSION_INFORMATION_BITS[version][12];
        qrcode[qrcode_size*(qrcode_size - 11 + 0) + 2].value = VERSION_INFORMATION_BITS[version][11];
        qrcode[qrcode_size*(qrcode_size - 11 + 1) + 2].value = VERSION_INFORMATION_BITS[version][10];
        qrcode[qrcode_size*(qrcode_size - 11 + 2) + 2].value = VERSION_INFORMATION_BITS[version][9];
        qrcode[qrcode_size*(qrcode_size - 11 + 0) + 3].value = VERSION_INFORMATION_BITS[version][8];
        qrcode[qrcode_size*(qrcode_size - 11 + 1) + 3].value = VERSION_INFORMATION_BITS[version][7];
        qrcode[qrcode_size*(qrcode_size - 11 + 2) + 3].value = VERSION_INFORMATION_BITS[version][6];
        qrcode[qrcode_size*(qrcode_size - 11 + 0) + 4].value = VERSION_INFORMATION_BITS[version][5];
        qrcode[qrcode_size*(qrcode_size - 11 + 1) + 4].value = VERSION_INFORMATION_BITS[version][4];
        qrcode[qrcode_size*(qrcode_size - 11 + 2) + 4].value = VERSION_INFORMATION_BITS[version][3];
        qrcode[qrcode_size*(qrcode_size - 11 + 0) + 5].value = VERSION_INFORMATION_BITS[version][2];
        qrcode[qrcode_size*(qrcode_size - 11 + 1) + 5].value = VERSION_INFORMATION_BITS[version][1];
        qrcode[qrcode_size*(qrcode_size - 11 + 2) + 5].value = VERSION_INFORMATION_BITS[version][0];

        qrcode[qrcode_size*(0) + qrcode_size - 11 + 0].value = VERSION_INFORMATION_BITS[version][17];
        qrcode[qrcode_size*(0) + qrcode_size - 11 + 1].value = VERSION_INFORMATION_BITS[version][16];
        qrcode[qrcode_size*(0) + qrcode_size - 11 + 2].value = VERSION_INFORMATION_BITS[version][15];
        qrcode[qrcode_size*(1) + qrcode_size - 11 + 0].value = VERSION_INFORMATION_BITS[version][14];
        qrcode[qrcode_size*(1) + qrcode_size - 11 + 1].value = VERSION_INFORMATION_BITS[version][13];
        qrcode[qrcode_size*(1) + qrcode_size - 11 + 2].value = VERSION_INFORMATION_BITS[version][12];
        qrcode[qrcode_size*(2) + qrcode_size - 11 + 0].value = VERSION_INFORMATION_BITS[version][11];
        qrcode[qrcode_size*(2) + qrcode_size - 11 + 1].value = VERSION_INFORMATION_BITS[version][10];
        qrcode[qrcode_size*(2) + qrcode_size - 11 + 2].value = VERSION_INFORMATION_BITS[version][9];
        qrcode[qrcode_size*(3) + qrcode_size - 11 + 0].value = VERSION_INFORMATION_BITS[version][8];
        qrcode[qrcode_size*(3) + qrcode_size - 11 + 1].value = VERSION_INFORMATION_BITS[version][7];
        qrcode[qrcode_size*(3) + qrcode_size - 11 + 2].value = VERSION_INFORMATION_BITS[version][6];
        qrcode[qrcode_size*(4) + qrcode_size - 11 + 0].value = VERSION_INFORMATION_BITS[version][5];
        qrcode[qrcode_size*(4) + qrcode_size - 11 + 1].value = VERSION_INFORMATION_BITS[version][4];
        qrcode[qrcode_size*(4) + qrcode_size - 11 + 2].value = VERSION_INFORMATION_BITS[version][3];
        qrcode[qrcode_size*(5) + qrcode_size - 11 + 0].value = VERSION_INFORMATION_BITS[version][2];
        qrcode[qrcode_size*(5) + qrcode_size - 11 + 1].value = VERSION_INFORMATION_BITS[version][1];
        qrcode[qrcode_size*(5) + qrcode_size - 11 + 2].value = VERSION_INFORMATION_BITS[version][0];
    }

    /* §Lock Format Information cells */
    qrcode[qrcode_size*(qrcode_size - 1) + 8].locked = LOCKED;
    qrcode[qrcode_size*(qrcode_size - 2) + 8].locked = LOCKED;
    qrcode[qrcode_size*(qrcode_size - 3) + 8].locked = LOCKED;
    qrcode[qrcode_size*(qrcode_size - 4) + 8].locked = LOCKED;
    qrcode[qrcode_size*(qrcode_size - 5) + 8].locked = LOCKED;
    qrcode[qrcode_size*(qrcode_size - 6) + 8].locked = LOCKED;
    qrcode[qrcode_size*(qrcode_size - 7) + 8].locked = LOCKED;
    qrcode[qrcode_size*(8) + qrcode_size - 8].locked = LOCKED;
    qrcode[qrcode_size*(8) + qrcode_size - 7].locked = LOCKED;
    qrcode[qrcode_size*(8) + qrcode_size - 6].locked = LOCKED;
    qrcode[qrcode_size*(8) + qrcode_size - 5].locked = LOCKED;
    qrcode[qrcode_size*(8) + qrcode_size - 4].locked = LOCKED;
    qrcode[qrcode_size*(8) + qrcode_size - 3].locked = LOCKED;
    qrcode[qrcode_size*(8) + qrcode_size - 2].locked = LOCKED;
    qrcode[qrcode_size*(8) + qrcode_size - 1].locked = LOCKED;
    qrcode[qrcode_size*(8) + 0].locked = LOCKED;
    qrcode[qrcode_size*(8) + 1].locked = LOCKED;
    qrcode[qrcode_size*(8) + 2].locked = LOCKED;
    qrcode[qrcode_size*(8) + 3].locked = LOCKED;
    qrcode[qrcode_size*(8) + 4].locked = LOCKED;
    qrcode[qrcode_size*(8) + 5].locked = LOCKED;
    qrcode[qrcode_size*(8) + 7].locked = LOCKED;
    qrcode[qrcode_size*(8) + 8].locked = LOCKED;
    qrcode[qrcode_size*(7) + 8].locked = LOCKED;
    qrcode[qrcode_size*(5) + 8].locked = LOCKED;
    qrcode[qrcode_size*(4) + 8].locked = LOCKED;
    qrcode[qrcode_size*(3) + 8].locked = LOCKED;
    qrcode[qrcode_size*(2) + 8].locked = LOCKED;
    qrcode[qrcode_size*(1) + 8].locked = LOCKED;
    qrcode[qrcode_size*(0) + 8].locked = LOCKED;

    /* Select best mask and apply it */
    int mask_penalties[MASK_NUMBER] = {0};
    bool break_from_masks = false;

    for (int current_mask = 0; current_mask < MASK_NUMBER + 1; current_mask++) {

        /* If a manual mask is selected, bypass masks computations */
        if (mask != MASK_ANY) {
            current_mask = mask;
            break_from_masks = true;
        }

        /* Compare penalties */
        if (current_mask == MASK_NUMBER) {
            int min_penalty = mask_penalties[0];
            current_mask = 0;

            int best_mask;
            for (best_mask = 0; best_mask < MASK_NUMBER; best_mask++) {
                if (min_penalty > mask_penalties[best_mask]) {
                    min_penalty = mask_penalties[best_mask];
                    current_mask = best_mask;
                }
            }
            break_from_masks = true;
        }
        
        /* Manually put the Format Information bits */
        qrcode[qrcode_size*(qrcode_size - 1) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][0];
        qrcode[qrcode_size*(qrcode_size - 2) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][1];
        qrcode[qrcode_size*(qrcode_size - 3) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][2];
        qrcode[qrcode_size*(qrcode_size - 4) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][3];
        qrcode[qrcode_size*(qrcode_size - 5) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][4];
        qrcode[qrcode_size*(qrcode_size - 6) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][5];
        qrcode[qrcode_size*(qrcode_size - 7) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][6];
        qrcode[qrcode_size*(8) + qrcode_size - 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][7];
        qrcode[qrcode_size*(8) + qrcode_size - 7].value = TYPE_INFORMATION_BITS[current_mask][correction_level][8];
        qrcode[qrcode_size*(8) + qrcode_size - 6].value = TYPE_INFORMATION_BITS[current_mask][correction_level][9];
        qrcode[qrcode_size*(8) + qrcode_size - 5].value = TYPE_INFORMATION_BITS[current_mask][correction_level][10];
        qrcode[qrcode_size*(8) + qrcode_size - 4].value = TYPE_INFORMATION_BITS[current_mask][correction_level][11];
        qrcode[qrcode_size*(8) + qrcode_size - 3].value = TYPE_INFORMATION_BITS[current_mask][correction_level][12];
        qrcode[qrcode_size*(8) + qrcode_size - 2].value = TYPE_INFORMATION_BITS[current_mask][correction_level][13];
        qrcode[qrcode_size*(8) + qrcode_size - 1].value = TYPE_INFORMATION_BITS[current_mask][correction_level][14];

        qrcode[qrcode_size*(8) + 0].value = TYPE_INFORMATION_BITS[current_mask][correction_level][0];
        qrcode[qrcode_size*(8) + 1].value = TYPE_INFORMATION_BITS[current_mask][correction_level][1];
        qrcode[qrcode_size*(8) + 2].value = TYPE_INFORMATION_BITS[current_mask][correction_level][2];
        qrcode[qrcode_size*(8) + 3].value = TYPE_INFORMATION_BITS[current_mask][correction_level][3];
        qrcode[qrcode_size*(8) + 4].value = TYPE_INFORMATION_BITS[current_mask][correction_level][4];
        qrcode[qrcode_size*(8) + 5].value = TYPE_INFORMATION_BITS[current_mask][correction_level][5];
        qrcode[qrcode_size*(8) + 7].value = TYPE_INFORMATION_BITS[current_mask][correction_level][6];
        qrcode[qrcode_size*(8) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][7];
        qrcode[qrcode_size*(7) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][8];
        qrcode[qrcode_size*(5) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][9];
        qrcode[qrcode_size*(4) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][10];
        qrcode[qrcode_size*(3) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][11];
        qrcode[qrcode_size*(2) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][12];
        qrcode[qrcode_size*(1) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][13];
        qrcode[qrcode_size*(0) + 8].value = TYPE_INFORMATION_BITS[current_mask][correction_level][14];

        /* Insert Data (it needs to be reinserted for every mask applied) */
        int i = qrcode_size - 1;
        int j = qrcode_size - 1;
        int current = 0;
        enum states{ASCENDING, DESCENDING} state = ASCENDING;
        bool is_right = true;
        while (i != qrcode_size - 1 || j != 0) {
            if (state == ASCENDING) {
                if (qrcode[qrcode_size*(i) + j].locked == UNLOCKED) {
                    qrcode[qrcode_size*(i) + j].value = data[current];
                    current++;
                }
                if (is_right) {
                    j -= 1;
                    is_right = false;
                } else {
                    j += 1;
                    is_right = true;
                    if (i != 0) {
                        i -= 1;
                    } else {
                        /* Rotate */
                        state = DESCENDING;
                        j = j == 8 ? 5 : j-2;
                    }
                }
            } else if (state == DESCENDING) {
                if (!qrcode[qrcode_size*(i) + j].locked) {
                    qrcode[qrcode_size*(i) + j].value = data[current];
                    current++;
                }
                if (is_right) {
                    j -= 1;
                    is_right = false;
                } else {
                    j += 1;
                    is_right = true;
                    if (i != qrcode_size - 1) {
                        i += 1;
                    } else {
                        /* Rotate */
                        state = ASCENDING;
                        j = j == 8 ? 5 : j-2;
                    }
                }
            }
        }

        /* Apply mask */
        switch (current_mask) {
            case 0:
                for (int i = 0; i < qrcode_size; i++) {
                    for (int j = 0; j < qrcode_size; j++) {
                        if ((i + j) % 2 == 0 && !qrcode[qrcode_size*i + j].locked)
                            qrcode[qrcode_size*i + j].value = !qrcode[qrcode_size*i + j].value;
                    }
                }
                break;
            case 1:
                for (int i = 0; i < qrcode_size; i++) {
                    for (int j = 0; j < qrcode_size; j++) {
                        if ((i) % 2 == 0 && !qrcode[qrcode_size*i + j].locked)
                            qrcode[qrcode_size*i + j].value = !qrcode[qrcode_size*i + j].value;
                    }
                }
                break;
            case 2:
                for (int i = 0; i < qrcode_size; i++) {
                    for (int j = 0; j < qrcode_size; j++) {
                        if ((j) % 3 == 0 && !qrcode[qrcode_size*i + j].locked)
                            qrcode[qrcode_size*i + j].value = !qrcode[qrcode_size*i + j].value;
                    }
                }
                break;
            case 3:
                for (int i = 0; i < qrcode_size; i++) {
                    for (int j = 0; j < qrcode_size; j++) {
                        if ((i + j) % 3 == 0 && !qrcode[qrcode_size*i + j].locked)
                            qrcode[qrcode_size*i + j].value = !qrcode[qrcode_size*i + j].value;
                    }
                }
                break;
            case 4:
                for (int i = 0; i < qrcode_size; i++) {
                    for (int j = 0; j < qrcode_size; j++) {
                        if ((int)(floor((double)i/2) + floor((double)j/3)) % 2 == 0 && !qrcode[qrcode_size*i + j].locked)
                            qrcode[qrcode_size*i + j].value = !qrcode[qrcode_size*i + j].value;
                    }
                }
                break;
            case 5:
                for (int i = 0; i < qrcode_size; i++) {
                    for (int j = 0; j < qrcode_size; j++) {
                        if (((i*j) % 2) + ((i*j) % 3) == 0 && !qrcode[qrcode_size*i + j].locked)
                            qrcode[qrcode_size*i + j].value = !qrcode[qrcode_size*i + j].value;
                    }
                }
                break;
            case 6:
                for (int i = 0; i < qrcode_size; i++) {
                    for (int j = 0; j < qrcode_size; j++) {
                        if ((((i*j) % 2) + ((i*j) % 3)) % 2 == 0 && !qrcode[qrcode_size*i + j].locked)
                            qrcode[qrcode_size*i + j].value = !qrcode[qrcode_size*i + j].value;
                    }
                }
                break;
            case 7:
                for (int i = 0; i < qrcode_size; i++) {
                    for (int j = 0; j < qrcode_size; j++) {
                        if ((((i+j) % 2) + ((i*j) % 3)) % 2 == 0 && !qrcode[qrcode_size*i + j].locked)
                            qrcode[qrcode_size*i + j].value = !qrcode[qrcode_size*i + j].value;
                    }
                }
                break;
        }

        if (break_from_masks)
            break;

        /* Compute penalties */
        /* Penalty 1: check for 5 or more blocks of the same color in a row (or column)*/
        int same_color_counter = 0;
        bool current_value = WHITE;
        for (int i = 0; i < qrcode_size; i++) {
            for (int j = 0; j < qrcode_size; j++) {
                if (qrcode[qrcode_size*(i) + j].value == current_value) {
                    same_color_counter++;
                } else {
                    if (same_color_counter >= 5)
                        mask_penalties[current_mask] += (same_color_counter - 2);
                    current_value = !current_value;
                    same_color_counter = 1;
                }
            }
            if (same_color_counter >= 5)
                mask_penalties[current_mask] += (same_color_counter - 2);
            same_color_counter = 0;
        }
        for (int j = 0; j < qrcode_size; j++) {
            for (int i = 0; i < qrcode_size; i++) {
                if (qrcode[qrcode_size*(i) + j].value == current_value) {
                    same_color_counter++;
                } else {
                    if (same_color_counter >= 5)
                        mask_penalties[current_mask] += (same_color_counter - 2);
                    current_value = !current_value;
                    same_color_counter = 1;
                }
            }
            if (same_color_counter >= 5)
                mask_penalties[current_mask] += (same_color_counter - 2);
            same_color_counter = 0;
        }

        /* Penaly 2: check for blocks of 4 squares */
        for (int i = 0; i < qrcode_size - 1; i++) {
            for (int j = 0; j < qrcode_size - 1; j++) {
                if (qrcode[qrcode_size*(i) + j].value == qrcode[qrcode_size*(i) + j + 1].value &&
                         qrcode[qrcode_size*(i) + j].value == qrcode[qrcode_size*(i + 1) + j].value &&
                         qrcode[qrcode_size*(i) + j].value == qrcode[qrcode_size*(i + 1) + j + 1].value) {
                    mask_penalties[current_mask] += 3;
                }
            }
        }

        /* Penalty 3: patterns BWBBBWBWWWW or WWWWBWBBBWB give penalties in rows or columns */
        const int PENALY_PATTERN_SIZE = 11;
        const char penalty_pattern1[] = {BLACK, WHITE, BLACK, BLACK, BLACK, WHITE, BLACK, WHITE, WHITE, WHITE, WHITE};
        const char penalty_pattern2[] = {WHITE, WHITE, WHITE, WHITE, BLACK, WHITE, BLACK, BLACK, BLACK, WHITE, BLACK};

        for (int i = 0; i < qrcode_size; i++) {
            for (int j = 0; j < qrcode_size - PENALY_PATTERN_SIZE; j++) {
                bool give_penalty = true;
                for (int p = 0; p < PENALY_PATTERN_SIZE; p++) {
                    if (qrcode[qrcode_size*(i) + j + p].value != penalty_pattern1[p]) {
                        give_penalty = false;
                        break;
                    }
                }
                if (give_penalty)
                    mask_penalties[current_mask] += 40;

                give_penalty = true;
                for (int p = 0; p < PENALY_PATTERN_SIZE; p++) {
                    if (qrcode[qrcode_size*(i) + j + p].value != penalty_pattern2[p]) {
                        give_penalty = false;
                        break;
                    }
                }
                if (give_penalty)
                    mask_penalties[current_mask] += 40;
            }
        }
        for (int i = 0; i < qrcode_size - PENALY_PATTERN_SIZE; i++) {
            for (int j = 0; j < qrcode_size; j++) {
                bool give_penalty = true;
                for (int p = 0; p < PENALY_PATTERN_SIZE; p++) {
                    if (qrcode[qrcode_size*(i + p) + j].value != penalty_pattern1[p]) {
                        give_penalty = false;
                        break;
                    }
                }
                if (give_penalty)
                    mask_penalties[current_mask] += 40;

                give_penalty = true;
                for (int p = 0; p < PENALY_PATTERN_SIZE; p++) {
                    if (qrcode[qrcode_size*(i + p) + j].value != penalty_pattern2[p]) {
                        give_penalty = false;
                        break;
                    }
                }
                if (give_penalty)
                    mask_penalties[current_mask] += 40;
            }
        }

        /* Penalty 4: based on the ratio between white and black cells */
        int black_counter = 0;
        int white_counter = 0;
        for (int i = 0; i < qrcode_size; i++) {
            for (int j = 0; j < qrcode_size; j++) {
                if (qrcode[qrcode_size*(i) + j].value == BLACK)
                    black_counter++;
                else
                    white_counter++;
            }
        }
        int black_ratio = floor(((double) black_counter / (black_counter + white_counter)) * 100);
        int candidate1 = abs(black_ratio - (black_ratio % 5) - 50);
        int candidate2 = abs(black_ratio + (5 - (black_ratio % 5)) - 50);
        mask_penalties[current_mask] += candidate1 < candidate2 ? candidate1*2 : candidate2*2;
    }
}

#define IMAGE_FACTOR 10

/* Print qrcode to ppm file */
void print_matrix(unsigned char data[], int version) {

    /* Padding is both above and below the qrcode, so add 2 to size */
    int size = get_qrcode_size(version) + 2*PADDING;

    FILE *image = fopen(IMAGE_FILE_NAME, "wb");
    fprintf(image, "P6\n");
    fprintf(image, "%d %d\n", (size)*IMAGE_FACTOR, (size)*IMAGE_FACTOR);
    fprintf(image, "255\n");
    for (int i = 0; i < size; i++) {
        for (int g = 0; g < IMAGE_FACTOR; g++) {
            for (int j = 0; j < size; j++) {
                for (int f = 0; f < IMAGE_FACTOR; f++) {
                    if (data[i*size+j] == WHITE) {
                        fprintf(image, "%c%c%c", 0xFF, 0xFF, 0xFF);
                    } else {
                        fprintf(image, "%c%c%c", 0x00, 0x00, 0x00);
                    }
                }
            }
        }
    }
    fclose(image);
}

/* Print the qrcode to terminal (the ratio of a character is usually h/w=2, so printing 2 characters should be enough to make it readable in general)*/
void print_ascii_matrix(unsigned char data[], int version) {

    /* Padding is both above and below the qrcode, so add 2 to size */
    int size = get_qrcode_size(version) + 2*PADDING;

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (data[i*size + j] == WHITE)
                 printf("██");
            else
                 printf("░░");
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char **argv) {

    /* More information about qrcode generation */
    bool debug = false;
    bool ascii_print = false;

    /* Read input arguments */
    bool manual_version_choice = false;
    int version = 1;
    int correction_level = CORRECTION_LOW;
    char *input;
    int input_length = 0;
    int mask = MASK_ANY;
    for (int argv_count = 1; argv_count < argc; argv_count++) {
        if (!strcmp(argv[argv_count], "-h") || !strcmp(argv[argv_count], "--help")) {
            printf("help: [parameters] inputfile\n-v [version (1-40)] (default: depends on input size)\n-e [correction (0-3)] (default: 0)\n-m [mask(0-7)] (default: best)\n-d (debug)\n--ascii (print to terminal instead of file)\n");
            return 0;
        } else if (!strcmp(argv[argv_count], "-d")) {
            debug = true;
        } else if (!strcmp(argv[argv_count], "-v")) {
            manual_version_choice = true;
            argv_count++;
            if (argv_count < argc) {
                version = atoi(argv[argv_count]);
                if (version <= 0 || version > QRCODE_VERSIONS)
                    version = 1;
            }
        } else if (!strcmp(argv[argv_count], "-e")) {
            argv_count++;
            if (argv_count < argc) {
                correction_level = atoi(argv[argv_count]);
                if (correction_level < CORRECTION_LOW || correction_level > CORRECTION_HIGH)
                    correction_level = CORRECTION_HIGH;
            }
        } else if (!strcmp(argv[argv_count], "-m")) {
            argv_count++;
            if (argv_count < argc) {
                mask = atoi(argv[argv_count]);
                if (mask < 0 || mask >= MASK_NUMBER)
                    mask = MASK_ANY;
            }
        } else if (!strcmp(argv[argv_count], "--ascii")){
            ascii_print = true;
        } else {
            input = argv[argv_count];
            input_length = strlen(input);
        }
    }

    /* Is user didn't specify any parameter, automatically choose them */
    if (!manual_version_choice)
        /* Find adequate qrcode for the input length */
        while (qrcode_infos[version][correction_level].character_capacity < input_length) {
            version++;
            if (version > QRCODE_VERSIONS)
                break;
        }

    if (qrcode_infos[version][correction_level].character_capacity < input_length) {
        printf("Input too large: [%d] (more than %d characters). Can't generate code...\n", input_length, qrcode_infos[version][correction_level].character_capacity);
        return 1;
    }

    if (debug) {
        printf("VERSION: [%d]\n", version);
        printf("CORRECTION LEVEL: [%d]\n", correction_level);
        if (mask == MASK_ANY)
            printf("MASK: [auto]\n");
        else
            printf("MASK: [%d]\n", mask);
        printf("\n");
    }

    int total_information_needed = BYTE_MODE_INDICATOR_SIZE + character_count_indicator_size[version] + qrcode_infos[version][correction_level].character_capacity*BYTE_SIZE + TERMINATOR_MAX_SIZE;

    /* Buffer caontaining the bits of data. */
    int information_buffer_position = 0;
    unsigned char information_buffer[total_information_needed];

    /* Insert byte mode into buffer */
    for (int i = 0; i < BYTE_MODE_INDICATOR_SIZE; i++) {
        information_buffer[information_buffer_position] = BYTE_MODE_INDICATOR[i];
        information_buffer_position++;
    }

    /* Insert input length into buffer */
    get_binary_from_integer(input_length, information_buffer + sizeof(unsigned char)*information_buffer_position, character_count_indicator_size[version]);
    information_buffer_position += character_count_indicator_size[version];

    /* Insert input into buffer */
    for (int i = 0; i < input_length; i++) {
        get_binary_from_integer(input[i], information_buffer + sizeof(unsigned char)*information_buffer_position, BYTE_SIZE);
        information_buffer_position += BYTE_SIZE;
    }

    /* The number is guarantedd to be 8n+4: add 4 0's as terminator and for making a multiple of 8 */
    for (int i = 0; i < TERMINATOR_MAX_SIZE; i++) {
        information_buffer[information_buffer_position] = 0;
        information_buffer_position++;
    }

    /* If the data is still not full, add filler bits */
    int filler_step = 0;
    for (int i = information_buffer_position/BYTE_SIZE; i < total_information_needed/BYTE_SIZE; i++) {
        get_binary_from_integer(FILLER_CHARACTERS[filler_step], information_buffer + sizeof(unsigned char)*information_buffer_position, BYTE_SIZE);
        information_buffer_position += BYTE_SIZE;
        filler_step = (filler_step + 1) % 2;
    }

    /* Reconvert the data in the buffer to get decmial numbers */
    unsigned char character_buffer[(total_information_needed/BYTE_SIZE)];
    for (int i = 0; i < total_information_needed/BYTE_SIZE; i++)
        character_buffer[i] = get_integer_from_binary(information_buffer + sizeof(unsigned char)*i*BYTE_SIZE);

    if (debug) {
        printf("Input data (+ padding):\n");
        for (int i = 0; i < total_information_needed/BYTE_SIZE; i++) {
            printf("%d ", character_buffer[i]);
        }
        printf("\n\n");
    }

    /* Pupulate lookup tables */
    log_lookup_table[0] = 1;
    for (int i = 1; i < LOOKUPTABLE_SIZE; i++) {
        log_lookup_table[i] = log_lookup_table[i - 1] * 2;
        if (log_lookup_table[i] > 255) {
            /* If number is over 255, xor it with 285 */
            log_lookup_table[i] ^= 285;
        }
    }
    for (int i = 1; i < LOOKUPTABLE_SIZE; i++) {
        log_reverse_lookup_table[log_lookup_table[i]] = i;
    }

    /* Generator polynomial */
    int generator_polynomial_size = qrcode_infos[version][correction_level].error_correction_codewords_per_block + 1;
    unsigned char generator_polynomial[generator_polynomial_size];
    get_generator_polynomial(generator_polynomial, qrcode_infos[version][correction_level].error_correction_codewords_per_block);

    if (debug) {
        printf("Generator polynomial of qrcode:\n");
        for (int i = 0; i < generator_polynomial_size; i++) {
            printf("%d ", generator_polynomial[i]);
        }
        printf("\n\n");
    }

    /* Correction blocks (variables to make the code more readable)*/
    int ecc_per_block = qrcode_infos[version][correction_level].error_correction_codewords_per_block;
    int blocks1 = qrcode_infos[version][correction_level].blocks_in_group1;
    int words_per_block1 = qrcode_infos[version][correction_level].data_codewords_per_block_in_group1;
    int blocks2 = qrcode_infos[version][correction_level].blocks_in_group2;
    int words_per_block2 = qrcode_infos[version][correction_level].data_codewords_per_block_in_group2;

    if (debug)
        printf("Group1: %d blocks, %d words per block\nGroup2: %d blocks, %d words per block\n\n", blocks1, words_per_block1, blocks2, words_per_block2);

    /* Buffer for the correction characters */
    unsigned char correction_character_buffer[ecc_per_block * (blocks1 + blocks2)];

    for (int i = 0; i < blocks1; i++) {
        get_correction_words(character_buffer + sizeof(unsigned char)*(i*words_per_block1), words_per_block1, generator_polynomial, generator_polynomial_size, correction_character_buffer + sizeof(unsigned char)*(i*ecc_per_block));
    }

    for (int i = 0; i < blocks2; i++) {
        get_correction_words(character_buffer + sizeof(unsigned char)*(words_per_block1*blocks1 + i*words_per_block2), words_per_block2, generator_polynomial, generator_polynomial_size, correction_character_buffer + sizeof(unsigned char)*(blocks1*ecc_per_block + i*ecc_per_block));
    }

    if (debug) {
        printf("Correction data:");
        for (int i = 0; i < ecc_per_block*(blocks1+blocks2); i++) {
            if ((i % ecc_per_block) == 0)
                printf("\n");
            printf("%d ", correction_character_buffer[i]);
        }
        printf("\n\n");
    }

    /* Fill final information buffer (data + error correction) */
    unsigned char qrcode_buffer[total_information_needed + ecc_per_block*(blocks1+blocks2)*BYTE_SIZE + REMAINDER_BITS_NUMER[version]];
    for (int i = 0; i < (words_per_block1 > words_per_block2 ? words_per_block1 : words_per_block2); i++) {
        /* If the second group exists, its blocks are always 1 block larger than those in block 1 */
        if (i < words_per_block1) {
            for (int j = 0; j < blocks1; j++)
                get_binary_from_integer(character_buffer[i + j*(words_per_block1)], qrcode_buffer + sizeof(unsigned char)*(i*(blocks1+blocks2) + j)*BYTE_SIZE, BYTE_SIZE);
            for (int j = 0; j < blocks2; j++)
                get_binary_from_integer(character_buffer[words_per_block1*blocks1 + i + j*(words_per_block2)], qrcode_buffer + sizeof(unsigned char)*(i*(blocks1+blocks2) + blocks1 + j)*BYTE_SIZE, BYTE_SIZE);
        } else {
            for (int j = 0; j < blocks2; j++)
                get_binary_from_integer(character_buffer[words_per_block1*blocks1 + i + j*(words_per_block2)], qrcode_buffer + sizeof(unsigned char)*(i*(blocks1+blocks2) + j)*BYTE_SIZE, BYTE_SIZE);
        }
    }
    for (int i = 0; i < ecc_per_block; i++) {
        for (int j = 0; j < (blocks1 + blocks2); j++)
            get_binary_from_integer(correction_character_buffer[i + j*(ecc_per_block)], qrcode_buffer + sizeof(unsigned char)*(blocks1*words_per_block1 + blocks2*words_per_block2 + i*(blocks1 + blocks2) + j)*BYTE_SIZE, BYTE_SIZE);
    }
    /* Add remainder bits */
    for (int i = 0; i < REMAINDER_BITS_NUMER[version]; i++) {
        qrcode_buffer[total_information_needed + ecc_per_block*(blocks1+blocks2)*BYTE_SIZE + i] = 0;
    }

    /* Matrix to populate with all qrcode data and patterns */
    cell_t qrcode[get_qrcode_size(version) * get_qrcode_size(version)];
    for (int i = 0; i < get_qrcode_size(version)*get_qrcode_size(version); i++) {
        qrcode[i].locked = UNLOCKED;
    }
    populate_qrcode(qrcode, qrcode_buffer, version, correction_level, mask);

    /* Create a qrcode grid with padding added */
    unsigned char qrcode_with_padding[(get_qrcode_size(version) + 2*PADDING) * (get_qrcode_size(version) + 2*PADDING)];
    for (int i = 0; i < get_qrcode_size(version) + 2*PADDING; i++) {
        for (int j = 0; j < get_qrcode_size(version) + 2*PADDING; j++) {
            if (i >= PADDING && j >= PADDING && i < (get_qrcode_size(version) + PADDING) && j < (get_qrcode_size(version) + PADDING))
                qrcode_with_padding[(get_qrcode_size(version) + 2*PADDING)*(i) + j] = qrcode[(get_qrcode_size(version))*(i - PADDING) + j - PADDING].value;
            else
                qrcode_with_padding[(get_qrcode_size(version) + 2*PADDING)*(i) + j] = WHITE;
        }
    }

    if (ascii_print)
        print_ascii_matrix(qrcode_with_padding, version);
    else
        print_matrix(qrcode_with_padding, version);

    return 0;
}
