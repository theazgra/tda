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
        azgra::Matrix<size_t> termFrequencyMatrix(m_termCount, m_documentCount);
        create_term_frequency_matrix(index, termFrequencyMatrix);

        // Calculate inverse document frequency idf_t
        const auto invDocFreq = create_inverse_document_frequency_for_terms(termFrequencyMatrix);

        m_termFreqWeights = azgra::Matrix<azgra::f64>(m_termCount, m_documentCount, 0.0);
        // Calculate term frequency weights from inverse document frequencies
        calculate_term_frequency_weights(termFrequencyMatrix, invDocFreq);


//        Vraťte 10 dokumentů s nejvyšším skóre.


    }

    void VectorModel::create_term_frequency_matrix(const TermIndex &index, azgra::Matrix<size_t> &termFrequencyMatrix)
    {
        size_t rowIndex = 0;
        for (const auto&[term, termOccurencies] : index)
        {
            m_terms[term] = rowIndex;
            for (const auto &termOccurence : termOccurencies)
            {
                termFrequencyMatrix.at(rowIndex, termOccurence.docId) = termOccurence.occurenceCount;
            }
            ++rowIndex;
        }
        fprintf(stdout, "Created term freq matrix.\n");
    }

    std::vector<azgra::f64> VectorModel::create_inverse_document_frequency_for_terms(const azgra::Matrix<size_t> &termFreq)
    {
        //
        std::vector<azgra::f64> termInvDocFreq(m_termCount);
        for (size_t termId = 0; termId < m_termCount; ++termId)
        {
            const auto &rowBegin = termFreq.row_begin(termId);
            const auto &rowEnd = termFreq.row_end(termId);
            const size_t termDocumentCount = azgra::collection::sum(rowBegin, rowEnd, 0);
            termInvDocFreq[termId] = log10(static_cast<azgra::f64>(m_documentCount) / static_cast<azgra::f64>(termDocumentCount));
        }
        fprintf(stdout, "Calculated inverse document frequency for terms\n");
        return termInvDocFreq;
    }

    void VectorModel::calculate_term_frequency_weights(const azgra::Matrix<size_t> &termFreq, const std::vector<azgra::f64> &invDocFreq)
    {
        // Spočítejte tf-idf váhy: tf-idf_t_d = tf_t_d * idf_t
        for (size_t termRow = 0; termRow < termFreq.rows(); ++termRow)
        {
            const azgra::f64 invTermDocFreq = invDocFreq[termRow];
            for (size_t docCol = 0; docCol < termFreq.cols(); ++docCol)
            {

                m_termFreqWeights.at(termRow, docCol) = (termFreq.at(termRow, docCol) * invTermDocFreq);
            }
        }
        fprintf(stdout, "Calculated term frequency weights \n");
        m_initialized = true;
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

        std::vector<DocumentScore> documentScore(m_documentCount);
        for (size_t i = 0; i < m_documentCount; ++i)
        {
            documentScore[i] = DocumentScore(i, 0.0);
        }

        const auto keywords = queryText.split(" ");
        for (const auto &keyword : keywords)
        {
            AsciiString str = stem_word(keyword.data(), keyword.length());
            const std::string key = std::string(str.get_c_string());
            const size_t termIndex = m_terms.at(key);


            for (size_t documentCol = 0; documentCol < m_documentCount; ++documentCol)
            {
                documentScore[documentCol].score += m_termFreqWeights.at(termIndex, documentCol);
            }
        }

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
}