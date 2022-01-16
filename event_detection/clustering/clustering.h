//
// Created by dietrich on 15.01.22.
//

#ifndef GEOBURST_OSM_CLUSTERING_H
#define GEOBURST_OSM_CLUSTERING_H

#include <algorithm>
#include <limits>
#include <eigen3/Eigen/SparseCore>
#include <eigen3/Eigen/Dense>

#include "../pre_processing/Tweet.h"
#include "tweet_similarity/similarity_handler.h"
#include "sliding_window/sliding_window.h"
#include "../pre_processing/Tweet.h"
#include "../common/config_handler/config_handler.h"

using PreProcessing::TweetParser::Tweet;
using common::config_handler::ConfigFileHandler;
using EventTweet::SlidingWindow::SnapShot;
using EventTweet::TweetSimilarity::TextualSimilarityHandler;
using Eigen::SparseVector;
using Eigen::Matrix;
using Eigen::ColMajor;
using Eigen::RowMajor;
using Eigen::Dynamic;

namespace EventTweet::Clustering {

    using DistMapType = Matrix<double, Dynamic, Dynamic, ColMajor>;
    double const UNREACHABLE = std::numeric_limits<double>::max();

    enum STATE {
        FAILURE = -4,
        NOISE,
        UNDEFINED,
        UNPROCESSED,
        SUCCESS,
        PROCESSED
    };

    struct Point {
        std::string tweet_id;

        double longitude = 0.;

        double latitude = 0.;

        int cluster_id = UNDEFINED;

        // for OPTICS
        int processed = UNPROCESSED;

        double core_dist = std::numeric_limits<double>::max();

        double reachability_dist = UNREACHABLE;

        int neighbor_num = 0;

        std::unordered_multiset<std::string> word_bag;

        Point() {
            word_bag = {};
        }

        explicit Point(Tweet& tweet) {
            word_bag = {};
            tweet_id = tweet.GetTweetID();
            longitude = tweet.GetLongitude();
            latitude = tweet.GetLatitude();
            word_bag = tweet.GetWordBag();
        }

        ~Point() {
            word_bag.clear();
            tweet_id.clear();
            longitude = 0.;
            latitude = 0.;
            cluster_id = 0;
            processed = 0;
            core_dist = 0.;
            reachability_dist = 0.;
        }

        Point(const Point& _point) {
            this->longitude = _point.longitude;
            this->latitude = _point.latitude;
            this->cluster_id = _point.cluster_id;
            this->processed = _point.processed;
            this->core_dist = _point.core_dist;
            this->reachability_dist = _point.reachability_dist;
            this->tweet_id = _point.tweet_id;
            this->word_bag = _point.word_bag;
        }

        Point& operator=(const Point& _point) {
            if (this == &_point) {
                return *this;
            }
            Point temp_point = _point;
            std::swap(*this, temp_point);
            return *this;
        }

        Point(Point&& _point) noexcept {
            this->longitude = std::exchange(_point.longitude, 0.);
            this->latitude = std::exchange(_point.latitude, 0.);
            this->cluster_id = std::exchange(_point.cluster_id, 0);
            this->tweet_id = std::exchange(_point.tweet_id, "");
            this->processed = std::exchange(_point.processed, 0);
            this->core_dist = std::exchange(_point.core_dist, 0.);
            this->reachability_dist = std::exchange(_point.reachability_dist, 0.);
            this->word_bag = std::exchange(_point.word_bag, {});
        }

        Point& operator=(Point&& _point) noexcept {
            if (this == &_point) {
                return *this;
            }
            this->longitude = std::exchange(_point.longitude, 0.);
            this->latitude = std::exchange(_point.latitude, 0.);
            this->cluster_id = std::exchange(_point.cluster_id, 0);
            this->tweet_id = std::exchange(_point.tweet_id, "");
            this->processed = std::exchange(_point.processed, 0);
            this->core_dist = std::exchange(_point.core_dist, 0.);
            this->reachability_dist = std::exchange(_point.reachability_dist, 0.);
            this->word_bag = std::exchange(_point.word_bag, {});
            return *this;
        }
    };

    class BaseClustering {
    public:
        int minimum_points = 1;

        double epsilon = 1.;

        int cluster_id = 0;

        DistMapType dist_map;

        std::vector<Point> points;

        std::unordered_map<std::string, int> tweet_position_map;

        SnapShot& snapshot;

    public:
        BaseClustering(SnapShot &_snapshot, TweetSimilarity::TweetSimilarityHandler &tweet_similarity_handler,
                       ConfigFileHandler &config_file_handler);

        virtual ~BaseClustering();

    public:
        virtual int Cluster() = 0;

        std::vector<Point>& GetPoints() {
            return points;
        }

        int GetPointSize() {
            return points.size();
        }

        std::vector<int> CalculateCluster(Point& point_lhs);

        double CalculateDistance(const Point& point_lhs, const Point& point_rhs);
    };
}

#endif //GEOBURST_OSM_CLUSTERING_H
