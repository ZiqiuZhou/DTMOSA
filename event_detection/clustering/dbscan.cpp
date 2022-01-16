//
// Created by dietrich on 16.12.21.
//
#include "dbscan.h"

namespace EventTweet::Clustering {
    DBSCAN::DBSCAN(SnapShot &_snapshot,
                   TweetSimilarity::TweetSimilarityHandler &tweet_similarity_handler,
                   ConfigFileHandler &config_file_handler)
                   : BaseClustering(_snapshot, tweet_similarity_handler,config_file_handler) {
    }

    DBSCAN::~DBSCAN() {
    }

    int DBSCAN::Cluster() {
        if (points.empty() || points.size() == 1) {
            return FAILURE;
        }
        for (auto& point : points) {
            if (point.cluster_id == UNDEFINED) {
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
                if (seed_point.cluster_id != UNDEFINED) {
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
}