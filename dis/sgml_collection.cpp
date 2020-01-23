#include <azgra/collection/enumerable.h>
#include "sgml_collection.h"

namespace dis
{

    static std::vector<DocId> create_delta_vector(const std::vector<DocId> &documentSet)
    {
        std::vector<DocId> result(documentSet.size());

        for (size_t i = 0; i < documentSet.size(); ++i)
        {
            if (i == 0)
            {
                result[i] = documentSet[i];
            }
            else
            {
                result[i] = documentSet[i] - documentSet[i - 1];
            }
        }
        return result;
    }

    static std::vector<DocId> reconstruct_from_delta(const std::vector<DocId> &delta)
    {
        std::vector<DocId> result(delta.size());

        for (size_t i = 0; i < delta.size(); ++i)
        {
            if (i == 0)
            {
                result[i] = delta[i];
            }
            else
            {
                result[i] = delta[i] + result[i - 1];
            }
        }
        return result;
    }

    static std::pair<size_t, size_t> largest_lte_fib_num_index(const std::vector<size_t> &fibSeq,
                                                               const size_t target,
                                                               const size_t maxExclusiveIndex)
    {
        always_assert(maxExclusiveIndex > 1);
        for (auto i = static_cast<size_t>(maxExclusiveIndex - 1); i >= 0; --i)
        {
            if (fibSeq[i] <= target)
            {
                //return <value,index>
                return std::make_pair(fibSeq[i], i);// (maxExclusiveIndex - 1 - i));
            }
        }
        always_assert(false && "Didn't find fibonacci number!");
        return std::make_pair(-1, -1);
    }

    static void encode_delta_with_fibonacci_sequence(azgra::io::stream::OutMemoryBitStream &bitStream,
                                                     const std::string &term,
                                                     const std::vector<DocId> &delta,
                                                     const std::vector<size_t> &fibSeq)
    {
        // Write term
        const auto termLen = static_cast<azgra::u32>(term.length());
        bitStream.write_value(termLen);
        for (const char &c : term)
        {
            bitStream.write_value<azgra::byte>(c);
        }
        always_assert(delta.size() <= std::numeric_limits<azgra::u32>::max());
        auto deltaSize = static_cast<azgra::u32> (delta.size());
        bitStream.write_value(deltaSize);

        for (const DocId &value : delta)
        {
            long remaining = static_cast<long> (value);
            std::vector<size_t> fibIndicis;
            size_t maxIndex = fibSeq.size();
            while (remaining > 0)
            {
                auto[value, index] = largest_lte_fib_num_index(fibSeq, remaining, maxIndex);
                fibIndicis.push_back(index);
                remaining -= value;
            }
            const size_t maxFibIndex = azgra::collection::max(fibIndicis.begin(), fibIndicis.end());
            for (size_t i = 0; i <= maxFibIndex; ++i)
            {
                // Index is set write 1
                const bool bit = std::find(fibIndicis.begin(), fibIndicis.end(), i) != fibIndicis.end();
                bitStream << bit;
            }
            // Write terminating 1.
            bitStream << true;
        }
    }

    static std::vector<std::pair<std::string, std::vector<DocId>>>
    decode_deltas_from_fibonacci_sequence(azgra::io::stream::InMemoryBitStream &bitStream, std::vector<size_t> &fibSeq)
    {
        std::vector<std::pair<std::string, std::vector<DocId>>> deltas;
        auto termLen = bitStream.read_value<azgra::u32>();
        std::vector<char> termData(termLen);
        for (size_t i = 0; i < termLen; ++i)
        {
            termData[i] = bitStream.read_value<azgra::byte>();
        }
        std::string term = std::string(termData.data(), termLen);

        auto deltaSize = bitStream.read_value<azgra::u32>();
        do
        {

            int deltaValueRemaining = deltaSize;
            std::vector<DocId> delta;
            bool prevWasOne = false;
            std::vector<size_t> fibIndices;
            long index = -1;

            while (deltaValueRemaining)
            {
                ++index;
                if (bitStream.read_bit())   // 1
                {
                    if (prevWasOne)
                    {
                        // If previous was one reset here and decode from indices.
                        size_t result = 0;
                        for (const size_t &fibIndex : fibIndices)
                        {
                            result += fibSeq[fibIndex];
                        }
                        delta.push_back(result);

                        --deltaValueRemaining;
                        index = -1;
                        fibIndices.clear();
                        prevWasOne = false;
                    }
                    else
                    {
                        prevWasOne = true;
                        fibIndices.push_back(index);
                    }
                }
                else                        // 0
                {
                    prevWasOne = false;
                }
            }
            always_assert (delta.size() == static_cast<size_t>(deltaSize));
            deltas.emplace_back(term, delta);

            termLen = bitStream.read_value<azgra::u32>();
            if (termLen == 0)
            {
                break;
            }
            termData.resize(termLen);
            for (size_t i = 0; i < termLen; ++i)
            {
                termData[i] = bitStream.read_value<azgra::byte>();
            }
            term = std::string(termData.data(), termLen);
            deltaSize = bitStream.read_value<azgra::u32>();
        } while (deltaSize);
        return deltas;
    }

//
//    void test()
//    {
//        const auto delta = create_delta_vector({1, 20, 50, 100, 101, 200, 450, 813, 1568});
//        const auto delta2 = create_delta_vector({2000, 2500, 3221});
//        auto fibSeq = generate_fibonacci_sequence(40);
//        azgra::OutMemoryBitStream bitStream;
//        encode_delta_with_fibonacci_sequence(bitStream, "ahoj", delta, fibSeq);
//        encode_delta_with_fibonacci_sequence(bitStream, "svete", delta2, fibSeq);
//        bitStream.write_value<azgra::u32>(0);
//
//        auto buffer = bitStream.get_flushed_buffer();
//        azgra::InMemoryBitStream bitDecoderStream(&buffer);
//        auto decodedDeltas = decode_deltas_from_fibonacci_sequence(bitDecoderStream, fibSeq);
//        //auto reconstructedOriginalValues = reconstruct_from_delta(decodedDeltas[0]);
//        const auto size = buffer.size();
//    }


