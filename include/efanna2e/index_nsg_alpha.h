#pragma once

#include "util.h"
#include "parameters.h"
#include "neighbor.h"
#include "index.h"
#include <cassert>
#include <unordered_map>
#include <string>
#include <sstream>
#include <stack>

namespace efanna2e
{

    class IndexNSGAlpha : public Index
    {
    public:
        explicit IndexNSGAlpha(const size_t dimension, const size_t n, Metric m, Index *initializer);

        virtual ~IndexNSGAlpha();

        virtual void Save(const char *filename) override;
        virtual void Load(const char *filename) override;

        virtual void Build(size_t n, const float *data, const Parameters &parameters) override;

        virtual void Search(
            const float *query,
            const float *x,
            size_t k,
            const Parameters &parameters,
            unsigned *indices) override;
        void SearchWithOptGraph(
            const float *query,
            size_t K,
            const Parameters &parameters,
            unsigned *indices);
        void OptimizeGraph(float *data);

    protected:
        typedef std::vector<std::vector<unsigned>> CompactGraph;
        typedef std::vector<SimpleNeighbors> LockGraph;
        typedef std::vector<nhood> KNNGraph;

        CompactGraph final_graph_;

        Index *initializer_ = nullptr;
        void init_graph(const Parameters &parameters);
        void get_neighbors(
            const float *query,
            const Parameters &parameter,
            std::vector<Neighbor> &retset,
            std::vector<Neighbor> &fullset);
        void get_neighbors(
            const float *query,
            const Parameters &parameter,
            std::vector<bool> &flags,
            std::vector<Neighbor> &retset,
            std::vector<Neighbor> &fullset);
        // void add_cnn(unsigned des, Neighbor p, unsigned range, LockGraph& cut_graph_);
        void InterInsert(unsigned n, unsigned range, std::vector<std::mutex> &locks, SimpleNeighbor *cut_graph_);
        void sync_prune(unsigned q, std::vector<Neighbor> &pool, const Parameters &parameter, std::vector<bool> &flags, SimpleNeighbor *cut_graph_);
        void Link(const Parameters &parameters, SimpleNeighbor *cut_graph_);
        void Load_nn_graph(const char *filename);
        void tree_grow(const Parameters &parameter);
        void DFS(std::vector<bool> &flag, unsigned root, unsigned &cnt);
        void findroot(std::vector<bool> &flag, unsigned &root, const Parameters &parameter);

    private:
        unsigned width;
        unsigned ep_;
        std::vector<std::mutex> locks;
        char *opt_graph_ = nullptr;
        size_t node_size;
        size_t data_len;
        size_t neighbor_len;
        KNNGraph nnd_graph;
    };
}
