//
// Created by dietrich on 14.11.21.
//

#include "random_walk_with_restart.h"

namespace EventTweet::RWR {

    RandomWalkWithRestart::~RandomWalkWithRestart() {
        relevance_score.setZero();
        restart_vector.setZero();
        adjacent_matrix_ptr.reset();
    }

    void RandomWalkWithRestart::InitScore() {
        SparseMatrix<double, ColMajor>& adjacent_matrix = *adjacent_matrix_ptr;
        if (adjacent_matrix.isVector()) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " Invalid adjacent matrix." << std::endl;
            return ;
        }
        // construct a sparse identity matrix
        auto SIZE = adjacent_matrix.cols();
        if (SIZE != relevance_score.rows()) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " construct a sparse identity matrix failed." << std::endl;
            return ;
        }

        SparseMatrix<double, ColMajor> Identity(SIZE, SIZE);
        Identity.setIdentity();
        SparseMatrix<double, ColMajor> system_matrix = Identity - alpha * adjacent_matrix;
        SimplicialLDLT<Eigen::SparseMatrix<double>> solver;
        solver.compute(system_matrix);
        if(solver.info()!= Success) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " Invalid system_matrix." << std::endl;
            return ;
        }
        Eigen::SparseMatrix<double, Eigen::ColMajor> Inv = solver.solve(Identity);
        relevance_score = (1 - alpha) * Inv * restart_vector;
        return ;
    }

    SparseVector<double, ColMajor>& RandomWalkWithRestart::Iterate() {
        int iter = 0;
        SparseMatrix<double, ColMajor>& adjacent_matrix = *adjacent_matrix_ptr;
        if (adjacent_matrix.isVector()) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " Invalid adjacent matrix." << std::endl;
            return relevance_score;
        }
        if ((adjacent_matrix.cols() != relevance_score.rows()) || (adjacent_matrix.cols() != restart_vector.rows())) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " Invalid matrix-vector sizes." << std::endl;
            return relevance_score;
        }

        while (iter < total_iteration) {
            relevance_score = alpha * adjacent_matrix * relevance_score + (1. - alpha) * restart_vector;
            ++iter;
        }
        return relevance_score;
    }
}