    SgmlFileCollection::SgmlFileCollection(std::vector<const char *> sgmlFilePaths)
    {
        m_inputFilePaths = std::move(sgmlFilePaths);
    }

    void SgmlFileCollection::load_and_preprocess_sgml_files(const char *stopwordFile)
    {
        const auto stopwords = strings_to_views(azgra::io::read_lines(stopwordFile));

        DocId docId = 1;
        m_sgmlFiles.resize(m_inputFilePaths.size());
        for (size_t fileIndex = 0; fileIndex < m_inputFilePaths.size(); ++fileIndex)
        {
            m_sgmlFiles[fileIndex] = SgmlFile::load(m_inputFilePaths[fileIndex], docId);
            m_sgmlFiles[fileIndex].preprocess_article_text(stopwords);
            m_sgmlFiles[fileIndex].destroy_original_text();
        }
        documentCount = docId-1;
        fprintf(stdout, "Document count: %lu\n", documentCount);
    }

    void SgmlFileCollection::create_term_index_with_vector_model()
    {
        m_index.clear();

        for (const auto &sgmlFile : m_sgmlFiles)
        {
            sgmlFile.index_atricles(m_index);
        }
        fprintf(stdout, "Created index with %lu terms\n", m_index.size());
        m_vectorModel = VectorModel(m_index, documentCount);
    }

    void SgmlFileCollection::dump_index(const char *path)
    {
        std::ofstream dump(path, std::ios::out);
        for (const auto &term : m_index)
        {
            dump << term.first << ':';
            for (const DocumentOccurence &docOcc : term.second)
            {
                dump << docOcc.docId << ",";
            }
            dump << "\n";
        }
    }

    void SgmlFileCollection::load_index(const char *path)
    {
        m_index.clear();
        std::vector<std::pair<std::string, std::set<DocumentOccurence>>> mapPairs;
        std::function<std::pair<std::string, std::set<DocumentOccurence>>(const azgra::string::SmartStringView<char> &)> fn =
                [](const azgra::string::SmartStringView<char> &line)
                {
                    auto index = line.index_of(":");
                    std::string term = std::string(line.substring(0, index).string_view());
                    auto docIds = line.substring(index + 1).split(',');
                    std::set<DocumentOccurence> ids;
                    for (const auto dIdStr : docIds)
                    {
                        if (!dIdStr.is_empty())
                        {
                            ids.insert(DocumentOccurence(atol(dIdStr.data()), 0));
                        }
                    }
                    return std::make_pair(term, ids);
                };

        mapPairs = azgra::io::parse_by_lines<std::pair<std::string, std::set<DocumentOccurence>>>(path, fn);
        m_index = TermIndex(mapPairs.begin(), mapPairs.end());
        fprintf(stdout, "%lu\n", mapPairs.size());
    }

    void SgmlFileCollection::save_preprocessed_documents(const char *path)
    {
        std::ofstream fStream(path, std::ios::out);
        always_assert(fStream.is_open());

        for (auto &sgmlFile : m_sgmlFiles)
        {
            for (auto &article : sgmlFile.get_articles())
            {
                fStream << "DocId:" << article.get_docId() << '\n';
                fStream << article.get_processed_string() << '\n';
            }
        }
    }

