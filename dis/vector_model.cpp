#include <sstream>
#include "vector_model.h"
#include "porter_stemmer.h"

namespace dis
{

    ////////////////////////////// TermInfo implementation //////////////////////////////

    void TermInfo::add_document_occurence(const DocumentOccurence &occurence)
    {
        termDocumentInfos[occurence.docId] = TermDocumentInfo(occurence.occurenceCount);
    }

    void TermInfo::add_to_magnitudes(std::vector<float> &occurenceMagnitude, std::vector<float> &weightMagnitude) const
    {
        for (const auto &[key, value] : termDocumentInfos)
        {
            occurenceMagnitude[key] += pow(value.count, 2);
            weightMagnitude[key] += pow(value.weight, 2);
        }
    }

    void TermInfo::apply_normalization(const std::vector<float> &occurenceMagnitude, const std::vector<float> &weightMagnitude)
    {
        for (auto &[key, value] : termDocumentInfos)
        {
#if DEBUG
            assert(!isnan(occurenceMagnitude[key]) && "occurenceMagnitude is NaN");
            assert(!isnan(weightMagnitude[key]) && "weightMagnitude is NaN");
#endif
            value.normalizedCount = static_cast<float>(value.count) / occurenceMagnitude[key];
            value.normalizedWeight = static_cast<float>(value.weight) / weightMagnitude[key];
        }
    }

    void TermInfo::calculate_document_weights()
    {
        for (auto &[key, value] : termDocumentInfos)
        {
            assert(value.count > 0);
            //termFreqValue == 0.0 ? 0.0 : (termFreqValue * invTermDocFreq);
            value.weight = static_cast<float>(value.count) * invDocFreq;
        }
    }

    void TermInfo::fill_in_tf_matrices(const size_t row, azgra::Matrix<float> &termDocument_tf_mat, azgra::Matrix<float> &termDocument_tfidf_mat) const 
    {
        for (const auto&[docId, docTermInfo] : termDocumentInfos)
        {
            termDocument_tf_mat.at(row, docId) = docTermInfo.normalizedCount;
            termDocument_tfidf_mat.at(row, docId) = docTermInfo.normalizedWeight;
        }
    }

    ////////////////////////////// VectorModel implementation //////////////////////////////

    VectorModel::VectorModel(const TermIndex &index, const size_t documentCount)
    {
        m_documentCount = documentCount;
        m_termCount = index.size();
        create_vector_model(index);
    }

    void VectorModel::create_vector_model(const TermIndex &index)
    {
        initialize_term_info(index);
        normalize_model();
    }

    void VectorModel::initialize_term_info(const TermIndex &index)
    {
        size_t totalTermOccurence;
        for (const auto&[term, termOccurencies] : index)
        {
            totalTermOccurence = 0;
            m_terms[term] = TermInfo();
            TermInfo &termInfo = m_terms.at(term); 
            for (const auto &docOccurence : termOccurencies)
            {
                if (docOccurence.occurenceCount > 0)
                {
                    totalTermOccurence += docOccurence.occurenceCount;
                    termInfo.add_document_occurence(docOccurence);
                }
            }
            termInfo.invDocFreq = log10(static_cast<azgra::f32>(m_documentCount) / static_cast<azgra::f32>(totalTermOccurence));
            termInfo.calculate_document_weights();
        }
        fprintf(stdout, "Initialized vector model\n");
        m_initialized = true;
    }

    void VectorModel::normalize_model()
    {
        // NOTE(Moravec): DocId starts from 1 not from zero.
        std::vector<float> occurenceMagnitude(m_documentCount + 1, 0.0);
        std::vector<float> weightMagnitude(m_documentCount + 1, 0.0);

        for (const auto&[term, termInfo] : m_terms)
        {
            termInfo.add_to_magnitudes(occurenceMagnitude, weightMagnitude);
        }
        fprintf(stdout, "Calculated magnitutes..\n");
        for (auto&[term, termInfo] : m_terms)
        {
            termInfo.apply_normalization(occurenceMagnitude, weightMagnitude);
        }
        fprintf(stdout, "Applied normalization to vector model...\n");
    }

