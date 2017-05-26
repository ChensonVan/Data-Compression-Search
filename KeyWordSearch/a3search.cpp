#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <chrono>
#include "porter2_stemmer.h"

using namespace std;

#define K 1800000           // define threshold to save current df-map to disk

ifstream inFile;
ofstream outFile;

string inputFolder;         // input files folder
string indexFolder;         // index fiels folder
string currentFile;         // current fiel name
string *queryTerms;         // the point of all query terms
int totalWordCount;

unsigned int currentWordID; // current word ID
unsigned short int currentFileID;   // current file ID
unsigned short int numOfQuery = 1;  // num of qurty term

typedef pair<unsigned short int, unsigned short int> dfPairs;
unordered_map<string, vector<dfPairs> > docFreqPairs;       // key is a stemed word, value is the vactor comtaining a list of <docID, freq>
unordered_map<string, unsigned short int> localWordCount;   // key is a word,  value is local count in a file
map<unsigned short int, string> allFileMaps;                // the map of docID to doc name
map<unsigned short int, unsigned short int> commonInvList;  // save the common occuring list of all query terms

// stop words
const set<string> stopWords = {"able", "about", "above", "according", "accordingly", "across", "actually", "after", "afterwards", "again", "against", "all", "allow", "allows", "almost", "alone", "along", "already", "also", "although", "always", "among", "amongst", "and", "another", "any", "anybody", "anyhow", "anyone", "anything", "anyway", "anyways", "anywhere", "apart", "appear", "appreciate", "appropriate", "are", "around", "aside", "ask", "asking", "associated", "available", "away", "awfully", "became", "because", "become", "becomes", "becoming", "been", "before", "beforehand", "behind", "being", "believe", "below", "beside", "besides", "best", "better", "between", "beyond", "both", "brief", "but", "came", "can", "cannot", "cant", "cause", "causes", "certain", "certainly", "changes", "clearly", "com", "come", "comes", "concerning", "consequently", "consider", "considering", "contain", "containing", "contains", "corresponding", "could", "course", "currently", "definitely", "described", "despite", "did", "different", "does", "doing", "done", "down", "downwards", "during", "each", "edu", "eight", "either", "else", "elsewhere", "enough", "entirely", "especially", "etc", "even", "ever", "every", "everybody", "everyone", "everything", "everywhere", "exactly", "example", "except", "far", "few", "fifth", "first", "five", "followed", "following", "follows", "for", "former", "formerly", "forth", "four", "from", "further", "furthermore", "get", "gets", "getting", "given", "gives", "goes", "going", "gone", "got", "gotten", "greetings", "had", "happens", "hardly", "has", "have", "having", "hello", "help", "hence", "her", "here", "hereafter", "hereby", "herein", "hereupon", "hers", "herself", "him", "himself", "his", "hither", "hopefully", "how", "howbeit", "however", "ignored", "immediate", "inasmuch", "inc", "indeed", "indicate", "indicated", "indicates", "inner", "insofar", "instead", "into", "inward", "its", "itself", "just", "keep", "keeps", "kept", "know", "knows", "known", "last", "lately", "later", "latter", "latterly", "least", "less", "lest", "let", "like", "liked", "likely", "little", "look", "looking", "looks", "ltd", "mainly", "many", "may", "maybe", "mean", "meanwhile", "merely", "might", "more", "moreover", "most", "mostly", "much", "must", "myself", "name", "namely", "near", "nearly", "necessary", "need", "needs", "neither", "never", "nevertheless", "new", "next", "nine", "nobody", "non", "none", "noone", "nor", "normally", "not", "nothing", "novel", "now", "nowhere", "obviously", "off", "often", "okay", "old", "once", "one", "ones", "only", "onto", "other", "others", "otherwise", "ought", "our", "ours", "ourselves", "out", "outside", "over", "overall", "own", "particular", "particularly", "per", "perhaps", "placed", "please", "plus", "possible", "presumably", "probably", "provides", "que", "quite", "rather", "really", "reasonably", "regarding", "regardless", "regards", "relatively", "respectively", "right", "said", "same", "saw", "say", "saying", "says", "second", "secondly", "see", "seeing", "seem", "seemed", "seeming", "seems", "seen", "self", "selves", "sensible", "sent", "serious", "seriously", "seven", "several", "shall", "she", "should", "since", "six", "some", "somebody", "somehow", "someone", "something", "sometime", "sometimes", "somewhat", "somewhere", "soon", "sorry", "specified", "specify", "specifying", "still", "sub", "such", "sup", "sure", "take", "taken", "tell", "tends", "than", "thank", "thanks", "thanx", "that", "thats", "the", "their", "theirs", "them", "themselves", "then", "thence", "there", "thereafter", "thereby", "therefore", "therein", "theres", "thereupon", "these", "they", "think", "third", "this", "thorough", "thoroughly", "those", "though", "three", "through", "throughout", "thru", "thus", "together", "too", "took", "toward", "towards", "tried", "tries", "truly", "try", "trying", "twice", "two", "under", "unfortunately", "unless", "unlikely", "until", "unto", "upon", "use", "used", "useful", "uses", "using", "usually", "uucp", "value", "various", "very", "via", "viz", "want", "wants", "was", "way", "welcome", "well", "went", "were", "what", "whatever", "when", "whence", "whenever", "where", "whereafter", "whereas", "whereby", "wherein", "whereupon", "wherever", "whether", "which", "while", "whither", "who", "whoever", "whole", "whom", "whose", "why", "will", "willing", "wish", "with", "within", "without", "wonder", "would", "would", "yes", "yet", "you", "your", "yours", "yourself", "yourselves", "zero"};

