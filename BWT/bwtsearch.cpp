#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

using namespace std;

const int offSet = 0;                       // define offSet
const int sizeOfIndex = 128 - offSet;       // define the size of index
const int sizeOfSegment = 20000;            // define the size of segment 

int numOfSegment = 8000;                    // define the default number of segment
int numOfQuery = 1;                         // define the default number of query
int segCounter[sizeOfIndex] = {0};          // define the counter for calculating occ in Prefix[1, p]
char buffer[sizeOfSegment];                 // define the buffer for reading a segment from file
int **pOcc;                                 // size = numOfSegment x sizeOfIndex

string inFilePath;
string indexPath;
FILE *pFile;

void init();

void backward_search(string *query);
int binary_search(int count, char c);
string forward_decode(int pos);
string backward_decode(int pos);

int get_numOfSegment();
int get_identity(string match);
string get_sentence(string match);
char get_character_from_BWT(int pos);
char get_character_from_orderedBWT(int pos);
int get_th_in_orderedBWT(int pos, char c);
int get_count(int c);
int get_occ(int c);

void calculate_occ(int endPosition);
int calculate_inverse_occ(int count, char c);


int main(int argc, char* argv[]) {
    inFilePath = argv[1];
    indexPath  = argv[2];
    numOfQuery = argc - 3;
    pFile = fopen(inFilePath.c_str(), "rb" );

    string *query = new string[numOfQuery];
    for (int i = 0; i < numOfQuery; i++) {
        query[i] = argv[i + 3];
    }

    // calculate the number of segment of current file
    numOfSegment = get_numOfSegment() + 2;
    // cout << "size of segment = " << numOfSegment << endl;

    // create a two-dimensional array for keeping occurrance information
    pOcc = new int*[numOfSegment];
    for (int i = 0; i < numOfSegment; i++) {
        pOcc[i] = new int[sizeOfIndex];
    }

    init();
    // cout << "inv_occ space = " << calculate_inverse_occ(64065, ' ') << endl; 
    backward_search(query);
}


/**
 * @brief: backward search in following steps:
 * 1. find the longest query by given an array of all query   
 * 2. find the first and last position in the BWT file if the query is matched
 * 3. find the prefix and suffix using backward_decode and forward_decode
 * 4. concat the sent without id and matching all query by going throught the array 
 * 5. print the sent with id if the sent is matched by all queries
 * @param: query, an array which contains all query strings
 */
void backward_search(string *query) {
    // find the longest query as first query
    string longestQuery = "";
    for (int i = 0; i < numOfQuery; i++) {
        if (query[i].length() > longestQuery.length())
            longestQuery = query[i];
    }

    int i = longestQuery.length() - 1;
    int c = (int)longestQuery[i];
    int first = get_count(c) + 1;       // get the first position of character in ordered BWT file
    int last = get_count(c + 1);        // get the last position of character in ordered BWT file

    while ((first <= last) && (i) >= 1) {
        c = (int)longestQuery[i - 1];   // get the next character in query to be matched
        i = i - 1;                      // move the point to next

        calculate_occ(first - 1);       // re-calculate the occurrance of all chars in prefix[1, first - 1]
        first = (get_count(c) + 1) + get_occ(c);    // get the new 'first'

        calculate_occ(last);            // re-calculate the occurrance of all chars in prefix[1, last]
        last  = get_count(c) + get_occ(c);          // get the new 'last'
    }

    if (last < first) {                 // didn't match if last < first
        return;
    } 
    // cout << "size of machaed query = " << last - first + 1 << endl;

    // find the whole sents by given a position in BWT file
    map<int, char> matchedQuery;
    int id;
    bool findFlag;
    string prefix, sent;

    for (int i = first; i < last + 1; i++) {
        prefix = backward_decode(i);    // get the prefix
        id = get_identity(prefix);

        // if the id has been matched, ignore the sentence
        if (matchedQuery.find(id) != matchedQuery.end()) {
            continue;
        } else {
            matchedQuery.insert(pair<int, char>(id, 'f'));
        }

        // if the prefix didn't contains ']', the 'pos' is located between '[' and ']' 
        if (prefix.find(']') == string::npos) {
            continue;
        } 
        sent = get_sentence(prefix);
        sent += forward_decode(i);      // get the full sent without identity ID

        findFlag = true;
        // go through whole query array 
        for (int j = 0; j < numOfQuery; j++) {
            if (sent.find(query[j]) == string::npos) {
                findFlag = false;
                break;
            }
        }
        if (findFlag) {
            cout << "[" << id << "]" << sent << endl;
        }
    }
    // cout << "cout size is " << matchedQuery.size() << endl;
}


