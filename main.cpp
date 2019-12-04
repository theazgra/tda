#include "dis/SgmlFile.h"
#include "dis/porter_stemmer.h"
#include "dis/sgml_collection.h"

#define ReutersFiles { "/mnt/d/codes/git/tda/data/txtdata/reut2-000.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-001.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-002.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-003.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-004.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-005.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-006.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-007.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-008.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-009.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-010.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-011.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-012.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-013.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-014.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-015.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-016.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-017.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-018.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-019.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-020.sgm", \
                        "/mnt/d/codes/git/tda/data/txtdata/reut2-021.sgm" }

#define ReutersFilesSmall { "/mnt/d/codes/git/tda/data/txtdata/reut2-000.sgm", \
                            "/mnt/d/codes/git/tda/data/txtdata/reut2-002.sgm", \
                            "/mnt/d/codes/git/tda/data/txtdata/reut2-020.sgm", \
                            "/mnt/d/codes/git/tda/data/txtdata/reut2-021.sgm" }

int main(int argc, char **argv)
{
    dis::SgmlFileCollection collection(ReutersFilesSmall);
    //dis::SgmlFileCollection collection({"/mnt/d/codes/git/tda/data/txtdata/reut2-021.sgm"});
    collection.load_and_preprocess_sgml_files("/mnt/d/codes/git/tda/data/txtdata/stopwords.txt");
    collection.create_term_index_with_vector_model();
//    collection.get_vector_model().save("vector_model.data");
    //collection.get_vector_model().load("/mnt/d/codes/git/tda/cmake-build-release/vector_model.data");
    //collection.dump_index("small_index.data");
    //collection.load_index("index.data");
//    collection.dump_term_frequency_matrix("term_frequency.matrix");
//    collection.load_term_frequency_matrix("term_frequency.matrix");

//    //collection.save_preprocessed_documents("processedDocuments.txt");
//    collection.create_term_index();
//    collection.dump_index("index.data");

//    collection.dump_compressed_index("index.fbenc");
    //collection.load_compressed_index("index.fbenc");
    //collection.dump_index("decompressed_index.data");

    azgra::BasicStringView<char> queryText(argv[1]);

//    azgra::string::SmartStringView<char> ssw(queryText);
//    auto queryResult = collection.query(ssw, true);

    auto queryResult = collection.get_vector_model().query_documents(queryText);

//    collection.dump_index("index2.data");
    //fprintf(stdout, "Query: '%s' terms found in %lu documents.\n", qt.data(), queryResult.documents.size());
    return 0;
#if 0
    if (argc == 2)
    {
        fprintf(stdout, "%s --> %s\n", argv[1], stem_word(argv[1]).get_c_string());
        return 0;
    }
    test_porter_stemmer();

    char *inputFile = const_cast<char *>("/mnt/d/codes/git/tda/data/txtdata/reut2-021.sgm");
    char *outputFile = const_cast<char *>("/mnt/d/codes/git/tda/data/txtdata/reut2-021.txt");
    char *stopwordsFile = const_cast<char *>("/mnt/d/codes/git/tda/data/txtdata/stopwords.txt");
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
#endif
}
