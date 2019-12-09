#pragma once

#include <azgra/matrix.h>
#include <azgra/io/stream/out_binary_file_stream.h>
#include <azgra/io/stream/in_binary_file_stream.h>
#include <azgra/io/stream/in_binary_buffer_stream.h>
#include <azgra/collection/enumerable.h>
#include "term_index.h"

namespace dis
{
    struct DocumentScore
    {
        DocId documentId{};
        azgra::f64 score{};

        DocumentScore()
        {
            score = 0.0f;
        }

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

    struct TermDocumentInfo
    {
        size_t count{};
        float weight{};
        float normalizedCount{};
        float normalizedWeight{};

        TermDocumentInfo() = default;

        explicit TermDocumentInfo(const size_t documentCount) : count(documentCount)
        {

        }

        
    };


    struct TermInfo
    {
        std::map<DocId, TermDocumentInfo> termDocumentInfos{};
        float invDocFreq{};

        TermInfo() = default;

        void add_document_occurence(const DocumentOccurence &occurence);

        void add_to_magnitudes(std::vector<float> &occurenceMagnitude, std::vector<float> &weightMagnitude) const;

        void apply_normalization(const std::vector<float> &occurenceMagnitude, const std::vector<float> &weightMagnitude);

        void calculate_document_weights();

        void fill_in_tf_matrices(const size_t row, azgra::Matrix<float> &termDocument_tf_mat, azgra::Matrix<float> &termDocument_tfidf_mat) const;
    };

    class VectorModel
    {
    private:
        size_t m_documentCount;
        size_t m_termCount;
        std::map<std::string, TermInfo> m_terms;
        bool m_initialized = false;

        void create_vector_model(const TermIndex &index);

        void initialize_term_info(const TermIndex &index);

        [[nodiscard]] std::vector<std::pair<std::string, float>> create_normalized_query_vector(const azgra::BasicStringView<char> &queryTxt) const;

        [[nodiscard]] float dot(const azgra::Matrix<float> &mat, const size_t col1, const size_t col2) const;
        void evaluate_vector_query(std::vector<DocumentScore> &scores, const std::vector<std::pair<std::string, float>> &vectorQueryTerm) const;

        void normalize_model();

        std::pair<DocId, DocId> find_most_similar_document(const size_t docId, const azgra::Matrix<float> &termDocument_tf_mat, const azgra::Matrix<float> &termDocument_tfidf_mat) const;

    public:
        VectorModel() = default;

        explicit VectorModel(const TermIndex &index, const size_t documentCount);

        [[nodiscard]] std::vector<DocId> query_documents(const azgra::BasicStringView<char> &queryText) const;
        
        void save_most_similar_documents(const char *tfSimilarityFile) const;

        // void save(const char *filePath) const;

        // void load(const char *filePath);
    };
}