/**
 * @brief: given a position, the suffix string from the current charcter to next '['
 * @param: pos, the position of character in BWT file
 */
string forward_decode(int pos) {
    string suffix = "";
    char c = get_character_from_orderedBWT(pos);    // get the character of position in BWT file
    int th = get_th_in_orderedBWT(pos, c);          // calculat the order number of current character
                                                    // over all this character in ordered BWT file
    int counter = 0;
    while (c != '[') {
        suffix += c;
        pos = calculate_inverse_occ(th, c);         // given 'th' and 'c', calculate the position of same character in BWT file
        c = get_character_from_orderedBWT(pos);     // get a character in orderedBWT fiel by given a position
        th = get_th_in_orderedBWT(pos, c);          // calcualt the order number
    }
    return suffix;
}


/**
 * @brief: given a position, the prefix string from the current charcter to previous '['
 * @param: pos, the position of character in BWT file
 */
string backward_decode(int pos) {
    string prefix = "";
    char c = get_character_from_BWT(pos);           // get the character of position in BWT file

    while ( c != '[') {
        prefix = c + prefix;
        calculate_occ(pos);                         // calculate the occurrence of all char in prefix(from 1 to pos) of BWT file
        pos = get_count((int) c) + get_occ((int) c);// calculate the new position
        c = get_character_from_BWT(pos);
    }
    return c + prefix;
}


/**
 * @brief: calculate the occurrence of all char in prefix(from 1 to pos) of BWT file
 * @param: endPosition, the end position in BWT file
 */
void calculate_occ(int endPosition) {
    int numOfSeg = (endPosition / sizeOfSegment);   // calculate the number of segment 
    int start = numOfSeg * sizeOfSegment;
    int totalSize = endPosition - start;            // the size we need to read char from BWT fiel from position of start

    if (numOfSeg > 0) {
        for (int i = 0; i < sizeOfIndex; i++) {
            segCounter[i] = pOcc[numOfSeg][i];
        }
    } else {
        for (int i = 0; i < sizeOfIndex; i++) {
            segCounter[i] = 0;
        }
    }

    clearerr(pFile);
    fseek(pFile, start, SEEK_SET);                  // seek the position of start
    fread(buffer, totalSize, 1, pFile);             // read to buffer

    for (int i = 0; i < totalSize; i++) {
        segCounter[(int)buffer[i] - offSet] += 1;   // accumulate all occurrance
    }
}


/**
 * @brief: calculate the accuracy position of character in BWT file by given 
 *         the order 'count' and char 'c'.
 * @param: count, the order number of current 'c' in all same characterin orderted BWT file
 * @param: c, the character
 */
int calculate_inverse_occ(int count, char c) {
    int numOfSeg = 0;
    int characterIndex = (int)c - offSet;
    int index = 0;

    // linear search
    // find the number of segment, which contian the 'c' we looking for
    // for (int i = 1; i < numOfSegment; i++) {
    //     if (pOcc[i][characterIndex] >= count) {
    //         numOfSeg = i - 1;
    //         // if (i > 1) {
    //         //     count -= pOcc[numOfSeg][characterIndex];
    //         // }
    //         index = numOfSeg * sizeOfSegment;
    //         break;
    //     }
    // }

    // binary search
    numOfSeg = binary_search(count, c);
    if (numOfSeg > 0) {
        count -= pOcc[numOfSeg][characterIndex];
    }
    index = numOfSeg * sizeOfSegment;

    int start = numOfSeg * sizeOfSegment;               // the star position of current segment
    int totalSize = sizeOfSegment;                      // read the whole segment into buffer

    clearerr(pFile);
    fseek(pFile, start, SEEK_SET);                      // move the file pointer to start position
    fread(buffer, totalSize, 1, pFile);                 // read the segment into buffer

    for (int i = 0; i < totalSize; i++) {
        index++;
        if (buffer[i] == c) {
            count--;
        }
        if (count <= 0) {
            break;
        }
    }
    return index;
}


/**
 * @brief: calculate the ocurance of all character in any (n * sizeOfSegment) position
 *         and keep that in array pOcc form 1 to numOfSegment
 *         calculate the all occurrance of all character before the current character 
 *         according to ASCII oder in orderedBWT file and keep that in pOcc[0]
 */
