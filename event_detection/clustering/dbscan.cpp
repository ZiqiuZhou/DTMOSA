//
// Created by dietrich on 16.12.21.
//
#include "dbscan.h"

namespace EventTweet::DBSCAN {

    DBSCAN::DBSCAN(SnapShot &_snapshot, TweetSimilarity::TweetSimilarityHandler &tweet_similarity_handler,
                   ConfigFileHandler &config_file_handler) {
        snapshot = _snapshot;
        auto& spatial_map = tweet_similarity_handler.GetSpatialDistMap();
        auto& textual_map = tweet_similarity_handler.GetTextualDistMap();
        dist_map = 0.5 * spatial_map;
        dist_map += (0.5 * textual_map);
        minimum_points = config_file_handler.GetValue("minimum_points", 10);
        epsilon = config_file_handler.GetValue("epsilon", 10.0);

        auto& tweet_map = snapshot.GetTweetMap();
        for (auto& tweet_kv: tweet_map) {
            Tweet tweet = tweet_kv.second;
            Point point(tweet);
            points.emplace_back(std::move(point));
        }
    }

    DBSCAN::~DBSCAN() {
        dist_map.setZero();
        snapshot.Reset();
        points.clear();
        minimum_points = 0;
        epsilon = 0.;
    }

    int DBSCAN::Cluster() {
        int cluster_id = 1;
        if (points.empty() || points.size() == 1) {
            return FAILURE;
        }
        for (auto iter = points.begin(); iter != points.end(); ++iter) {
            Point& point = *iter;
            if (point.cluster_id == UNCLASSIFIED) {

            }
        }
    }

    int DBSCAN::ExpandCluster(Point& point, int cluster_id) {

    }

//    std::vector<int> DBSCAN::CalculateCluster(Point& point) {
//        int index = 0;
//        std::vector<int> clusterIndex;
//        for (auto iter = points.begin(); iter != points.end(); ++iter) {
//            if ()
//        }
//    }

    double DBSCAN::CalculateDistance(const Point& point_lhs, const Point& point_rhs) {
        const std::string& tweet_lhs_id = point_lhs.tweet_id;
        const std::string& tweet_rhs_id = point_rhs.tweet_id;

    }
}