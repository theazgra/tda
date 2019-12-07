#pragma once

#include <azgra/matrix.h>
#include <azgra/io/stream/out_binary_file_stream.h>
#include <azgra/io/stream/in_binary_file_stream.h>
#include <azgra/io/stream/in_binary_buffer_stream.h>
#include <azgra/collection/enumerable.h>
#include "term_index.h"

namespace dis
{
    inline azgra::f32 dot(const std::vector<azgra::f32> &a, const std::vector<azgra::f32> &b)
    {
        always_assert(a.size() == b.size());
        azgra::f32 result = 0.0;

        for (size_t i = 0; i < a.size(); ++i)
        {
            result += (a[i] * b[i]);
        }

        return result;
    }
    

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
        azgra::Matrix<azgra::f32> m_termFreq;
        azgra::Matrix<azgra::f32> m_termFreqWeights;
        bool m_initialized = false;

        void create_vector_model(const TermIndex &index);

        void create_term_frequency_matrix(const TermIndex &index);

        std::vector<azgra::f32> create_inverse_document_frequency_for_terms();

        void calculate_term_frequency_weights(const std::vector<azgra::f32> &invDocFreq);

        [[nodiscard]] std::vector<std::pair<size_t,azgra::f32>> create_normalized_query_vector(const azgra::BasicStringView<char> &queryTxt) const ;
        [[nodiscard]] azgra::f32 dot_with_term_pairs(const size_t docCol, const std::vector<std::pair<size_t, azgra::f32>> &vectorQueryTerm) const;

        void normalize_matrix(azgra::Matrix<azgra::f32> &matrix);
        void normalize_matrices();

    public:
        VectorModel() = default;

        explicit VectorModel(const TermIndex &index, const size_t documentCount);

        [[nodiscard]] std::vector<DocId> query_documents(const azgra::BasicStringView<char> &queryText) const;

        void save(const char *filePath) const;

        void load(const char *filePath);
    };
}