# qrcode_generator
QRCode generator in C
```
help: [parameters] inputfile
-v [version (1-40)] (default: depends on input size)
-e [correction (0-3)] (default: 0)
-m [mask (0-7)] (default: best)
-o [filename] (print to ppm file instead of to the terminal)
-d (debug: more info on qrcode process)
```
*Only uses byte mode*.
## To compile
```
gcc qrcode_generator.c -lm -o qrcodegen
```
Made following [Thonky's guide](https://www.thonky.com/qr-code-tutorial/)