    azgra::f32 VectorModel::dot(const azgra::Matrix<float> &mat, const size_t col1, const size_t col2) const
    {
        always_assert(col1 < mat.cols() && col2 < mat.cols());
        azgra::f32 result = 0.0;

        for (size_t row = 0; row < mat.rows(); row++)
        {
            result += (mat.at(row, col1) * mat.at(row, col2));
        }
        return result;
    }

    void VectorModel::evaluate_vector_query(std::vector<DocumentScore> &scores, const std::vector<std::pair<std::string, azgra::f32>> &vectorQueryTerm) const
    {
        for (const auto &[term, termQueryValue] : vectorQueryTerm)
        {
            const TermInfo &termInfo = m_terms.at(term);
            for (const auto &[docId, termDocInfo] : termInfo.termDocumentInfos)
            {
                scores[docId].score += (termDocInfo.normalizedWeight * termQueryValue);
            } 
        }
    }

    std::vector<DocId> VectorModel::query_documents(const azgra::BasicStringView<char> &queryTxt) const
    {
        std::vector<DocId> result;
        azgra::string::SmartStringView queryText(queryTxt);
        if (queryText.is_empty())
        {
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "Query string is empty.\n");
            return result;
        }
        if (!m_initialized)
        {
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "Index wasn't created nor loaded.\n");
            return result;
        }

        const auto queryVector = create_normalized_query_vector(queryTxt);

        std::vector<DocumentScore> documentScore(m_documentCount);
        for (size_t docId = 0; docId < m_documentCount; docId++)
        {
            documentScore[docId].documentId = docId;
        }
        
        evaluate_vector_query(documentScore, queryVector);


        std::sort(documentScore.begin(), documentScore.end(), std::greater<>());

        std::stringstream docStream;
        result.resize(10);
        for (int i = 0; i < 10; ++i)
        {
            docStream << "Document: " << documentScore[i].documentId << " with score: " << documentScore[i].score << '\n';
            result[i] = documentScore[i].documentId;
        }

        fprintf(stdout, "\n%s\n", docStream.str().c_str());

        return result;
    }

    std::pair<DocId, DocId> VectorModel::find_most_similar_document(const size_t docId, 
                                                                    const azgra::Matrix<float> &termDocument_tf_mat, 
                                                                    const azgra::Matrix<float> &termDocument_tfidf_mat) const
    {
        float bestTfSimilarity = 0.0f;
        float bestTfIdfSimilarity = 0.0f;
        DocId tfDoc = 0,tfIdfDoc = 0;
        float currentTfSimilarity, currentTfIdfSimilarity;
        for (size_t otherDocId = 0; otherDocId < m_documentCount; otherDocId++)
        {
            if (otherDocId == docId)
            {
                continue;
            }

            currentTfSimilarity = dot(termDocument_tf_mat, docId, otherDocId);
            currentTfIdfSimilarity = dot(termDocument_tfidf_mat, docId, otherDocId);

            if (currentTfSimilarity > bestTfSimilarity)
            {
                bestTfSimilarity = currentTfSimilarity;
                tfDoc = otherDocId;
            }

            if (currentTfIdfSimilarity > bestTfIdfSimilarity)
            {
                bestTfIdfSimilarity = currentTfIdfSimilarity;
                tfIdfDoc = otherDocId;
            }
        }

        return std::make_pair(tfDoc, tfIdfDoc);
    }

    void VectorModel::save_most_similar_documents(const char *tfSimilarityFile) const
    {
        // Create document-term matrix.
        const size_t termCount = m_terms.size();
       always_assert(m_termCount == termCount);

        azgra::Matrix<float> termDocument_tf_mat(termCount, m_documentCount, 0.0);
        azgra::Matrix<float> termDocument_tfidf_mat(termCount, m_documentCount, 0.0);

        size_t rowIndex = 0;
        for (const auto&[term, termInfo] : m_terms)
        {
            termInfo.fill_in_tf_matrices(rowIndex++, termDocument_tf_mat, termDocument_tfidf_mat);
        }
        fprintf(stdout, "Constructed term document matrices...\n");

        std::ofstream resultStream(tfSimilarityFile, std::ios::out);
        always_assert(resultStream.is_open());
        resultStream << "Document;TF_MostSimilar;TF_IDF_MostSimilar" << '\n';

//#pragma omp parallel for
        for (size_t docId = 0; docId < m_documentCount; docId++)
        {
            auto [tfSimilar, tfidfSimilar] = find_most_similar_document(docId, termDocument_tf_mat, termDocument_tfidf_mat);   
            resultStream << docId << ';' << tfSimilar << ';' << tfidfSimilar << '\n';

            if (docId % 50 == 0)
            {
                fprintf(stdout, "Finished document %lu/%lu\n",docId, m_documentCount );
            }
        }
    }

    // void VectorModel::save(const char *filePath) const
    // {
    //     azgra::io::stream::OutBinaryFileStream str(filePath);
    //     str.write_ulong64(m_documentCount);
    //     str.write_ulong64(m_termCount);

    //     for (const auto &term : m_terms)
    //     {
    //         const size_t termLen = term.first.length();
    //         str.write_ulong64(termLen);
    //         str.write_bytes_from_buffer(term.first.data(), termLen);
    //     }
    //     // Just for checking
    //     str.write_byte(0);
    //     for (const azgra::f32 &value : m_termFreqWeights.get_data())
    //     {
    //         str.write_float(value);
    //     }
    //     // Just for checking
    //     str.write_byte(0);
    // }

    // void VectorModel::load(const char *filePath)
    // {
    //     azgra::io::stream::InBinaryFileStream str(filePath);
    //     m_documentCount = str.consume_ulong64();
    //     m_termCount = str.consume_ulong64();

    //     for (size_t tId = 0; tId < m_termCount; ++tId)
    //     {
    //         const size_t termLen = str.consume_ulong64();
    //         const auto termBytes = str.consume_bytes(termLen);
    //         always_assert(termLen == termBytes.size());
    //         std::string term((const char *) termBytes.data(), termLen);
    //         m_terms[term] = tId;
    //     }

    //     const azgra::byte check1 = str.consume_byte();
    //     always_assert(check1 == 0);

    //     const size_t floatValueCount = m_termCount * m_documentCount;
    //     const size_t readSize = floatValueCount * sizeof(azgra::f32);
    //     const auto matrixData = str.consume_bytes(readSize);

    //     std::vector<azgra::f32> values(floatValueCount);
    //     memcpy(values.data(), matrixData.data(), readSize);

    //     m_termFreqWeights = azgra::Matrix<azgra::f32>(m_termCount, m_documentCount, values);
    //     const azgra::byte check2 = str.consume_byte();
    //     always_assert(check2 == 0);
    //     m_initialized = true;
    // }

    std::vector<std::pair<std::string, azgra::f32>> VectorModel::create_normalized_query_vector(const azgra::BasicStringView<char> &queryTxt) const
    {
        const auto keywords = azgra::string::SmartStringView(queryTxt).split(" ");
        size_t correctKeywordCount = azgra::collection::count_if(keywords.begin(), keywords.end(),
                                                                 [](const azgra::string::SmartStringView<char> &s)
                                                                 {
                                                                     return !s.is_empty();
                                                                 });
        always_assert(correctKeywordCount <= keywords.size());
        const azgra::f32 denumerator = sqrt(static_cast<azgra::f32>(correctKeywordCount));
        std::vector<std::pair<std::string, azgra::f32>> queryVector(correctKeywordCount);
        int index = 0;
        for (const auto &keyword : keywords)
        {
            const AsciiString str = stem_word(keyword.data(), keyword.length());
            const std::string term = std::string(str.get_c_string());
            queryVector[index++] = std::make_pair(term, (1.0f / denumerator));
        }
        return queryVector;
    }
}