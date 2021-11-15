//
// Created by dietrich on 14.11.21.
//

#ifndef GEOBURST_OSM_RANDOM_WALK_WITH_RESTART_H
#define GEOBURST_OSM_RANDOM_WALK_WITH_RESTART_H

#include <eigen3/Eigen/SparseCore>
#include <eigen3/Eigen/SparseCholesky>
#include <memory>

#include "sliding_window/sliding_window.h"
#include "co_occurrence_graph.h"

using EventTweet::SlidingWindow::SnapShot;
using EventTweet::Co_Occurrence_Graph::KeyWordGraph;
using Eigen::SparseMatrix;
using Eigen::SparseVector;
using Eigen::SimplicialLDLT;
using Eigen::ColMajor;
using Eigen::Success;

namespace EventTweet::RWR {

    class RandomWalkWithRestart {
    private:
        double alpha;
        int total_iteration;
        int vertex_number;
        int vertex_index;
        SparseVector<double, ColMajor> relevance_score;
        SparseVector<double, ColMajor> restart_vector;
        std::shared_ptr<SparseMatrix<double, ColMajor> > adjacent_matrix_ptr;

    public:
        RandomWalkWithRestart(double restart_probability,
                              int _iter, int number, int index,
                              SparseMatrix<double, ColMajor>& matrix) {
            alpha = restart_probability;
            total_iteration = _iter;
            vertex_number = number;
            vertex_index = index;
            restart_vector.resize(vertex_number);
            restart_vector.insert(vertex_index) = 1.0;
            relevance_score.resize(vertex_number);
            relevance_score = restart_vector;

            adjacent_matrix_ptr.reset();
            adjacent_matrix_ptr = std::make_shared<SparseMatrix<double, ColMajor>>(matrix);
        }

        ~RandomWalkWithRestart();

        void InitScore();

        SparseVector<double, ColMajor>& Iterate();
    };

//    class SemanticImpact {
//    private:
//
//    };
}

#endif //GEOBURST_OSM_RANDOM_WALK_WITH_RESTART_H