void getAllFiles();
void getCommonInvList();
void generateInvertedList();
void updateGlobalMaps();
void saveAllTokens();
void saveAllFileNames();
void readAllFileNames();
void rankInvList();
void checkInfo();
void checkExistOfIndexFolder();
bool checkStopWord(string &element);
void mergeInvertedList();
map<unsigned short int, unsigned short int> readInvertedList(string &term);
map<unsigned short int, unsigned short int> splitString(const string& str, const string& delimiter);




template<typename T1, typename T2>
/**
 * @brief: overwite add structure of two maps
 * 1. return the commmon element of two maps
 */
struct add {
    // it will be better to use const references, but then you'll not be able
    // to use operator[], and should be use `find` instead 
    add( std::map<T1, T2>& m1, std::map<T1, T2>& m2 ) : m1(m1), m2(m2) {}

    std::pair<T1,T2> operator()( const T1& index )
    {
        return make_pair(index, m1[index] + m2[index]); 
    }
    private:
        std::map<T1, T2>& m1, & m2;
};

/**
 * @brief: overwite cmp function of two vector
 */
bool cmp(const pair<string, double> &p1, const pair<string, double> &p2) {
    if(p1.second != p2.second)
        return p1.second > p2.second;
    return p1.first < p2.first;
}




int main(int argc, char* argv[]) {
    inputFolder = argv[1];
    indexFolder  = argv[2];
    numOfQuery = argc - 3;

    // get all query terms
    queryTerms = new string[numOfQuery];
    int offSet;
    if(strcmp(argv[3], "-c") == 0) {
        offSet = 5;
        numOfQuery -= 2;
    } else {
        offSet = 3;
    }
    for (int i = 0; i < numOfQuery; ++i) {
        queryTerms[i] = argv[i + offSet];
    }

    // check the exist of index foler
    // if not, create that and generate inverted-list of all input fles
    checkExistOfIndexFolder();

    // read the map files of docID-docName
    readAllFileNames();
    // get the commond inverted list of all query terms
    getCommonInvList();
    // rank the inverted list according the average occurance and file name
    rankInvList();
    return 0;
}


