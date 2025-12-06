#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

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
enum CORRECTION {LOW, MEDIUM, QUARTILE, HIGH};

#define ENCODING_MODES 4
enum ENCODING_MODE {NUMERIC, ALPHANUMERIC, BYTE, KANJI};

#define PADDING 4

#define MASK_NUMBER 8
#define MASK_ANY -1

#define ALIGN_PATTERN_LOCATION_SIZE 7

/* Table with the data about qrcodes I could not compute (so I just keep it here).
 * To make more sense, qrcodes go from 1 to 40 and not from 0 to 39 (the first row is filled with invalid parameters). */
typedef struct version_related_information {
    int character_capacity[ENCODING_MODES];
    int total_codewords;
    int error_correction_codewords_per_block;
    int blocks_in_group1;
    int data_codewords_per_block_in_group1;
    int blocks_in_group2;
    int data_codewords_per_block_in_group2;
} version_related_information_t;

typedef struct qrcode_information {
    int character_count_indicator_size[ENCODING_MODES];
    int remainder_bits;
    version_related_information_t correction_level_info[CORRECTION_LEVELS];
    int align_pattern_locations[ALIGN_PATTERN_LOCATION_SIZE];

} qrcode_information_t;

const qrcode_information_t QRCODE_INFO[QRCODE_VERSIONS + 1] = {
    { {-1, -1, -1, -1}, -1, { {-1, -1, -1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1, -1, -1} }, {-1, -1, -1, -1, -1, -1, -1} },
    { {10, 9, 8, 8}, 0, { { {41, 25, 17, 10}, 19, 7, 1, 19, 0, 0}, { {34, 20, 14, 8}, 16, 10, 1, 16, 0, 0}, { {27, 16, 11, 7}, 13, 13, 1, 13, 0, 0}, { {17, 10, 7, 4}, 9, 17, 1, 9, 0, 0} }, {-1, -1, -1, -1, -1, -1, -1} }, /* V1 doesn't have align patterns */
    { {10, 9, 8, 8}, 7, { {{77, 47, 32, 20}, 34, 10, 1, 34, 0, 0}, {{63, 38, 26, 16}, 28, 16, 1, 28, 0, 0}, {{48, 29, 20, 12}, 22, 22, 1, 22, 0, 0}, {{34, 20, 14, 8}, 16, 28, 1, 16, 0, 0} }, {6, 18, 6, 6, 6, 6, 6} }, /* trailing 6's in alignment pattern will be ignored */
    { {10, 9, 8, 8}, 7, { {{127, 77, 53, 32}, 55, 15, 1, 55, 0, 0}, {{101, 61, 42, 26}, 44, 26, 1, 44, 0, 0}, {{77, 47, 32, 20}, 34, 18, 2, 17, 0, 0}, {{58, 35, 24, 15}, 26, 22, 2, 13, 0, 0} }, {6, 22, 6, 6, 6, 6, 6} },
    { {10, 9, 8, 8}, 7, { {{187, 114, 78, 48}, 80, 20, 1, 80, 0, 0}, {{149, 90, 62, 38}, 64, 18, 2, 32, 0, 0}, {{111, 67, 46, 28}, 48, 26, 2, 24, 0, 0}, {{82, 50, 34, 21}, 36, 16, 4, 9, 0, 0} }, {6, 26, 6, 6, 6, 6, 6} },
    { {10, 9, 8, 8}, 7, { {{255, 154, 106, 65}, 108, 26, 1, 108, 0, 0}, {{202, 122, 84, 52}, 86, 24, 2, 43, 0, 0}, {{144, 87, 60, 37}, 62, 18, 2, 15, 2, 16}, {{106, 64, 44, 27}, 46, 22, 2, 11, 2, 12} }, {6, 30, 6, 6, 6, 6, 6} },
    { {10, 9, 8, 8}, 7, { {{322, 195, 134, 82}, 136, 18, 2, 68, 0, 0}, {{255, 154, 106, 65}, 108, 16, 4, 27, 0, 0}, {{178, 108, 74, 45}, 76, 24, 4, 19, 0, 0}, {{139, 84, 58, 36}, 60, 28, 4, 15, 0, 0} }, {6, 34, 6, 6, 6, 6, 6} },
    { {10, 9, 8, 8}, 0, { {{370, 224, 154, 95}, 156, 20, 2, 78, 0, 0}, {{293, 178, 122, 75}, 124, 18, 4, 31, 0, 0}, {{207, 125, 86, 53}, 88, 18, 2, 14, 4, 15}, {{154, 93, 64, 39}, 66, 26, 4, 13, 1, 14} }, {6, 22, 38, 6, 6, 6, 6} },
    { {10, 9, 8, 8}, 0, { {{461, 279, 192, 118}, 194, 24, 2, 97, 0, 0}, {{365, 221, 152, 93}, 154, 22, 2, 38, 2, 39}, {{259, 157, 108, 66}, 110, 22, 4, 18, 2, 19}, {{202, 122, 84, 52}, 86, 26, 4, 14, 2, 15} }, {6, 24, 42, 6, 6, 6, 6} },
    { {10, 9, 8, 8}, 0, { {{552, 335, 230, 141}, 232, 30, 2, 116, 0, 0}, {{432, 262, 180, 111}, 182, 22, 3, 36, 2, 37}, {{312, 189, 130, 80}, 132, 20, 4, 16, 4, 17}, {{235, 143, 98, 60}, 100, 24, 4, 12, 4, 13} }, {6, 26, 46, 6, 6, 6, 6} },
    { {12, 11, 16, 10}, 0, { {{652, 395, 271, 167}, 274, 18, 2, 68, 2, 69}, {{513, 311, 213, 131}, 216, 26, 4, 43, 1, 44}, {{364, 221, 151, 93}, 154, 24, 6, 19, 2, 20}, {{288, 174, 119, 74}, 122, 28, 6, 15, 2, 16} }, {6, 28, 50, 6, 6, 6, 6} },
    { {12, 11, 16, 10}, 0, { {{772, 468, 321, 198}, 324, 20, 4, 81, 0, 0}, {{604, 366, 251, 155}, 254, 30, 1, 50, 4, 51}, {{427, 259, 177, 109}, 180, 28, 4, 22, 4, 23}, {{331, 200, 137, 85}, 140, 24, 3, 12, 8, 13} }, {6, 30, 54, 6, 6, 6, 6} },
    { {12, 11, 16, 10}, 0, { {{883, 535, 367, 226}, 370, 24, 2, 92, 2, 93}, {{691, 419, 287, 177}, 290, 22, 6, 36, 2, 37}, {{489, 296, 203, 125}, 206, 26, 4, 20, 6, 21}, {{374, 227, 155, 96}, 158, 28, 7, 14, 4, 15} }, {6, 32, 58, 6, 6, 6, 6} },
    { {12, 11, 16, 10}, 0, { {{1022, 619, 425, 262}, 428, 26, 4, 107, 0, 0}, {{796, 483, 331, 204}, 334, 22, 8, 37, 1, 38}, {{580, 352, 241, 149}, 244, 24, 8, 20, 4, 21}, {{427, 259, 177, 109}, 180, 22, 12, 11, 4, 12} }, {6, 34, 62, 6, 6, 6, 6} },
    { {12, 11, 16, 10}, 3, { {{1101, 667, 458, 282}, 461, 30, 3, 115, 1, 116}, {{871, 528, 362, 223}, 365, 24, 4, 40, 5, 41}, {{621, 376, 258, 159}, 261, 20, 11, 16, 5, 17}, {{468, 283, 194, 120}, 197, 24, 11, 12, 5, 13} }, {6, 26, 46, 66, 6, 6, 6} },
    { {12, 11, 16, 10}, 3, { {{1250, 758, 520, 320}, 523, 22, 5, 87, 1, 88}, {{991, 600, 412, 254}, 415, 24, 5, 41, 5, 42}, {{703, 426, 292, 180}, 295, 30, 5, 24, 7, 25}, {{530, 321, 220, 136}, 223, 24, 11, 12, 7, 13} }, {6, 26, 48, 70, 6, 6, 6} },
    { {12, 11, 16, 10}, 3, { {{1408, 854, 586, 361}, 589, 24, 5, 98, 1, 99}, {{1082, 656, 450, 277}, 453, 28, 7, 45, 3, 46}, {{775, 470, 322, 198}, 325, 24, 15, 19, 2, 20}, {{602, 365, 250, 154}, 253, 30, 3, 15, 13, 16} }, {6, 26, 50, 74, 6, 6, 6} },
    { {12, 11, 16, 10}, 3, { {{1548, 938, 644, 397}, 647, 28, 1, 107, 5, 108}, {{1212, 734, 504, 310}, 507, 28, 10, 46, 1, 47}, {{876, 531, 364, 224}, 367, 28, 1, 22, 15, 23}, {{674, 408, 280, 173}, 283, 28, 2, 14, 17, 15} }, {6, 30, 54, 78, 6, 6, 6} },
    { {12, 11, 16, 10}, 3, { {{1725, 1046, 718, 442}, 721, 30, 5, 120, 1, 121}, {{1346, 816, 560, 345}, 563, 26, 9, 43, 4, 44}, {{948, 574, 394, 243}, 397, 28, 17, 22, 1, 23}, {{746, 452, 310, 191}, 313, 28, 2, 14, 19, 15} }, {6, 30, 56, 82, 6, 6, 6} },
    { {12, 11, 16, 10}, 3, { {{1903, 1153, 792, 488}, 795, 28, 3, 113, 4, 114}, {{1500, 909, 624, 384}, 627, 26, 3, 44, 11, 45}, {{1063, 644, 442, 272}, 445, 26, 17, 21, 4, 22}, {{813, 493, 338, 208}, 341, 26, 9, 13, 16, 14} }, {6, 30, 58, 86, 6, 6, 6} },
    { {12, 11, 16, 10}, 3, { {{2061, 1249, 858, 528}, 861, 28, 3, 107, 5, 108}, {{1600, 970, 666, 410}, 669, 26, 3, 41, 13, 42}, {{1159, 702, 482, 297}, 485, 30, 15, 24, 5, 25}, {{919, 557, 382, 235}, 385, 28, 15, 15, 10, 16} }, {6, 34, 62, 90, 6, 6, 6} },
    { {12, 11, 16, 10}, 4, { {{2232, 1352, 929, 572}, 932, 28, 4, 116, 4, 117}, {{1708, 1035, 711, 438}, 714, 26, 17, 42, 0, 0}, {{1224, 742, 509, 314}, 512, 28, 17, 22, 6, 23}, {{969, 587, 403, 248}, 406, 30, 19, 16, 6, 17} }, {6, 28, 50, 72, 94, 6, 6} },
    { {12, 11, 16, 10}, 4, { {{2409, 1460, 1003, 618}, 1006, 28, 2, 111, 7, 112}, {{1872, 1134, 779, 480}, 782, 28, 17, 46, 0, 0}, {{1358, 823, 565, 348}, 568, 30, 7, 24, 16, 25}, {{1056, 640, 439, 270}, 442, 24, 34, 13, 0, 0} }, {6, 26, 50, 74, 98, 6, 6} },
    { {12, 11, 16, 10}, 4, { {{2620, 1588, 1091, 672}, 1094, 30, 4, 121, 5, 122}, {{2059, 1248, 857, 528}, 860, 28, 4, 47, 14, 48}, {{1468, 890, 611, 376}, 614, 30, 11, 24, 14, 25}, {{1108, 672, 461, 284}, 464, 30, 16, 15, 14, 16} }, {6, 30, 54, 78, 102, 6, 6} },
    { {12, 11, 16, 10}, 4, { {{2812, 1704, 1171, 721}, 1174, 30, 6, 117, 4, 118}, {{2188, 1326, 911, 561}, 914, 28, 6, 45, 14, 46}, {{1588, 963, 661, 407}, 664, 30, 11, 24, 16, 25}, {{1228, 744, 511, 315}, 514, 30, 30, 16, 2, 17} }, {6, 28, 54, 80, 106, 6, 6} },
    { {12, 11, 16, 10}, 4, { {{3057, 1853, 1273, 784}, 1276, 26, 8, 106, 4, 107}, {{2395, 1451, 997, 614}, 1000, 28, 8, 47, 13, 48}, {{1718, 1041, 715, 440}, 718, 30, 7, 24, 22, 25}, {{1286, 779, 535, 330}, 538, 30, 22, 15, 13, 16} }, {6, 32, 58, 84, 110, 6, 6} },
    { {12, 11, 16, 10}, 4, { {{3283, 1990, 1367, 842}, 1370, 28, 10, 114, 2, 115}, {{2544, 1542, 1059, 652}, 1062, 28, 19, 46, 4, 47}, {{1804, 1094, 751, 462}, 754, 28, 28, 22, 6, 23}, {{1425, 864, 593, 365}, 596, 30, 33, 16, 4, 17} }, {6, 30, 58, 86, 114, 6, 6} },
    { {14, 13, 16, 12}, 4, { {{3517, 2132, 1465, 902}, 1468, 30, 8, 122, 4, 123}, {{2701, 1637, 1125, 692}, 1128, 28, 22, 45, 3, 46}, {{1933, 1172, 805, 496}, 808, 30, 8, 23, 26, 24}, {{1501, 910, 625, 385}, 628, 30, 12, 15, 28, 16} }, {6, 34, 62, 90, 118, 6, 6} },
    { {14, 13, 16, 12}, 3, { {{3669, 2223, 1528, 940}, 1531, 30, 3, 117, 10, 118}, {{2857, 1732, 1190, 732}, 1193, 28, 3, 45, 23, 46}, {{2085, 1263, 868, 534}, 871, 30, 4, 24, 31, 25}, {{1581, 958, 658, 405}, 661, 30, 11, 15, 31, 16} }, {6, 26, 50, 74, 98, 122, 6} },
    { {14, 13, 16, 12}, 3, { {{3909, 2369, 1628, 1002}, 1631, 30, 7, 116, 7, 117}, {{3035, 1839, 1264, 778}, 1267, 28, 21, 45, 7, 46}, {{2181, 1322, 908, 559}, 911, 30, 1, 23, 37, 24}, {{1677, 1016, 698, 430}, 701, 30, 19, 15, 26, 16} }, {6, 30, 54, 78, 102, 126, 6} },
    { {14, 13, 16, 12}, 3, { {{4158, 2520, 1732, 1066}, 1735, 30, 5, 115, 10, 116}, {{3289, 1994, 1370, 843}, 1373, 28, 19, 47, 10, 48}, {{2358, 1429, 982, 604}, 985, 30, 15, 24, 25, 25}, {{1782, 1080, 742, 457}, 745, 30, 23, 15, 25, 16} }, {6, 26, 52, 78, 104, 130, 6} },
    { {14, 13, 16, 12}, 3, { {{4417, 2677, 1840, 1132}, 1843, 30, 13, 115, 3, 116}, {{3486, 2113, 1452, 894}, 1455, 28, 2, 46, 29, 47}, {{2473, 1499, 1030, 634}, 1033, 30, 42, 24, 1, 25}, {{1897, 1150, 790, 486}, 793, 30, 23, 15, 28, 16} }, {6, 30, 56, 82, 108, 134, 6} },
    { {14, 13, 16, 12}, 3, { {{4686, 2840, 1952, 1201}, 1955, 30, 17, 115, 0, 0}, {{3693, 2238, 1538, 947}, 1541, 28, 10, 46, 23, 47}, {{2670, 1618, 1112, 684}, 1115, 30, 10, 24, 35, 25}, {{2022, 1226, 842, 518}, 845, 30, 19, 15, 35, 16} }, {6, 34, 60, 86, 112, 138, 6} },
    { {14, 13, 16, 12}, 3, { {{4965, 3009, 2068, 1273}, 2071, 30, 17, 115, 1, 116}, {{3909, 2369, 1628, 1002}, 1631, 28, 14, 46, 21, 47}, {{2805, 1700, 1168, 719}, 1171, 30, 29, 24, 19, 25}, {{2157, 1307, 898, 553}, 901, 30, 11, 15, 46, 16} }, {6, 30, 58, 86, 114, 142, 6} },
    { {14, 13, 16, 12}, 3, { {{5253, 3183, 2188, 1347}, 2191, 30, 13, 115, 6, 116}, {{4134, 2506, 1722, 1060}, 1725, 28, 14, 46, 23, 47}, {{2949, 1787, 1228, 756}, 1231, 30, 44, 24, 7, 25}, {{2301, 1394, 958, 590}, 961, 30, 59, 16, 1, 17} }, {6, 34, 62, 90, 118, 146, 6} },
    { {14, 13, 16, 12}, 0, { {{5529, 3351, 2303, 1417}, 2306, 30, 12, 121, 7, 122}, {{4343, 2632, 1809, 1113}, 1812, 28, 12, 47, 26, 48}, {{3081, 1867, 1283, 790}, 1286, 30, 39, 24, 14, 25}, {{2361, 1431, 983, 605}, 986, 30, 22, 15, 41, 16} }, {6, 30, 54, 78, 102, 126, 150} },
    { {14, 13, 16, 12}, 0, { {{5836, 3537, 2431, 1496}, 2434, 30, 6, 121, 14, 122}, {{4588, 2780, 1911, 1176}, 1914, 28, 6, 47, 34, 48}, {{3244, 1966, 1351, 832}, 1354, 30, 46, 24, 10, 25}, {{2524, 1530, 1051, 647}, 1054, 30, 2, 15, 64, 16} }, {6, 24, 50, 76, 102, 128, 154} },
    { {14, 13, 16, 12}, 0, { {{6153, 3729, 2563, 1577}, 2566, 30, 17, 122, 4, 123}, {{4775, 2894, 1989, 1224}, 1992, 28, 29, 46, 14, 47}, {{3417, 2071, 1423, 876}, 1426, 30, 49, 24, 10, 25}, {{2625, 1591, 1093, 673}, 1096, 30, 24, 15, 46, 16} }, {6, 28, 54, 80, 106, 132, 158} },
    { {14, 13, 16, 12}, 0, { {{6479, 3927, 2699, 1661}, 2702, 30, 4, 122, 18, 123}, {{5039, 3054, 2099, 1292}, 2102, 28, 13, 46, 32, 47}, {{3599, 2181, 1499, 923}, 1502, 30, 48, 24, 14, 25}, {{2735, 1658, 1139, 701}, 1142, 30, 42, 15, 32, 16} }, {6, 32, 58, 84, 110, 136, 162} },
    { {14, 13, 16, 12}, 0, { {{6743, 4087, 2809, 1729}, 2812, 30, 20, 117, 4, 118}, {{5313, 3220, 2213, 1362}, 2216, 28, 40, 47, 7, 48}, {{3791, 2298, 1579, 972}, 1582, 30, 43, 24, 22, 25}, {{2927, 1774, 1219, 750}, 1222, 30, 10, 15, 67, 16} }, {6, 26, 54, 82, 110, 138, 166} },
    { {14, 13, 16, 12}, 0, { {{7089, 4296, 2953, 1817}, 2956, 30, 19, 118, 6, 119}, {{5596, 3391, 2331, 1435}, 2334, 28, 18, 47, 31, 48}, {{3993, 2420, 1663, 1024}, 1666, 30, 34, 24, 34, 25}, {{3057, 1852, 1273, 784}, 1276, 30, 20, 15, 61, 16} }, {6, 30, 58, 86, 114, 142, 170} },
};

