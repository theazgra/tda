#pragma once

#include <azgra/matrix.h>
#include <azgra/collection/enumerable.h>
#include "term_index.h"

namespace dis
{
    struct DocumentScore
    {
        DocId documentId{};
        azgra::f64 score{};

        DocumentScore() = default;

        DocumentScore(const DocId id, const azgra::f64 initialScore) : documentId(id), score(initialScore)
        {

        }

        bool operator<(const DocumentScore &other) const
        {
            return (score < other.score);
        }

        bool operator>(const DocumentScore &other) const
        {
            return (score > other.score);
        }
    };

    class VectorModel
    {
    private:
        size_t m_documentCount;
        size_t m_termCount;
        std::map<std::string, size_t> m_terms;
        azgra::Matrix<azgra::f64> m_termFreqWeights;
        bool m_initialized = false;

        void create_vector_model(const TermIndex &index);

        void create_term_frequency_matrix(const TermIndex &index, azgra::Matrix<size_t> &termFrequencyMatrix);

        std::vector<azgra::f64> create_inverse_document_frequency_for_terms(const azgra::Matrix<size_t> &termFreq);

        void calculate_term_frequency_weights(const azgra::Matrix<size_t> &termFreq, const std::vector<azgra::f64> &invDocFreq);



    public:
        VectorModel() = default;

        explicit VectorModel(const TermIndex &index, const size_t documentCount);

        std::vector<DocId> query_documents(const azgra::BasicStringView<char> &queryText) const;

    };
}