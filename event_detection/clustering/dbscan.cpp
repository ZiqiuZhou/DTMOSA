//
// Created by dietrich on 16.12.21.
//
#include "dbscan.h"

namespace EventTweet::Clustering {

    DBSCAN::DBSCAN(SnapShot &_snapshot, TweetSimilarity::TweetSimilarityHandler &tweet_similarity_handler,
                   ConfigFileHandler &config_file_handler): snapshot(_snapshot) {
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
        int cluster_id = 1;
        if (points.empty() || points.size() == 1) {
            return FAILURE;
        }
        for (auto& point : points) {
            if (point.cluster_id == UNCLASSIFIED) {
                if (ExpandCluster(point, cluster_id) != FAILURE) {
                    cluster_id += 1;
                }
            }
        }
        return SUCCESS;
    }

    int DBSCAN::ExpandCluster(Point& point, int cluster_id) {
        std::vector<int> cluster_seeds = CalculateCluster(point);
        if (cluster_seeds.empty() || cluster_seeds.size() <= minimum_points) {
            point.cluster_id = NOISE;
            return FAILURE;
        } else {
            int index = 0;
            int index_core_point = 0;
            for (int pos : cluster_seeds) {
                if (pos >= points.size()) {
                    return FAILURE;
                }
                points[pos].cluster_id = cluster_id;
                if (points[pos].latitude == point.latitude && points[pos].longitude == point.longitude) {
                    index_core_point = index;
                }
                ++index;
            }
            cluster_seeds.erase(cluster_seeds.begin() + index_core_point);

            for (std::size_t i = 0; i < cluster_seeds.size(); ++i) {
                int pos = cluster_seeds[i];
                std::vector<int> cluster_neighbors = CalculateCluster(points[pos]);
                if (cluster_neighbors.size() >= minimum_points) {
                    for (int neighbor_pos : cluster_neighbors) {
                        if (points[neighbor_pos].cluster_id == UNCLASSIFIED || points[neighbor_pos].cluster_id == NOISE) {
                            if (points[neighbor_pos].cluster_id == UNCLASSIFIED) {
                                cluster_seeds.push_back(neighbor_pos);
                            }
                            points[neighbor_pos].cluster_id = cluster_id;
                        }
                    }
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

        if (index_lhs >= dist_map.cols() || index_rhs >= dist_map.cols()) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " tweet index out of bound." << std::endl;
            return 1.;
        }
        double distance = dist_map(index_lhs, index_rhs);
        return distance;
    }
}