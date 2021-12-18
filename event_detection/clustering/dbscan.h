//
// Created by dietrich on 16.12.21.
//

#ifndef GEOBURST_OSM_DBSCAN_H
#define GEOBURST_OSM_DBSCAN_H

#include <vector>
#include <eigen3/Eigen/SparseCore>
#include <eigen3/Eigen/Dense>

#include "tweet_similarity/similarity_handler.h"
#include "sliding_window/sliding_window.h"

using EventTweet::SlidingWindow::SnapShot;
using EventTweet::TweetSimilarity::TextualSimilarityHandler;
using Eigen::SparseVector;
using Eigen::Matrix;
using Eigen::ColMajor;
using Eigen::RowMajor;
using Eigen::Dynamic;

namespace EventTweet::DBSCAN {

    using DistMapType = Matrix<double, Dynamic, Dynamic, ColMajor>;

    struct Point {

    };

    class DBSCAN {
    private:
        DistMapType dist_map;

    public:
        DBSCAN(TweetSimilarity::TweetSimilarityHandler& tweet_similarity_handler);
    };
}
#endif //GEOBURST_OSM_DBSCAN_H