void init() {
    ifstream inFile;
    inFile.open(inFilePath);

    int counter[sizeOfIndex] = {0};
    int currentASCII;
    int count = 0;
    int line_count = 1;

    while(!inFile.eof()) {
        currentASCII = (int)inFile.get() - offSet;
        counter[currentASCII] += 1;
        count++;

        if ((count % sizeOfSegment) == 0) {     // keep the occurrance into pOcc in any (n * sizeOfSegment) position
            for (int i = 0; i < sizeOfIndex; i++) {
                pOcc[line_count][i] = counter[i];
            }
            line_count++;
        }
    }
    if ((count % sizeOfSegment) != 0) {         // keep the last occurrance into pOcc if there are remind count
        for (int i = 0; i < sizeOfIndex; i++) {
            pOcc[line_count][i] = counter[i];
        }
    }

    // the total number of all chars in ordered BWT file,
    // which are alphabetically smaller then c (including repetitions of chars)
    int sum = 0;
    for (int i = sizeOfIndex - 1; i > 0; i--) {
        counter[i] = 0;
        for (int j = 0; j < i; j++) {
            counter[i] += counter[j];
        }
    }
    // write the calculate result to pOcc[0]
    for (int i = 0; i < sizeOfIndex; i++) {
        pOcc[0][i] = counter[i];
    }
    inFile.close();
}


/**
 * @brief: calculat hte number of segment of input BWT file
 */
int get_numOfSegment() {
    clearerr(pFile);
    fseek(pFile, 0, SEEK_END);
    int size =(int)ftell(pFile);
    return (size /sizeOfSegment);
}


/**
 * @brief: get the character by given a position in BWT file
 */
char get_character_from_BWT(int pos) {
    clearerr(pFile);
    fseek(pFile, pos - 1, SEEK_SET);
    char c = fgetc(pFile);
    return c;
}


/**
 * @brief: get the character by given a position in ordered BWT file
 */
char get_character_from_orderedBWT(int pos) {
    for (int i = 0; i < sizeOfIndex; i++) {
        if (pOcc[0][i] >= pos) {
            return (char)(i - 1 + offSet);
        }
    }
    return (char)offSet;
}


/**
 * @brief: get the order number of char 'c' in postion 'pos'
 *         over all char 'c' in ordered BWT file
 *         label from 1
 */
int get_th_in_orderedBWT(int pos, char c) {
    return pos - pOcc[0][(int)c - offSet];
}


/**
 * @brief: get the total count of all charc in ordered BWT file, which are 
 *         alphabetically smaller then c (including repetitions of chars)
 */
int get_count(int c) {
    return pOcc[0][c - offSet];
}


/**
 * @brief: get the count in segCounter by given a char 'c', and
 *         this function should be running after calculate_occ,
 *         which has re-calculate the occurrance of all chars in prefix[1, pos]
 */
int get_occ(int c) {
    return segCounter[c - offSet];
}


/**
 * @brief: get identity ID of given a sent
 */
int get_identity(string match) {
    int end = match.find(']');
    string id = match.substr(1, end - 1);
    return atoi(id.c_str());
}


/**
 * @brief: get the sentence without ID
 */
string get_sentence(string match) {
    int end = match.find(']');
    return match.substr(end + 1);
}


/**
 * @brief: binary search, find the largest number among those numbers which
 *         are all smaller than the number of 'count'
 * @param: count: the occurrance of char 'c'
 *         c: the char 'c'
 */ 
int binary_search(int count, char c) {
    int characterIndex = (int)c - offSet;
    int mid, front = 1, back = numOfSegment - 1;
    while(front <= back) {
        if (back - front == 1) {
            if (pOcc[front][characterIndex] >= count) {
                return front - 1;
            } else {
                if (pOcc[back][characterIndex] >= count) {
                    return back - 1;
                }
            }
        }
        else if (front == back) {
            if (pOcc[front][characterIndex] >= count) {
                return front - 1;
            } else {
                return front;
            }
        }

        mid = (front + back) / 2;
        // case 1: if the number in 'mid' == count
        //         looking for the all numbers from front to mid
        if (pOcc[mid][characterIndex] == count) {
            if (pOcc[mid - 1][characterIndex] == count) {
                back = back - 2;
            }
            else if (pOcc[mid - 1][characterIndex] < count) {
                return mid - 1;
            }
        }
        // case 2: if the number in 'mid' > count
        //         looking for the all numbers from front to (mid - 1)
        else if (pOcc[mid][characterIndex] > count) {
            if (pOcc[mid - 1][characterIndex] >= count) {
                back = mid - 1;
            } else { // pOcc[mid + 1][characterIndex] < count
                return mid - 1;
            }
        }
        // case 3: if the number in 'mid' < count
        //         looking for the all numbers from mid to back
        else if (pOcc[mid][characterIndex] < count) {
            if (pOcc[mid + 1][characterIndex] >= count) {
                return mid;
            } else { // pOcc[mid + 1][characterIndex] < count
                front = mid + 1;
            }
        }
    }
    return 0;
}
