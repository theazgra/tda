#include <sstream>
#include "vector_model.h"
#include "porter_stemmer.h"

namespace dis
{
    VectorModel::VectorModel(const TermIndex &index, const size_t documentCount)
    {
        m_documentCount = documentCount;
        m_termCount = index.size();
        create_vector_model(index);
    }

    void VectorModel::create_vector_model(const TermIndex &index)
    {
        // Calculate term frequency tf_t_d for  each term in every document and save it in matrix

        fprintf(stdout, "Matrix: %lu rows, %lu cols\n", m_termCount, m_documentCount);
        create_term_frequency_matrix(index);

        // Calculate inverse document frequency idf_t
        const auto invDocFreq = create_inverse_document_frequency_for_terms();

        m_termFreqWeights = azgra::Matrix<azgra::f32>(m_termCount, m_documentCount, 0.0);
        // Calculate term frequency weights from inverse document frequencies
        calculate_term_frequency_weights(invDocFreq);

        normalize_matrices();
    }

    void VectorModel::create_term_frequency_matrix(const TermIndex &index)
    {
        m_termFreq = azgra::Matrix<azgra::f32>(m_termCount, m_documentCount, 0.0f);
        size_t rowIndex = 0;
        for (const auto&[term, termOccurencies] : index)
        {
            m_terms[term] = rowIndex;
            for (const auto &termOccurence : termOccurencies)
            {
                assert(termOccurence.occurenceCount >= 0 && "Term occurence is lower than 0");
                m_termFreq.at(rowIndex, termOccurence.docId) = termOccurence.occurenceCount;
            }
            ++rowIndex;
        }
        fprintf(stdout, "Created term freq matrix.\n");
    }

    std::vector<azgra::f32> VectorModel::create_inverse_document_frequency_for_terms()
    {
        //
        std::vector<azgra::f32> termInvDocFreq(m_termCount);
        for (size_t termId = 0; termId < m_termCount; ++termId)
        {
            const auto &rowBegin = m_termFreq.row_begin(termId);
            const auto &rowEnd = m_termFreq.row_end(termId);
            const size_t termDocumentCount = azgra::collection::sum(rowBegin, rowEnd, 0);
            termInvDocFreq[termId] = log10(static_cast<azgra::f32>(m_documentCount) / static_cast<azgra::f32>(termDocumentCount));
        }
        fprintf(stdout, "Calculated inverse document frequency for terms\n");
        return termInvDocFreq;
    }

    void VectorModel::calculate_term_frequency_weights(const std::vector<azgra::f32> &invDocFreq)
    {
        float termFreqValue;
        // Spočítejte tf-idf váhy: tf-idf_t_d = tf_t_d * idf_t
        for (size_t termRow = 0; termRow < m_termFreq.rows(); ++termRow)
        {
            const azgra::f32 invTermDocFreq = invDocFreq[termRow];
            for (size_t docCol = 0; docCol < m_termFreq.cols(); ++docCol)
            {
                termFreqValue = m_termFreq.at(termRow, docCol);
                m_termFreqWeights.at(termRow, docCol) = termFreqValue == 0.0 ? 0.0 : (termFreqValue * invTermDocFreq);

#if DEBUG
                // DEBUG
                if (isnan(m_termFreqWeights.at(termRow, docCol)))
                {
                    fprintf(stderr, "NaN in m_termFreqWeights at (%lu,%lu) m_termFreq value: %.4f invTermDocFreq value: %.4f\n",termRow,docCol, m_termFreq.at(termRow, docCol),invTermDocFreq);
                    assert(false && "Found NaN value.");
                }
#endif
            }
        }
        fprintf(stdout, "Calculated term frequency weights \n");
        m_initialized = true;
    }

    void VectorModel::normalize_matrix(azgra::Matrix<azgra::f32> &matrix)
    {
        
        for (size_t col = 0; col < matrix.cols(); ++col)
        {
            azgra::f32 magnitude = 0.0;
            const auto colValues = matrix.col(col);
            for (const azgra::f32 val : colValues)
            {
                magnitude += pow(val, 2);
                assert(!isnan(val) && "Value is NaN");
            }
            assert(!isnan(magnitude) && "Magnitude is NaN");
            for (size_t row = 0; row < matrix.rows(); ++row)
            {
                matrix.at(row, col) /= magnitude;
            }
        }
    }

