# qrcode_generator
QRCode generator in C
```
help: [parameters] inputfile
-v [version (1-40)] (default: depends on input size)
-c [correction (0: Low, 1: Medium, 2: Quartile, 3: High)] (default: 0)
-m [mask (0-7)] (default: best)
-o [filename] (print to ppm file instead of to the terminal)
-e [encoding (0: Numeric, 1: Alphanumeric, 2: Byte, 3: Kanji)] (default: 2)
--negative (invert colors)
--iso (use ISO-8859-1 instead of UTF-8 in Byte mode for compatibility)
-d (debug: more info on qrcode process)
```
The program may or may not work on a non-UTF-8 locale system.
## To compile
```
gcc qrcode_generator.c -lm -o qrcodegen
```
On some systems *iconv* might need to be linked
```
gcc qrcode_generator.c -lm -liconv -o qrcodegen
```
Made following [Thonky's guide](https://www.thonky.com/qr-code-tutorial/)