/**
 * @brief: generate an inverted lists step:
 * 1. split a document by non-alphabetic symbol
 * 2. conver the token to lower case
 * 3. check if the token is stopword
 * 4. count the frequence of all words in a document
 * 5. update the global map when a document finishment
 * 6. clean local word count map
 * @param: query, an array which contains all query terms
 */
void generateInvertedList() {
    int swc = 0;
    int twc = 0;
    int nwc = 0;
    for (const auto& f : allFileMaps)
    {
        currentFileID = f.first;
        currentFile = f.second;
        // open the input files
        inFile.open((inputFolder + "/" + currentFile).c_str());

        // read the file character by character
        string word = "";
        char c;
        while(inFile.peek() != EOF) {
            c = tolower(inFile.get());
            if (isalpha(c)) {
                word.push_back(c);
            } else {
                // check teh word size, and ignore those words which length is 2
                if (word.size() > 2) {
                    ++twc;
                    transform(word.begin(), word.end(), word.begin(), ::tolower);
                    // Porter2Stemmer::stem(word);
                    // ignore stop word
                    if (!checkStopWord(word)) {
                        // count the local word frequence
                        ++localWordCount[word];
                        ++nwc;
                    } else {
                        ++swc;
                    }
                }
                word.clear();
            }
        }
        // updateglobal map information when a document is finished
        updateGlobalMaps();
        localWordCount.clear();
        inFile.close();

        // if totalWordCount > memory threshold
        if (totalWordCount > K) {
            // cout << "SAVE ALL TOKENS" << endl;
            // checkInfo();
            // save the current map information to disk
            saveAllTokens();
            totalWordCount = 0;
        }
    }
    // cout << endl;
    // checkInfo();
    saveAllTokens();
    // cout << "total  word count = " << twc << endl;
    // cout << "stop   word count = " << swc << endl;
    // cout << "normal word count = " << nwc << endl << endl;
    // cout << "TOTAL  word count = " << totalWordCount << endl;
}


/**
 * @brief: update global map information when a document is finished
 * 1. get a token from local word count map
 * 2. stem the token
 * 3. check if the token is new or not
 * 4. if the token is new, then create a new vector<docID-freq-pairs> in global map
 * 5. if the token has occured before, push the <docID, freq> to the back of globalMap[token]
 * 6. clean local word count map
 * @param: local word count map
 */
void updateGlobalMaps() {
    string token;
    unsigned short int freq;
    for (const auto& wc : localWordCount) {
        token = wc.first;
        freq = wc.second;
        ++totalWordCount;
        // stem the word
        Porter2Stemmer::stem(token);
        if (checkStopWord(token)) {
            // cout << "after == " << token << endl;
            // cout << "before == " << wc.first << endl << endl;
            continue;
        }

        // check the token is new or not
        auto it = docFreqPairs.find(token);
        if (it != docFreqPairs.end()) {
            // create a new vector for this new token
            docFreqPairs[token].push_back(make_pair(currentFileID, freq));
        } else {
            // push the <docID, freq> to the back of the vector of the token
            vector<dfPairs> df = {{currentFileID, freq}};
            docFreqPairs[token] = df;
        }
    }
    localWordCount.clear();
} 


/**
 * @brief: get the common inverted-list of all query terms
 * 1. get the inv-list of the first term
 * 2. get the inv-list of the second term, and do intersection of these two lists to a new
 * 3. get the inv-list of the third term and do intersection of thesw to lists
 * 4. repeate previous steps until all terms has visited
 * @param: all query terms
 * @return: common inverted-list
 */
