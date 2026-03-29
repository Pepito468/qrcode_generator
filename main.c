#define ENABLE_QRCODE_LIB
#include "qrcode_generator.h"

int main(int argc, char **argv) {

    /* QRCODE template */
    qrcode_template_t qrcode_template = QRCODE_TEMPLATE_DEFAULT;
    /* Output type */
    enum OUTPUT_TYPE output_type = TERMINAL;
    char *file_name = NULL;

    /* argv handling */
    for (int argv_count = 1; argv_count < argc; argv_count++) {
        if (!strcmp(argv[argv_count], "-h") || !strcmp(argv[argv_count], "--help")) {
            printf("help: [parameters] inputfile\n-v [version (1-40)] (default: depends on input size)\n-c [correction (0: Low, 1: Medium, 2: Quartile, 3: High)] (default: 0)\n-m [mask (0-7)] (default: best)\n-o [filename] (print to ppm file instead of to the terminal)\n-e [encoding (0: Numeric, 1: Alphanumeric, 2: Byte, 3: Kanji)] (default: 2)\n--negative (invert colors)\n--iso (use ISO-8859-1 instead of UTF-8 in Byte mode for compatibility)\n-d (debug: more info on qrcode process)\n");
            return 0;
        } else if (!strcmp(argv[argv_count], "-d")) {
            qrcode_template.debug = true;
        } else if (!strcmp(argv[argv_count], "-v")) {
            argv_count++;
            if (argv_count < argc) {
                qrcode_template.version = atoi(argv[argv_count]);
                if (qrcode_template.version < 1 || qrcode_template.version > QRCODE_VERSIONS)
                    qrcode_template.version = 0;
            }
        } else if (!strcmp(argv[argv_count], "-c")) {
            argv_count++;
            if (argv_count < argc) {
                qrcode_template.correction_level = atoi(argv[argv_count]);
                if (qrcode_template.correction_level < LOW || qrcode_template.correction_level > HIGH)
                    qrcode_template.correction_level = LOW;
            }
        } else if (!strcmp(argv[argv_count], "-m")) {
            argv_count++;
            if (argv_count < argc) {
                qrcode_template.mask = atoi(argv[argv_count]);
                if (qrcode_template.mask < 0 || qrcode_template.mask >= MASK_NUMBER)
                    qrcode_template.mask = MASK_ANY;
            }
        } else if (!strcmp(argv[argv_count], "-o")) {
            argv_count++;
            if (argv_count < argc) {
                output_type = FILE_PPM;
                file_name = argv[argv_count];
            }
        } else if (!strcmp(argv[argv_count], "-e")) {
            argv_count++;
            if (argv_count < argc) {
                qrcode_template.encoding_mode = atoi(argv[argv_count]);
                if (qrcode_template.encoding_mode < NUMERIC || qrcode_template.encoding_mode > KANJI)
                    qrcode_template.encoding_mode = BYTE;
            }
        } else if (!strcmp(argv[argv_count], "--negative")) {
            qrcode_template.negative = true;
        } else if (!strcmp(argv[argv_count], "--iso")) {
            qrcode_template.iso = true;
        } else {
            qrcode_template.text = argv[argv_count];
        }
    }

    unsigned char *qrcode = generate_qrcode(&qrcode_template);
    if (!qrcode)
        return 1;

    print_matrix(qrcode, qrcode_template.version, output_type, file_name);

    return 0;
}
