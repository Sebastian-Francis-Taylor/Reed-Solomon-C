# Reed-Solomon

## dependencies
- Compiler (GCC)
- Standard C library (Already on most systems)

## Installation
To install the program simply clone this repository:
```bash
git clone https://github.com/Raaaainbow/Reed-Solomon-C
```
compile the program
```bash
make clean && make
```
run the program
```
bin/rs_demo
```

## Features
This program opperates over a GF(2**8) field and can correct up to 16 errors for a message length of 255.

## Usage
To alter the encoded message, simply open `main.c` in your editor of choice, and edit the array 'encoded_message'. Make sure that you change the length depending on your message.
If you wish to change the amount of errors, or the error value, change the locations and values of the xor's in `main.c`.

## Reference
If you wish to know more of the theoretical basis for Reed-Solomon decoding, along with the method used in the repository, you can read the book 'A Course In Error-Correcting Codes' by Jørn Justesen & Tom Høholdt (ISBN: 3-03719-001-9)

## License
This project is licensed under [LICENSE](LICENSE).
