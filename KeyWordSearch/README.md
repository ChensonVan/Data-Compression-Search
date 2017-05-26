comp9319 - Assignment3 Description.

The program do search query terms following these steps:

1. For the first time running the code wiht a new input files, the program will create an index folder by given name to save files, includes fileNameMap file and tokanInvertedList file.

    step 1: get all file names in the input folder and assign a docID for that.
    step 2: for each files in the input folder, calculating each word frequency in this file.
    step 3: whenever counting word frequency in a file is finished, update the local word count to global map following the format: token <<doc1, freq1>, <doc2, freq2>, ...>. Before the word pushed to the global map, the program will check the word is stopWords or not and also do the stem on the word. These two steps will save some space on inverted-list files.
    step 4: whenever the size of global map great than the threshold we specified, all words with the doc-freq pairs will save to disk, and the program re-calculate that again untill alll files was processed.
    step 5: finally, when all tmp index files will be merged into one. 

2. After the first time runing the code, the index folder has existed. So the code will read inverted lists of all query terms from tokenInvertedList file. Then the program do intersection operation between these inverted lists and calculate a common inverted list in which all of these query terms occureed. Finally the program ranks the common inverted list and prints ecah record according to the average occuring time.