    QueryResult SgmlFileCollection::query(azgra::string::SmartStringView<char> &queryText, const bool verbose) const
    {
        QueryResult result = {};
        if (queryText.is_empty())
        {
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "Query string is empty.\n");
            return result;
        }
        if (m_index.empty())
        {
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "Index wasn't created nor loaded.\n");
            return result;
        }

        auto keywords = queryText.split(" ");
        std::vector<SizedIndexEntry> indexEntries;
        for (const auto &keyword : keywords)
        {
            AsciiString str = stem_word(keyword.data(), keyword.length());
            const std::string key = std::string(str.get_c_string());
            if (keyword.is_empty() || (m_index.find(key) == m_index.end()))
            {
                azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "Term %s is not found in any documents.\n",
                                       key.c_str());
                return result;
            }
            else
            {
                azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Cyan, "Term %s is found in %lu documents.\n",
                                       key.c_str(), m_index.at(key).size());

            }


            indexEntries.push_back(SizedIndexEntry(m_index.at(key))); // NOLINT(hicpp-use-emplace,
            // modernize-use-emplace)
        }

        if (indexEntries.empty())
        {
            azgra::print_if(verbose, "Query returned no results.\n");
            return result;
        }

        std::sort(indexEntries.begin(), indexEntries.end());

        //result.documents = indexEntries[0].documents;
        std::vector<DocId> unionVector = std::vector<DocId>(indexEntries[0].documents.begin(),
                                                            indexEntries[0].documents.end());
        if (indexEntries.size() > 1)
        {
            for (size_t i = 1; i < indexEntries.size(); ++i)
            {
                std::vector<DocId> unionResult;
                unionResult.clear();
                std::set_intersection(unionVector.begin(), unionVector.end(),
                                      indexEntries[i].documents.begin(),
                                      indexEntries[i].documents.end(),
                                      std::back_inserter(unionResult));
                unionVector = unionResult;
            }
        }
        result.documents = std::set<DocId>(unionVector.begin(), unionVector.end());

        if (verbose)
        {
            fprintf(stdout, "Query `%s` returned %lu documents\n", queryText.data(), result.documents.size());
            std::stringstream docStream;

            for (const auto &docId:result.documents)
            {

                docStream << docId << ',';
            }
            fprintf(stdout, "Documents:\n%s\n", docStream.str().c_str());
        }

        return result;
    }

    void SgmlFileCollection::dump_compressed_index(const char *filePath) const
    {
        auto fibSeq = generate_fibonacci_sequence(50);
        azgra::io::stream::OutMemoryBitStream bitStream;
        for (const auto&[term, documentSet] : m_index)
        {
            auto documentVector = azgra::collection::select(documentSet.begin(),
                                                            documentSet.end(),
                                                            [](const DocumentOccurence &occurence)
                                                            { return occurence.docId; });
            std::sort(documentVector.begin(), documentVector.end());
            auto deltaVector = create_delta_vector(documentVector);
            encode_delta_with_fibonacci_sequence(bitStream, term, deltaVector, fibSeq);
        }
        bitStream.write_value<azgra::u32>(0);
        const auto buffer = bitStream.get_flushed_buffer();
        azgra::io::dump_bytes(buffer, filePath);
    }

    void SgmlFileCollection::load_compressed_index(const char *filePath)
    {
        fprintf(stdout, "Loading compressed index...\n");
        auto fibSeq = generate_fibonacci_sequence(50);
        azgra::io::stream::InBinaryFileStream compressedIndexBinaryStream(filePath);
        const auto buffer = compressedIndexBinaryStream.consume_whole_file();
        azgra::io::stream::InMemoryBitStream bitStream(&buffer);
        fprintf(stdout, "Decoding delta values...\n");
        const auto decompressedIndexPairs = decode_deltas_from_fibonacci_sequence(bitStream, fibSeq);

        m_index.clear();
        for (const auto&[term, deltaVector] : decompressedIndexPairs)
        {
            const auto ids = reconstruct_from_delta(deltaVector);
            const auto occurrencies = azgra::collection::select(ids.begin(), ids.end(),
                                                                [](const DocId &docId)
                                                                {
                                                                    return DocumentOccurence(docId, 0);
                                                                });
            const auto documentIds = azgra::collection::vector_as_set(occurrencies);
            m_index.emplace(term, documentIds);
            //m_index[term] = documentIds;
        }
        fprintf(stdout, "Loaded index with %lu terms.\n", m_index.size());

    }

    VectorModel &SgmlFileCollection::get_vector_model()
    {
        return m_vectorModel;
    }

    std::vector<size_t> generate_fibonacci_sequence(const size_t N)
    {
        int n = N + 1;
        std::vector<size_t> fibN(N);
        size_t a = 0;
        size_t b = 1;
        size_t i = 0;
        while (n-- > 1)
        {
            size_t t = a;
            a = b;
            b += t;
            fibN[i++] = b;
        }
        //return b;
        return fibN;
    }
}