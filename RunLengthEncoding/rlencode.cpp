// COMP9319 2017s1 Assignment 1: Run-length encoding
// Written by Changxun Fan(z5006334) for COMP9319

#include <fstream>
#include <iostream>

using namespace std;

void rlencode(string inFilePath, string outFilePath, int option);
void print_or_write_symbol(ofstream& outFile, char symbol, int option);
void print_or_write_number(ofstream& outFile, int number, int option);

int main(int argc, char* argv[])
{
    string inFilePath = argv[1];
    string outFilePath = "";
    int option = argc;

    if (argc == 3) {
        outFilePath = argv[2];
    }

    rlencode(inFilePath, outFilePath, option);

    return 0;
}

// This method is used to encode the inFile using rlencoding algorithm
// when the option = 2, we only print the final result to screen
// when the option = 3, we only write the final result as binary to outFile
void rlencode(string inFilePath, string outFilePath, int option) {
    ifstream inFile;
    ofstream outFile;

    inFile.open(inFilePath.c_str(), ios::in);
    outFile.open(outFilePath.c_str(), ios::out | ios::binary);

    char currentByte = inFile.get();
    char previousByte = currentByte;
    char nextByte;
    int runLength = 1;
    currentByte = inFile.get();

    // when there is only one character in input file
    if (inFile.eof() || currentByte == EOF) {
        print_or_write_symbol(outFile, previousByte, option);
    }

    while(!inFile.eof())
    {
        nextByte = inFile.get();

        // if current bytes is equal to the previous byte
        if (currentByte == previousByte) {
            runLength++;
        }

        if(currentByte != previousByte || nextByte == EOF) {
            // only print or write symbol once if runLength = 1
            if (runLength == 1) {
                print_or_write_symbol(outFile, previousByte, option);
            }
            // print or write symbol twice if runLength = 2
            else if (runLength == 2)
            {
                print_or_write_symbol(outFile, previousByte, option);
                print_or_write_symbol(outFile, previousByte, option);
            }
            // print or write symbol using rle notation if runLength > 2
            else {
                print_or_write_symbol(outFile, previousByte, option);
                print_or_write_number(outFile, runLength, option);
            }
            if ((previousByte != currentByte) && (nextByte == EOF)) {
                print_or_write_symbol(outFile, currentByte, option);
            }
            runLength = 1;
        }
        previousByte = currentByte;
        currentByte = nextByte;
    }

    outFile.close();
    inFile.close();
}


// This method is used to print the symbol or write it to outFile
// option = 2, we only print the final result to screen
// option = 3, we only write the final result as binary to outFile
void print_or_write_symbol(ofstream& outFile, char symbol, int option) {
    if (option == 2) {
        cout << symbol;
    }
    else if (option == 3) {
        outFile.write(&symbol, sizeof(char));
    }
}

// This method is used to print the rle notation or write it to outFile
// option = 2, we only print the final result to screen
// option = 3, we only write the final result as binary to outFile
void print_or_write_number(ofstream& outFile, int runLength, int option) {
    int num;
    runLength -= 3;
    if (option == 2) {
        cout << "[" << runLength << "]";
    }
    else if (option == 3) {
        // decompose the runLength into more than one bit and wirte it to outFile
        // shift the runLength from low bit to high bit
        if (runLength > 0) {
            while (runLength > 0) {
                num = (runLength & 0x7F);
                num = (num | 0x80);
                runLength = (runLength >> 7);
                outFile.put(num);
            }
        }
        // only wirte that if the number can be encoded into one bit
        else {
            num = (runLength | 0x80);
            outFile.put(num);
        }
    }
}