/* Lookup tables for correction computation */
#define LOOKUPTABLE_SIZE 256
int log_lookup_table[LOOKUPTABLE_SIZE];
int log_reverse_lookup_table[LOOKUPTABLE_SIZE];

/* Encoding modes */
#define MODE_INDICATOR_SIZE 4
const unsigned char MODE_INDICATOR[ENCODING_MODES][MODE_INDICATOR_SIZE] = {
    {0, 0, 0, 1},
    {0, 0, 1, 0},
    {0, 1, 0, 0},
    {1, 0, 0, 0},
};

/* Terminator max possible size */
#define TERMINATOR_MAX_SIZE 4
/* Filler characters to add to data after the input if space is still not filled */
const int FILLER_CHARACTERS[] = {236, 17};

/* Format data constants */
#define FORMAT_INFORMATION_BITS_SIZE 15
#define ERROR_CORRECTION_LEVEL_BITS_SIZE 2
#define MASK_LEVEL_BITS_SIZE 3
#define FORMAT_POSITION_THRESHOLD 5
const unsigned char ERROR_CORRECTION_LEVEL_BITS[CORRECTION_LEVELS][ERROR_CORRECTION_LEVEL_BITS_SIZE] = { {0, 1}, {0, 0}, {1, 1}, {1, 0} };
#define FORMAT_STRING_GENERATOR_POLYNOMIAL_SIZE 11
const unsigned char FORMAT_INFORMATION_GENERATOR_POLYNOMIAL[FORMAT_STRING_GENERATOR_POLYNOMIAL_SIZE] = {1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1};
const unsigned char FORMAT_INFORMATION_MASK_STRING[FORMAT_INFORMATION_BITS_SIZE] = {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0};

