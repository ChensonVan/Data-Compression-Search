// COMP9319 2017s1 Assignment 1: Run-length encoding
// Written by Changxun Fan(z5006334) for COMP9319

#include <fstream>
#include <iostream>

using namespace std;

void rldecode(string inFilePath, string outFilePath, int option);
void print_or_write_symbol(ofstream& outFile, char symbol, int option);
void print_or_write_number(ofstream& outFile, int number, char symbol, int option);

int main(int argc, char* argv[])
{
    string inFilePath = argv[1];
    string outFilePath = "";
    int option = argc;

    if (argc == 3) {
        outFilePath = argv[2];
    }

    rldecode(inFilePath, outFilePath, option);

    return 0;
}

// This method is used to decode the inFile using rlencoding algorithm
// when the option = 2, we only print the final result to screen
// when the option = 3, we only write the final result as binary to outFile
void rldecode(string inFilePath, string outFilePath, int option) {
    ifstream inFile;
    ofstream outFile;

    inFile.open(inFilePath.c_str(), ios::in);
    outFile.open(outFilePath.c_str(), ios::out | ios::binary);

    char currentByte = inFile.get();
    char previousByte = currentByte;
    char lastCharacter;
    char nextByte;
    int runLength = 0;
    int counter = 0;        // count the number of successive number characters
    int num;

    while(!inFile.eof())
    {
        // if the first bit is 1, currentByte is a number character
        if (currentByte & 0x80) {
            // recover the first bit from 1 to 0
            num = (int)(currentByte & 0x7F);
            // shift the current number to left 7 bits, repeate 'counter' times
            for (int i = 0; i < counter; i++) {
                num = (num << 7);
            }
            // concat current numer(high 7 bits) 
            // and runLength(low bits) as a number 
            runLength = (num | runLength);
            // count the number of successive number
            counter++;
        }
        // if the first bit is 0, currentByte is a ASCII character
        else {
            if (counter > 0) {
                print_or_write_number(outFile, runLength, lastCharacter, option);
                runLength = 0;
                counter = 0;
            }
            print_or_write_symbol(outFile, currentByte, option);
            // used for print number characters
            lastCharacter = currentByte;
        }
        nextByte = inFile.get();
        previousByte = currentByte;
        currentByte = nextByte;
    }
    if (counter > 0) {
        print_or_write_number(outFile, runLength, lastCharacter, option);
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
void print_or_write_number(ofstream& outFile, int runLength, char symbol, int option) {
    int num;
    if (option == 2) {
        cout << "[" << runLength << "]";
    }
    else if (option == 3) {
        runLength += 3;
        // repeate print the symbol according to runLength
        for (int i = 1; i < runLength; i++) {
            outFile.write(&symbol, sizeof(char));
        }
    }
}
