#pragma once

#include <random>
#include <azgra/matrix.h>
#include <azgra/collection/enumerable_functions.h>

inline float mse(const azgra::Matrix<float> &simMat, const size_t col, const std::vector<float> &centroid)
{
    float result = 0;
    always_assert(simMat.rows() == centroid.size());
    for (size_t r = 0; r < centroid.size(); ++r)
    {
        result += pow((simMat.at(r, col) - centroid[r]), 2);
    }
    result = sqrt(result);
    return result;
}

inline float dot(const azgra::Matrix<float> &simMat, const size_t col, const std::vector<float> &centroid)
{
    float result = 0;
    always_assert(simMat.rows() == centroid.size());
    for (size_t r = 0; r < centroid.size(); ++r)
    {
        result += (simMat.at(r, col) * centroid[r]);
    }

    return result;
}

struct Cluster
{
    bool changed;
    std::vector<float> centroid;
    std::vector<size_t> documents;

    void clear_documents()
    {
        documents.clear();
    }

    float get_avg_sim(const azgra::Matrix<float> &termDocMat) const
    {
        if (documents.empty())
        { return 0; }
        float simVal = 0.0f;
        for (const size_t docId : documents)
        {
            float val = dot(termDocMat, docId, centroid);
            always_assert(!isinf(val));
//            if (isinf(val))
//            {
//                val = 0.0f;
//                //fprintf(stdout, "why is inf\n");
//            }

            simVal += val;
        }
        simVal /= static_cast<float>(documents.size());
        return simVal;
    }

    void recalculate_centroid(const azgra::Matrix<float> &termDocMat)
    {
        std::vector<float> newCentroid(centroid.size());
        const size_t docCount = documents.size();
        for (size_t r = 0; r < termDocMat.rows(); ++r)
        {
            newCentroid[r] = 0.0f;
            for (const size_t docId : documents)
            {
                newCentroid[r] += termDocMat.at(r, docId);
            }
            newCentroid[r] /= static_cast<float>(docCount);
        }

        changed = false;
        for (size_t i = 0; i < newCentroid.size(); ++i)
        {
            if (newCentroid[i] != centroid[i])
            {
                changed = true;
                break;
            }
        }
        centroid = std::move(newCentroid);
    }
};

class DocumentClusterer
{
private:
    azgra::Matrix<float> m_termDocMatrix;
    size_t m_clusterCount;
    size_t m_documentCount;
public:
    explicit DocumentClusterer(azgra::Matrix<float> &termDocMatrix, const size_t k);

    void clusterize();
};
