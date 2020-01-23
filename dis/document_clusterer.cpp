#include "document_clusterer.h"

using namespace azgra;
using namespace azgra::collection;

DocumentClusterer::DocumentClusterer(azgra::Matrix<float> &termDocMatrix, const size_t k)
{
    m_termDocMatrix = std::move(termDocMatrix);
    m_clusterCount = k;
    m_documentCount = termDocMatrix.rows();
}

static std::vector<size_t> generate_random_indices(const size_t k, const size_t maxInc)
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<size_t> rand(0, maxInc - 1);
    std::vector<size_t> indices(k);
    size_t rnd;

    for (size_t i = 0; i < k; ++i)
    {
        rnd = rand(mt);
        while (azgra::collection::contains(indices.begin(), indices.end(), rnd))
        {
            rnd = rand(mt);
        }
        indices[i] = rnd;
    }
    return indices;
}


void DocumentClusterer::clusterize()
{
    auto indices = generate_random_indices(m_clusterCount, m_termDocMatrix.cols());

    std::vector<Cluster> clusters = select(indices.begin(), indices.end(), [this](const size_t docId)
    {
        Cluster c = {};
        c.centroid = this->m_termDocMatrix.col(docId);
        return c;
    });

    float sim, maxSim;
    size_t closestClusterIndex;
    size_t it = 0;
    while (true)
    {
        for (Cluster &c : clusters)
        {
            c.clear_documents();
        }

        for (size_t docId = 0; docId < m_documentCount; ++docId)
        {
            maxSim = -1.0f;
            for (size_t clusterIndex = 0; clusterIndex < m_clusterCount; ++clusterIndex)
            {
                sim = dot(m_termDocMatrix, docId, clusters[clusterIndex].centroid);
                if (sim > maxSim)
                {
                    maxSim = sim;
                    closestClusterIndex = clusterIndex;
                }
            }
            clusters[closestClusterIndex].documents.push_back(docId);
        }

        float totalAvgSim = 0.0f;
        for (Cluster &c : clusters)
        {
            c.recalculate_centroid(m_termDocMatrix);
            totalAvgSim += c.get_avg_sim(m_termDocMatrix);
        }
        totalAvgSim /= static_cast<float>(clusters.size());

        if (!any(
                clusters.begin(),
                clusters.end(),
                [](const Cluster &c)
                { return c.changed; }))
        {
            break;
        }
        fprintf(stdout, "k-Means iteration %lu Total average MSE: %.8f\n", ++it, totalAvgSim);
    }
    fprintf(stdout, "k-Means finished..\n");

}