void getCommonInvList() {
    string term = queryTerms[0];
    commonInvList = readInvertedList(term);
    if(numOfQuery == 1) {
        return;
    }

    for (int i = 1; i < numOfQuery; ++i) {
        term = queryTerms[i];
        // find the common elements between commonInvList and current term inv-list
        map<unsigned short int, unsigned short int> invList1 = readInvertedList(term);
        map<unsigned short int, unsigned short int>::const_iterator begin1 = invList1.begin();
        map<unsigned short int, unsigned short int>::const_iterator begin2 = commonInvList.begin();
        vector<unsigned short int> v;
        for (; begin1 != invList1.end() && begin2 != commonInvList.end(); ) {
            if ( begin1->first < begin2->first ) 
                ++begin1;
            else if (begin2->first < begin1->first) 
                ++begin2;
            else v.push_back( begin1->first ), ++begin1, ++begin2;
        }
        // for those common element, add the value with the same key
        map<unsigned short int, unsigned short int> invList2;
        transform( v.begin(), v.end(), std::inserter(invList2, invList2.begin()), add<unsigned short int, unsigned short int>(invList1, commonInvList));
        commonInvList = invList2;
    }
}


/**
 * @brief: get the inverted list of token from idx file
 * 1. stem the token
 * 2. read the index fiel line by line
 * 3. if the line start with the token, then get the current inverted list
 * 4. combine the total inverted list with current tmp list
 * @param: a token
 * @return: the inverted list of the term
 */
map<unsigned short int, unsigned short int> readInvertedList(string &token) {
    map<unsigned short int, unsigned short int>termInvList;
    map<unsigned short int, unsigned short int>tmp;
    transform(token.begin(), token.end(), token.begin(), ::tolower);
    Porter2Stemmer::stem(token);

    string line;
    inFile.open(indexFolder + "/" + "a3SearchInvertedLists.idx", ios::in);
    // inFile.open(indexFolder + "/" + "mergeMap", ios::in);
    while(getline(inFile, line)) {
        if (line.substr(0, (token + ":").size()) == (token + ":")) {
            tmp = splitString(line.substr((token + ":").size()), ";");
            // combien two inverted list
            for (const auto& m : tmp) {
                termInvList[m.first] += m.second;
            }
        }
    }
    inFile.close();
    return termInvList;
}


/**
 * @brief: get all files in input folder and save that
 * 1. go through the input folder
 * 2. generated a ID for all fiels one by one
 * 3. save the files according the ID from 0
 * @param: input folder
 * @return: the inverted list of the term
 */
void getAllFiles(){
    DIR *pDIR;
    struct dirent *entry;
    pDIR = opendir(inputFolder.c_str());
    if(pDIR != NULL ){
        currentFileID = 1;
        while ((entry = readdir(pDIR)) != NULL) {
            // ignore the three system files
            if( strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".DS_Store") != 0 ){
                allFileMaps[currentFileID] = entry->d_name;
                ++currentFileID;
            }
        }
        closedir(pDIR);
    }
    // created the index folder
    system(("mkdir -p " + indexFolder).c_str());
    // save the map of docName with docID to disk
    saveAllFileNames();
}


/**
 * @brief: rank the final common inv list
 * 1. put all common docID with avg-frequency to vector
 * 2. sort the vector
 * 3. print the final result
 * @param: common inverted list
 */
void rankInvList() {
    vector< pair< string, double> > v;
    for(auto const& value: commonInvList) {
        v.push_back(make_pair(allFileMaps[value.first], (double)value.second / (double)numOfQuery));
    }
    sort(v.begin(), v.end(), cmp);
    // print the final result
    for (const auto& f : v) {
        cout << f.first << endl;
    }
}


/**
 * @brief: split the pair of docID and frequency from given a string
 * 1. format: token:docID, freq; docID, freq; ...
 * 1. split the string by delimiter ";"   
 * 2. split each pair by delimiter ","
 * 3. push the docID and freq to inverted lsit
 * @param: delimiter and the line of docID-freq
 */