/* Version information constants */
#define VERSION_INFORMATION_BITS_SIZE 18
#define VERSION_BITS_SIZE 6
#define VERSION_INFORMATION_GENERATOR_POLYNOMIAL_SIZE 13
#define VERSION_POSITION_THRESHOLD 6
const unsigned char VERSION_INFORMATION_GENERATOR_POLYNOMIAL[VERSION_INFORMATION_GENERATOR_POLYNOMIAL_SIZE] = {1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1};

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
        for (int i = 0; i < ALIGN_PATTERN_LOCATION_SIZE; i++) {
            for (int j = 0; j < ALIGN_PATTERN_LOCATION_SIZE; j++) {
                if (qrcode[qrcode_size*(QRCODE_INFO[version].align_pattern_locations[i]) + QRCODE_INFO[version].align_pattern_locations[j]].locked == UNLOCKED) {
                    for (int k = -2; k <= 2; k++) {
                        for (int h = -2; h <= 2; h++) {
                            qrcode[qrcode_size*(QRCODE_INFO[version].align_pattern_locations[i] + k) + QRCODE_INFO[version].align_pattern_locations[j] + h].locked = LOCKED;
                            qrcode[qrcode_size*(QRCODE_INFO[version].align_pattern_locations[i] + k) + QRCODE_INFO[version].align_pattern_locations[j] + h].value = BLACK;
                        }
                    }
                    for (int k = -1; k <= 1; k++) {
                        for (int h = -1; h <= 1; h++) {
                            qrcode[qrcode_size*(QRCODE_INFO[version].align_pattern_locations[i] + k) + QRCODE_INFO[version].align_pattern_locations[j] + h].value = WHITE;
                        }
                    }
                    qrcode[qrcode_size*(QRCODE_INFO[version].align_pattern_locations[i]) + QRCODE_INFO[version].align_pattern_locations[j]].value = BLACK;
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

        /* Put version bits */
        unsigned char version_bits[VERSION_INFORMATION_BITS_SIZE];
        get_binary_from_integer(version, version_bits , VERSION_BITS_SIZE);
        for (int i = VERSION_BITS_SIZE; i < VERSION_INFORMATION_BITS_SIZE; i++) {
            version_bits[i] = 0;
        }

        /* Compute correction bits */
        int current_position = 0;
        while (version_bits[current_position] == 0)
            current_position++;
        while (current_position < VERSION_POSITION_THRESHOLD) {
            for (int i = current_position; i < VERSION_INFORMATION_BITS_SIZE; i++) {
                if (i - current_position < VERSION_INFORMATION_GENERATOR_POLYNOMIAL_SIZE)
                    version_bits[i] ^= VERSION_INFORMATION_GENERATOR_POLYNOMIAL[i - current_position];
                else
                    version_bits[i] ^= 0;
            }
            while (version_bits[current_position] == 0)
                current_position++;

        }
        /* Put the bits again, as the correction bits should be right after these */
        get_binary_from_integer(version, version_bits, VERSION_BITS_SIZE);

        /* Manually put every bit in its place */
        qrcode[qrcode_size*(qrcode_size - 11 + 0) + 0].value = version_bits[17];
        qrcode[qrcode_size*(qrcode_size - 11 + 1) + 0].value = version_bits[16];
        qrcode[qrcode_size*(qrcode_size - 11 + 2) + 0].value = version_bits[15];
        qrcode[qrcode_size*(qrcode_size - 11 + 0) + 1].value = version_bits[14];
        qrcode[qrcode_size*(qrcode_size - 11 + 1) + 1].value = version_bits[13];
        qrcode[qrcode_size*(qrcode_size - 11 + 2) + 1].value = version_bits[12];
        qrcode[qrcode_size*(qrcode_size - 11 + 0) + 2].value = version_bits[11];
        qrcode[qrcode_size*(qrcode_size - 11 + 1) + 2].value = version_bits[10];
        qrcode[qrcode_size*(qrcode_size - 11 + 2) + 2].value = version_bits[9];
        qrcode[qrcode_size*(qrcode_size - 11 + 0) + 3].value = version_bits[8];
        qrcode[qrcode_size*(qrcode_size - 11 + 1) + 3].value = version_bits[7];
        qrcode[qrcode_size*(qrcode_size - 11 + 2) + 3].value = version_bits[6];
        qrcode[qrcode_size*(qrcode_size - 11 + 0) + 4].value = version_bits[5];
        qrcode[qrcode_size*(qrcode_size - 11 + 1) + 4].value = version_bits[4];
        qrcode[qrcode_size*(qrcode_size - 11 + 2) + 4].value = version_bits[3];
        qrcode[qrcode_size*(qrcode_size - 11 + 0) + 5].value = version_bits[2];
        qrcode[qrcode_size*(qrcode_size - 11 + 1) + 5].value = version_bits[1];
        qrcode[qrcode_size*(qrcode_size - 11 + 2) + 5].value = version_bits[0];

        qrcode[qrcode_size*(0) + qrcode_size - 11 + 0].value = version_bits[17];
        qrcode[qrcode_size*(0) + qrcode_size - 11 + 1].value = version_bits[16];
        qrcode[qrcode_size*(0) + qrcode_size - 11 + 2].value = version_bits[15];
        qrcode[qrcode_size*(1) + qrcode_size - 11 + 0].value = version_bits[14];
        qrcode[qrcode_size*(1) + qrcode_size - 11 + 1].value = version_bits[13];
        qrcode[qrcode_size*(1) + qrcode_size - 11 + 2].value = version_bits[12];
        qrcode[qrcode_size*(2) + qrcode_size - 11 + 0].value = version_bits[11];
        qrcode[qrcode_size*(2) + qrcode_size - 11 + 1].value = version_bits[10];
        qrcode[qrcode_size*(2) + qrcode_size - 11 + 2].value = version_bits[9];
        qrcode[qrcode_size*(3) + qrcode_size - 11 + 0].value = version_bits[8];
        qrcode[qrcode_size*(3) + qrcode_size - 11 + 1].value = version_bits[7];
        qrcode[qrcode_size*(3) + qrcode_size - 11 + 2].value = version_bits[6];
        qrcode[qrcode_size*(4) + qrcode_size - 11 + 0].value = version_bits[5];
        qrcode[qrcode_size*(4) + qrcode_size - 11 + 1].value = version_bits[4];
        qrcode[qrcode_size*(4) + qrcode_size - 11 + 2].value = version_bits[3];
        qrcode[qrcode_size*(5) + qrcode_size - 11 + 0].value = version_bits[2];
        qrcode[qrcode_size*(5) + qrcode_size - 11 + 1].value = version_bits[1];
        qrcode[qrcode_size*(5) + qrcode_size - 11 + 2].value = version_bits[0];
    }

    /* Lock Format Information cells */
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

        /* Get Format information bits */
        unsigned char format_bits[FORMAT_INFORMATION_BITS_SIZE];
        for (int i = 0; i < ERROR_CORRECTION_LEVEL_BITS_SIZE; i++)
            format_bits[i] = ERROR_CORRECTION_LEVEL_BITS[correction_level][i];
        get_binary_from_integer(current_mask, format_bits + ERROR_CORRECTION_LEVEL_BITS_SIZE, MASK_LEVEL_BITS_SIZE);
        for (int i = ERROR_CORRECTION_LEVEL_BITS_SIZE + MASK_LEVEL_BITS_SIZE; i < FORMAT_INFORMATION_BITS_SIZE; i++) {
            format_bits[i] = 0;
        }

        /* Compute correction bits */
        int current_position = 0;
        while (format_bits[current_position] == 0)
            current_position++;
        while (current_position < FORMAT_POSITION_THRESHOLD) {
            for (int i = current_position; i < FORMAT_INFORMATION_BITS_SIZE; i++) {
                if (i - current_position < FORMAT_STRING_GENERATOR_POLYNOMIAL_SIZE)
                    format_bits[i] ^= FORMAT_INFORMATION_GENERATOR_POLYNOMIAL[i - current_position];
                else
                    format_bits[i] ^= 0;
            }
            while (format_bits[current_position] == 0)
                current_position++;

        }
        /* Put initial bits again as the correction bits should already be after them */
        for (int i = 0; i < ERROR_CORRECTION_LEVEL_BITS_SIZE; i++)
            format_bits[i] = ERROR_CORRECTION_LEVEL_BITS[correction_level][i];
        get_binary_from_integer(current_mask, format_bits + ERROR_CORRECTION_LEVEL_BITS_SIZE, MASK_LEVEL_BITS_SIZE);

        /* Apply fixed mask */
        for (int i = 0; i < FORMAT_INFORMATION_BITS_SIZE; i++) {
            format_bits[i] ^= FORMAT_INFORMATION_MASK_STRING[i];
        }

        /* Manually put the Format Information bits */
        qrcode[qrcode_size*(qrcode_size - 1) + 8].value = format_bits[0];
        qrcode[qrcode_size*(qrcode_size - 2) + 8].value = format_bits[1];
        qrcode[qrcode_size*(qrcode_size - 3) + 8].value = format_bits[2];
        qrcode[qrcode_size*(qrcode_size - 4) + 8].value = format_bits[3];
        qrcode[qrcode_size*(qrcode_size - 5) + 8].value = format_bits[4];
        qrcode[qrcode_size*(qrcode_size - 6) + 8].value = format_bits[5];
        qrcode[qrcode_size*(qrcode_size - 7) + 8].value = format_bits[6];
        qrcode[qrcode_size*(8) + qrcode_size - 8].value = format_bits[7];
        qrcode[qrcode_size*(8) + qrcode_size - 7].value = format_bits[8];
        qrcode[qrcode_size*(8) + qrcode_size - 6].value = format_bits[9];
        qrcode[qrcode_size*(8) + qrcode_size - 5].value = format_bits[10];
        qrcode[qrcode_size*(8) + qrcode_size - 4].value = format_bits[11];
        qrcode[qrcode_size*(8) + qrcode_size - 3].value = format_bits[12];
        qrcode[qrcode_size*(8) + qrcode_size - 2].value = format_bits[13];
        qrcode[qrcode_size*(8) + qrcode_size - 1].value = format_bits[14];

        qrcode[qrcode_size*(8) + 0].value = format_bits[0];
        qrcode[qrcode_size*(8) + 1].value = format_bits[1];
        qrcode[qrcode_size*(8) + 2].value = format_bits[2];
        qrcode[qrcode_size*(8) + 3].value = format_bits[3];
        qrcode[qrcode_size*(8) + 4].value = format_bits[4];
        qrcode[qrcode_size*(8) + 5].value = format_bits[5];
        qrcode[qrcode_size*(8) + 7].value = format_bits[6];
        qrcode[qrcode_size*(8) + 8].value = format_bits[7];
        qrcode[qrcode_size*(7) + 8].value = format_bits[8];
        qrcode[qrcode_size*(5) + 8].value = format_bits[9];
        qrcode[qrcode_size*(4) + 8].value = format_bits[10];
        qrcode[qrcode_size*(3) + 8].value = format_bits[11];
        qrcode[qrcode_size*(2) + 8].value = format_bits[12];
        qrcode[qrcode_size*(1) + 8].value = format_bits[13];
        qrcode[qrcode_size*(0) + 8].value = format_bits[14];

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
void print_matrix_to_file(unsigned char data[], int version, char *file_name) {

    /* Padding is both above and below the qrcode, so add 2 to size */
    int size = get_qrcode_size(version) + 2*PADDING;

    FILE *image = fopen(file_name, "wb");
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
void print_matrix(unsigned char data[], int version) {

    /* Padding is both above and below the qrcode, so add 2 to size */
    int size = get_qrcode_size(version) + 2*PADDING;

    printf("\n");
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

    /* File printing */
    bool print_to_file = false;
    char *file_name = NULL;

    /* Read input arguments */
    bool manual_version_choice = false;

    /* Invert colors */
    bool negative_image = false;

    /* Defaults */
    int version = 1;
    enum CORRECTION correction_level = LOW;
    enum ENCODING_MODE encoding = BYTE;

    char *input;
    int input_length = 0;
    int mask = MASK_ANY;
    for (int argv_count = 1; argv_count < argc; argv_count++) {
        if (!strcmp(argv[argv_count], "-h") || !strcmp(argv[argv_count], "--help")) {
            printf("help: [parameters] inputfile\n-v [version (1-40)] (default: depends on input size)\n-c [correction (0: Low, 1: Medium, 2: Quartile, 3: High)] (default: 0)\n-m [mask (0-7)] (default: best)\n-o [filename] (print to ppm file instead of to the terminal)\n-e [encoding (0: Numeric, 1: Alphanumeric, 2: Byte, 3: Kanji)] (default: 2)\n--negative (invert colors)\n-d (debug: more info on qrcode process)\n");
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
        } else if (!strcmp(argv[argv_count], "-c")) {
            argv_count++;
            if (argv_count < argc) {
                correction_level = atoi(argv[argv_count]);
                if (correction_level < LOW || correction_level > HIGH)
                    correction_level = LOW;
            }
        } else if (!strcmp(argv[argv_count], "-m")) {
            argv_count++;
            if (argv_count < argc) {
                mask = atoi(argv[argv_count]);
                if (mask < 0 || mask >= MASK_NUMBER)
                    mask = MASK_ANY;
            }
        } else if (!strcmp(argv[argv_count], "-o")) {
            argv_count++;
            if (argv_count < argc) {
                print_to_file = true;
                file_name = argv[argv_count];
            }
        } else if (!strcmp(argv[argv_count], "-e")) {
            argv_count++;
            if (argv_count < argc) {
                encoding = atoi(argv[argv_count]);
                if (encoding < NUMERIC || encoding > KANJI)
                    encoding = BYTE;
            }
        } else if (!strcmp(argv[argv_count], "--negative")) {
            negative_image = true;
        } else {
            input = argv[argv_count];
            input_length = strlen(input);
        }
    }

    /* Is user didn't specify any parameter, automatically choose them */
    if (!manual_version_choice)
        /* Find adequate qrcode for the input length */
        while (QRCODE_INFO[version].correction_level_info[correction_level].character_capacity[encoding] < input_length) {
            version++;
            if (version > QRCODE_VERSIONS)
                break;
        }

    if (QRCODE_INFO[version].correction_level_info[correction_level].character_capacity[encoding] < input_length) {
        printf("Input too large: [%d] (more than %d characters). Can't generate code...\n", input_length, QRCODE_INFO[version].correction_level_info[correction_level].character_capacity[encoding]);
        return 1;
    }

    if (debug) {
        printf("VERSION: [%d]\n", version);
        printf("CORRECTION LEVEL: [");
        switch (correction_level) {
            case LOW:
                printf("Low");
                break;
            case MEDIUM:
                printf("Medium");
                break;
            case QUARTILE:
                printf("Quartile");
                break;
            case HIGH:
                printf("High");
                break;
        }
        printf("]\n");

        if (mask == MASK_ANY)
            printf("MASK: [auto]\n");
        else
            printf("MASK: [%d]\n", mask);

        printf("Encoding: [");
        switch (encoding) {
            case NUMERIC:
                printf("Numeric");
                break;
            case ALPHANUMERIC:
                printf("Alphanumeric");
                break;
            case BYTE:
                printf("Byte");
                break;
            case KANJI:
                printf("Kanji");
                break;
        }
        printf("]\n");
        printf("\n");
    }

    int total_information_needed = QRCODE_INFO[version].correction_level_info[correction_level].total_codewords * BYTE_SIZE;

    /* Buffer caontaining the bits of data. */
    int information_buffer_position = 0;
    unsigned char information_buffer[total_information_needed];

    /* Insert byte mode into buffer */
    for (int i = 0; i < MODE_INDICATOR_SIZE; i++) {
        information_buffer[information_buffer_position] = MODE_INDICATOR[encoding][i];
        information_buffer_position++;
    }

    /* Insert input length into buffer */
    get_binary_from_integer(input_length, information_buffer + sizeof(unsigned char)*information_buffer_position, QRCODE_INFO[version].character_count_indicator_size[encoding]);
    information_buffer_position += QRCODE_INFO[version].character_count_indicator_size[encoding];

    /* Insert input into buffer */
    int current_number = 0; /* Needed for some computations */

    switch (encoding) {
        case NUMERIC:
            for (int i = 0; i < input_length; i++) {
                if (input[i] < '0' || input[i] > '9') {
                    printf("Invalid character for Numeric encoding found: [%c].\n", input[i]);
                    return 1;
                }
                current_number = current_number*10 + (input[i] - '0');

                if ((i+1) % 3 == 0 || i == (input_length - 1)) {
                    int data_size = (i+1) % 3 == 0 ? 10 : ((i+1) % 3 == 2 ? 7 : 4);
                    get_binary_from_integer(current_number, information_buffer + sizeof(unsigned char)*information_buffer_position, data_size);
                    information_buffer_position += data_size;
                    current_number = 0;
                }
            }
            break;

        case ALPHANUMERIC:
            for (int i = 0; i < input_length; i++) {
                unsigned char current_character = input[i];
                if (current_character >= '0' && current_character <= '9')
                    current_number = current_number*45 + (current_character - '0');
                else if (current_character >= 'A' && current_character <= 'Z')
                    current_number = current_number*45 + (current_character - 'A') + 10;
                else if (current_character == ' ')
                    current_number = current_number*45 + 36;
                else if (current_character == '$')
                    current_number = current_number*45 + 37;
                else if (current_character == '%')
                    current_number = current_number*45 + 38;
                else if (current_character == '*')
                    current_number = current_number*45 + 39;
                else if (current_character == '+')
                    current_number = current_number*45 + 40;
                else if (current_character == '-')
                    current_number = current_number*45 + 41;
                else if (current_character == '.')
                    current_number = current_number*45 + 42;
                else if (current_character == '/')
                    current_number = current_number*45 + 43;
                else if (current_character == ':')
                    current_number = current_number*45 + 44;
                else {
                    printf("Invalid character for Alphanumeric encoding found: [%c].\n", current_character);
                    return 1;
                }

                if ((i+1) % 2 == 0 || i == (input_length - 1)) {
                    int data_size = (i+1) % 2 == 0 ? 11 : 6;
                    get_binary_from_integer(current_number, information_buffer + sizeof(unsigned char)*information_buffer_position, data_size);
                    information_buffer_position += data_size;
                    current_number = 0;
                }
            }
            break;

        case BYTE:
            for (int i = 0; i < input_length; i++) {
                get_binary_from_integer(input[i], information_buffer + sizeof(unsigned char)*information_buffer_position, BYTE_SIZE);
                information_buffer_position += BYTE_SIZE;
            }
            break;

        case KANJI:
            printf("Kanji encoding not implemented.\n");
            return 1;
            break;
    }

    /* Add terminator */
    for (int i = 0; i < TERMINATOR_MAX_SIZE && information_buffer_position < total_information_needed; i++) {
        information_buffer[information_buffer_position] = 0;
        information_buffer_position++;
    }

    /* Add 0's until the buffer size is a multiple of 8 */
    while (information_buffer_position % 8 != 0) {
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
    int generator_polynomial_size = QRCODE_INFO[version].correction_level_info[correction_level].error_correction_codewords_per_block + 1;
    unsigned char generator_polynomial[generator_polynomial_size];
    int ecc_per_block = QRCODE_INFO[version].correction_level_info[correction_level].error_correction_codewords_per_block;
    get_generator_polynomial(generator_polynomial, ecc_per_block);

    if (debug) {
        printf("Generator polynomial of qrcode:\n");
        for (int i = 0; i < generator_polynomial_size; i++) {
            printf("%d ", generator_polynomial[i]);
        }
        printf("\n\n");
    }

    /* Correction blocks (variables to make the code more readable)*/
    int blocks1 = QRCODE_INFO[version].correction_level_info[correction_level].blocks_in_group1;
    int words_per_block1 = QRCODE_INFO[version].correction_level_info[correction_level].data_codewords_per_block_in_group1;
    int blocks2 = QRCODE_INFO[version].correction_level_info[correction_level].blocks_in_group2;
    int words_per_block2 = QRCODE_INFO[version].correction_level_info[correction_level].data_codewords_per_block_in_group2;

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
    unsigned char qrcode_buffer[total_information_needed + ecc_per_block*(blocks1+blocks2)*BYTE_SIZE + QRCODE_INFO[version].remainder_bits];
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
    for (int i = 0; i < QRCODE_INFO[version].remainder_bits; i++) {
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

    if (negative_image)
        for (int i = 0; i < get_qrcode_size(version) + 2*PADDING; i++) {
            for (int j = 0; j < get_qrcode_size(version) + 2*PADDING; j++) {
                qrcode_with_padding[(get_qrcode_size(version) + 2*PADDING)*(i) + j] = !qrcode_with_padding[(get_qrcode_size(version) + 2*PADDING)*(i) + j];
            }
        }

    if (print_to_file)
        print_matrix_to_file(qrcode_with_padding, version, file_name);
    else
        print_matrix(qrcode_with_padding, version);

    return 0;
}
