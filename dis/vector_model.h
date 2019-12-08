#pragma once

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

        void add_document_occurence(const DocumentOccurence &occurence)
        {
            termDocumentInfos[occurence.docId] = TermDocumentInfo(occurence.occurenceCount);
        }

        void add_to_magnitudes(std::vector<float> &occurenceMagnitude, std::vector<float> &weightMagnitude) const
        {
            for (const auto &[key, value] : termDocumentInfos)
            {
                occurenceMagnitude[key] += pow(value.count, 2);
                weightMagnitude[key] += pow(value.weight, 2);
            }
        }

        void apply_normalization(const std::vector<float> &occurenceMagnitude, const std::vector<float> &weightMagnitude)
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

        void calculate_document_weights()
        {
            for (auto &[key, value] : termDocumentInfos)
            {
                assert(value.count > 0);
                //termFreqValue == 0.0 ? 0.0 : (termFreqValue * invTermDocFreq);
                value.weight = static_cast<float>(value.count) * invDocFreq;
            }
        }
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

        [[nodiscard]] float dot(const std::vector<float> &a, const std::vector<float> &b) const;
        void evaluate_vector_query(std::vector<DocumentScore> &scores, const std::vector<std::pair<std::string, float>> &vectorQueryTerm) const;

        void normalize_model();

    public:
        VectorModel() = default;

        explicit VectorModel(const TermIndex &index, const size_t documentCount);

        [[nodiscard]] std::vector<DocId> query_documents(const azgra::BasicStringView<char> &queryText) const;

        // void save(const char *filePath) const;

        // void load(const char *filePath);
    };
}