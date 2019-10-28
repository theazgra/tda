#include "dis/SgmlFile.h"

int main(int argc, char **argv)
{
    char *inputFile = const_cast<char*>("/mnt/d/codes/git/tda/data/txtdata/reut2-021.sgm");
    char *outputFile = const_cast<char*>("/mnt/d/codes/git/tda/data/txtdata/reut2-021.txt");
    char *stopwordsFile = const_cast<char*>("/mnt/d/codes/git/tda/data/txtdata/stopwords.txt");
    if (argc == 4)
    {

        inputFile = argv[1];
        outputFile = argv[2];
        stopwordsFile = argv[3];
    }
    dis::SgmlFile sgmlFile = dis::SgmlFile::load(inputFile);

    sgmlFile.save_preprocessed_text(outputFile, stopwordsFile);
    azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Green, "Save preprocessed text\n");

    return 0;
}