//
// Created by dietrich on 16.01.22.
//

#include "clustering.h"

namespace EventTweet::Clustering {

    BaseClustering::BaseClustering(SnapShot &_snapshot, TweetSimilarity::TweetSimilarityHandler &tweet_similarity_handler,
            ConfigFileHandler &config_file_handler) : snapshot(_snapshot){
            auto& spatial_map = tweet_similarity_handler.GetSpatialDistMap();
            auto& textual_map = tweet_similarity_handler.GetTextualDistMap();
            dist_map = spatial_map;
            dist_map += textual_map;
            minimum_points = config_file_handler.GetValue("minimum_points", 10);
            epsilon = config_file_handler.GetValue("epsilon", 10.0);

            auto& tweet_map = snapshot.GetTweetMap();
            int index = 0;
            for (auto& tweet_kv: tweet_map) {
                Tweet& tweet = tweet_kv.second;
                Point point(tweet);
                point.index = index++;
                points.emplace_back(std::move(point));
            }

            tweet_position_map = tweet_similarity_handler.GetTweetPositionMap();
    }

    BaseClustering::~BaseClustering(){
        dist_map.setZero();
        points.clear();
        tweet_position_map.clear();
        minimum_points = 0;
        epsilon = 0.;
    }

    std::vector<int> BaseClustering::CalculateCluster(Point& point_lhs) {
        int index = 0;
        std::vector<int> cluster_index;
        for (auto& point_rhs : points) {
            if (point_lhs.tweet_id != point_rhs.tweet_id && CalculateDistance(point_lhs, point_rhs) <= epsilon) {
                cluster_index.push_back(index);
            }
            ++index;
        }
        return cluster_index;
    }

    double BaseClustering::CalculateDistance(const Point& point_lhs, const Point& point_rhs) {
        const std::string& tweet_lhs_id = point_lhs.tweet_id;
        const std::string& tweet_rhs_id = point_rhs.tweet_id;
        int index_lhs = tweet_position_map[tweet_lhs_id];
        int index_rhs = tweet_position_map[tweet_rhs_id];

        double distance = dist_map(index_lhs, index_rhs);
        return distance;
    }
}