    void VectorModel::normalize_matrices()
    {
        // Normalizing of m_termFreq
        fprintf(stdout, "Normalizing m_termFreq\n");
        normalize_matrix(m_termFreq);

        // Normalizing of m_termFreqWeights
        fprintf(stdout, "Normalizing m_termFreqWeights\n");
        normalize_matrix(m_termFreqWeights);

        fprintf(stdout, "Normalized matrices...\n");
    }

    azgra::f32 VectorModel::dot_with_term_pairs(const size_t docCol, const std::vector<std::pair<size_t, azgra::f32>> &vectorQueryTerm) const
    {
        azgra::f32 result = 0.0;

        for (const auto&[index, termValue] : vectorQueryTerm)
        {
            result += m_termFreqWeights.at(index, docCol) * termValue;
        }

        return result;        
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

        for (size_t docCol = 0; docCol < m_documentCount; ++docCol)
        {
            //const auto colValues = m_termFreqWeights.col(docCol);
            const azgra::f32 dotResult = dot_with_term_pairs(docCol, queryVector);
            documentScore[docCol] = DocumentScore(docCol, dotResult);
        }

//        const auto keywords = queryText.split(" ");
//        for (const auto &keyword : keywords)
//        {
//            AsciiString str = stem_word(keyword.data(), keyword.length());
//            const std::string key = std::string(str.get_c_string());
//            const size_t termIndex = m_terms.at(key);
//
//
//            for (size_t documentCol = 0; documentCol < m_documentCount; ++documentCol)
//            {
//                documentScore[documentCol].score += m_termFreqWeights.at(termIndex, documentCol);
//            }
//        }

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

    void VectorModel::save(const char *filePath) const
    {
        azgra::io::stream::OutBinaryFileStream str(filePath);
        str.write_ulong64(m_documentCount);
        str.write_ulong64(m_termCount);

        for (const auto &term : m_terms)
        {
            const size_t termLen = term.first.length();
            str.write_ulong64(termLen);
            str.write_bytes_from_buffer(term.first.data(), termLen);
        }
        // Just for checking
        str.write_byte(0);
        for (const azgra::f32 &value : m_termFreqWeights.get_data())
        {
            str.write_float(value);
        }
        // Just for checking
        str.write_byte(0);
    }

    void VectorModel::load(const char *filePath)
    {
        azgra::io::stream::InBinaryFileStream str(filePath);
        m_documentCount = str.consume_ulong64();
        m_termCount = str.consume_ulong64();

        for (size_t tId = 0; tId < m_termCount; ++tId)
        {
            const size_t termLen = str.consume_ulong64();
            const auto termBytes = str.consume_bytes(termLen);
            always_assert(termLen == termBytes.size());
            std::string term((const char *) termBytes.data(), termLen);
            m_terms[term] = tId;
        }

        const azgra::byte check1 = str.consume_byte();
        always_assert(check1 == 0);

        const size_t floatValueCount = m_termCount * m_documentCount;
        const size_t readSize = floatValueCount * sizeof(azgra::f32);
        const auto matrixData = str.consume_bytes(readSize);

        std::vector<azgra::f32> values(floatValueCount);
        memcpy(values.data(), matrixData.data(), readSize);

        m_termFreqWeights = azgra::Matrix<azgra::f32>(m_termCount, m_documentCount, values);
        const azgra::byte check2 = str.consume_byte();
        always_assert(check2 == 0);
        m_initialized = true;
    }

    std::vector<std::pair<size_t, azgra::f32>> VectorModel::create_normalized_query_vector(const azgra::BasicStringView<char> &queryTxt) const
    {
        const auto keywords = azgra::string::SmartStringView(queryTxt).split(" ");
        size_t correctKeywordCount = azgra::collection::count_if(keywords.begin(), keywords.end(),
                                                                 [](const azgra::string::SmartStringView<char> &s)
                                                                 {
                                                                     return !s.is_empty();
                                                                 });
        always_assert(correctKeywordCount <= keywords.size());
        const azgra::f32 denumerator = sqrt(static_cast<azgra::f32>(correctKeywordCount));
        std::vector<std::pair<size_t, azgra::f32>> queryVector(correctKeywordCount);
        int index = 0;
        for (const auto &keyword : keywords)
        {
            const AsciiString str = stem_word(keyword.data(), keyword.length());
            const std::string key = std::string(str.get_c_string());
            const size_t termIndex = m_terms.at(key);
            queryVector[index++] = std::make_pair(termIndex, (1.0f / denumerator));
        }
        return queryVector;
    }
}