//
// Created by dietrich on 16.12.21.
//
#include "dbscan.h"

namespace EventTweet::Clustering {
    DBSCAN::DBSCAN(SnapShot &_snapshot, TweetSimilarity::TweetSimilarityHandler &tweet_similarity_handler,
                   ConfigFileHandler &config_file_handler): snapshot(_snapshot) {
        auto& spatial_map = tweet_similarity_handler.GetSpatialDistMap();
        auto& textual_map = tweet_similarity_handler.GetTextualDistMap();
        double bandwidth = config_file_handler.GetValue("kernel_bandwidth", 80.);
        dist_map = 0.5 * spatial_map;
        dist_map += (0.5 * bandwidth * textual_map);
        minimum_points = config_file_handler.GetValue("minimum_points", 10);
        epsilon = config_file_handler.GetValue("epsilon", 10.0);

        auto& tweet_map = snapshot.GetTweetMap();
        for (auto& tweet_kv: tweet_map) {
            Tweet& tweet = tweet_kv.second;
            Point point(tweet);
            points.emplace_back(std::move(point));
        }

        tweet_position_map = tweet_similarity_handler.GetTweetPositionMap();
    }

    DBSCAN::~DBSCAN() {
        dist_map.setZero();
        //snapshot.Reset();
        points.clear();
        tweet_position_map.clear();
        minimum_points = 0;
        epsilon = 0.;
    }

    int DBSCAN::Cluster() {
        if (points.empty() || points.size() == 1) {
            return FAILURE;
        }
        for (auto& point : points) {
            if (point.cluster_id == UNCLASSIFIED) {
                if (ExpandCluster(point) == FAILURE) {
                    continue;
                }
            }
        }
        return SUCCESS;
    }

    int DBSCAN::ExpandCluster(Point& point) {
        std::vector<int> cluster_seeds = CalculateCluster(point);
        std::unordered_set<int> seeds_set = {};
        for (int pos: cluster_seeds) {
            seeds_set.insert(pos);
        }

        if (cluster_seeds.empty() || cluster_seeds.size() < minimum_points) {
            point.cluster_id = NOISE;
            return FAILURE;
        } else {
            cluster_id++;
            int index = 0;
            int index_core_point = 0;
            for (int pos : cluster_seeds) {
                if (pos >= points.size()) {
                    return FAILURE;
                }
                if (points[pos].tweet_id == point.tweet_id) {
                    index_core_point = index;
                    points[pos].cluster_id = cluster_id;
                    break;
                }
                ++index;
            }
            cluster_seeds.erase(cluster_seeds.begin() + index_core_point);

            for (std::size_t i = 0, n = cluster_seeds.size(); i < n; ++i) {
                int pos = cluster_seeds[i];
                Point& seed_point = points[pos];
                if (seed_point.cluster_id == NOISE) {
                    seed_point.cluster_id = cluster_id;
                }
                if (seed_point.cluster_id != UNCLASSIFIED) {
                    continue;
                }
                seed_point.cluster_id = cluster_id;

                std::vector<int> cluster_neighbors = CalculateCluster(seed_point);
                if (cluster_neighbors.size() >= minimum_points) {
                    for (int neighbor_pos: cluster_neighbors) {
                        if (seeds_set.find(neighbor_pos) == seeds_set.end()) {
                            cluster_seeds.push_back(neighbor_pos);
                            seeds_set.insert(neighbor_pos);
                        }
                    }
                    n = cluster_seeds.size();
                }
            }
            return SUCCESS;
        }
    }

    std::vector<int> DBSCAN::CalculateCluster(Point& point_lhs) {
        int index = 0;
        std::vector<int> cluster_index;
        for (auto& point_rhs : points) {
            if (CalculateDistance(point_lhs, point_rhs) <= epsilon) {
                cluster_index.push_back(index);
            }
            ++index;
        }
        return cluster_index;
    }

    double DBSCAN::CalculateDistance(const Point& point_lhs, const Point& point_rhs) {
        const std::string& tweet_lhs_id = point_lhs.tweet_id;
        const std::string& tweet_rhs_id = point_rhs.tweet_id;
        int index_lhs = tweet_position_map[tweet_lhs_id];
        int index_rhs = tweet_position_map[tweet_rhs_id];

        double distance = dist_map(index_lhs, index_rhs);
        return distance;
    }
}