map<unsigned short int, unsigned short int> splitString(const string& str, const string& delimiter) {
    map<unsigned short int, unsigned short int>termInvList;
    string pairs;
    string::size_type pos1, pos2;
    pos2 = str.find(delimiter);
    pos1 = 0;
    while(string::npos != pos2)
    {
        pairs = str.substr(pos1, pos2-pos1);
        unsigned short int docID = (unsigned short)atoi(pairs.substr(0, pairs.find(",")).c_str());
        unsigned short int freq = (unsigned short)atoi(pairs.substr(pairs.find(",") + 1).c_str());
        termInvList[docID] += freq;
 
        pos1 = pos2 + delimiter.size();
        pos2 = str.find(delimiter, pos1);
    }
    if(pos1 != str.length()) {
        pairs = str.substr(pos1, pos2-pos1);
        unsigned short int docID = (unsigned short)atoi(pairs.substr(0, pairs.find(",")).c_str());
        unsigned short int freq = (unsigned short)atoi(pairs.substr(pairs.find(",") + 1).c_str());
        termInvList[docID] += freq;
    }
    return termInvList;
}


/**
 * @brief: check the element is stopWords or not
 */
bool checkStopWord(string &element) {
    // return false;
    return stopWords.find(element) != stopWords.end();
}


/**
 * @brief: check the exist of index folder
 * 1. if the folder has existed, close that 
 * 2. if not, create a new and generate the inverted-list
 */
void checkExistOfIndexFolder() {
    DIR *dir = opendir(indexFolder.c_str());
    if (dir)
    {
        closedir(dir);
    }
    else if (ENOENT == errno) {
        closedir(dir);
        getAllFiles();
        generateInvertedList();
    }
}


/**
 * @brief: save all token with the vactor of all docID-docFreq to disk
 * 1. format: token:docID,freq;docID,freq;....
 * 2. clear the docFreqPairs map
 */
void saveAllTokens() {
    outFile.open(indexFolder + "/" + "a3SearchInvertedLists.idx", ios_base::app | ios::out);
    for (const auto& w : docFreqPairs) {
        outFile << w.first << ":";
        for(const auto& v : w.second) {
            outFile << v.first << "," << v.second << ";";
        }
        outFile << endl;
    }
    docFreqPairs.clear();
    outFile.close();
}

/**
 * @brief: save all filename to disk according to the order of docID
 * 1. format: token:docID,freq;docID,freq;....
 */
void saveAllFileNames() {
    outFile.open(indexFolder + "/" + "a3SearchFileNameMaps.txt", ios::out);
    for (const auto& f : allFileMaps) {
        outFile << f.second << endl;
    }
    outFile.close();
}


/**
 * @brief: read all filename with docID from fiel
 * 1. the docID is the numer of the file occuring the file
 */
void readAllFileNames() {
    allFileMaps.clear();
    inFile.open(indexFolder + "/" + "a3SearchFileNameMaps.txt", ios::in);
    string line; 
    unsigned short int c = 1;
    while(getline(inFile, line)){
        allFileMaps[c] = line;
        ++c;
    }
    inFile.close();
}


void checkInfo() {
    cout << "docFreqPairs.size = " << docFreqPairs.size() << endl;
    cout << "localWordCount.size = " << localWordCount.size() << endl;
    cout << "allFileMaps.size = " << allFileMaps.size() << endl << endl;
}

/**
 * @brief: merge all tmp inverted-lists into one
 * 1. get all inverted-lists for all word and put that in map
 * 2. save the map to disk
 */
void mergeInvertedList() {
    unordered_map<string, string> mergeMap;
    string line;
    inFile.open(indexFolder + "/" + "a3SearchInvertedLists.idx", ios::in);
    while(getline(inFile, line)) {
        mergeMap[line.substr(0, line.find(":"))] += line.substr(line.find(":") + 1);
    }     
    inFile.close();
    outFile.open(indexFolder + "/" + "mergeMap.txt", ios::out);
    for(const auto& r : mergeMap){
        outFile << r.first << ":" << r.second << endl;
    }
    outFile.close();
    mergeMap.clear();
}

