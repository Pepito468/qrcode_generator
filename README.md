# qrcode_generator
QRCode generator in C
```
help: [parameters] inputfile
-v [version (1-40)] (default: depends on input size)
-e [correction (0-3)] (default: 0)
-m [mask(0-7)] (default: best)
-d (debug)
--ascii (print to terminal instead of file)
```
When executed outputs qrcode to *qrcode.ppm* in the same directory.
## To compile
```
gcc qrcode_generator.c -lm -o qrcodegen
```
Inspired by [Thonky's guide](https://www.thonky.com/qr-code-tutorial/)
