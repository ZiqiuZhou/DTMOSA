//
// Created by dietrich on 16.01.22.
//

#include "optics.h"

namespace EventTweet::Clustering {

    OPTICS::OPTICS(SnapShot &_snapshot,
                   TweetSimilarity::TweetSimilarityHandler &tweet_similarity_handler,
                   ConfigFileHandler &config_file_handler)
            : BaseClustering(_snapshot, tweet_similarity_handler,config_file_handler) {
        order_list = {};
    }

    OPTICS::~OPTICS() {
        order_list.clear();
    }

    void OPTICS::GetCorePoints() {
        std::vector<Point>& points = GetPoints();
        for (auto& point : points) {
            std::vector<int> neighbors_indices = CalculateCluster(point);
            point.neighbor_num = neighbors_indices.size();
            if (!neighbors_indices.empty() && neighbors_indices.size() >= minimum_points) {
                std::priority_queue<double, std::vector<double>, std::greater<double>> min_heap_dists;
                for (int index : neighbors_indices) {
                    double dist = CalculateDistance(point, points[index]);
                    min_heap_dists.push(dist);
                }

                int count = 0;
                while (!min_heap_dists.empty() && count < minimum_points - 1) {
                    ++count;
                    min_heap_dists.pop();
                }
                point.core_dist = min_heap_dists.top();
            }
        }
        return ;
    }

    int OPTICS::Cluster() {
        GetCorePoints();

        for (auto& point : points) {
            if (point.processed == UNPROCESSED) {
                point.processed = PROCESSED;
                order_list.emplace_back(point); // set point to output list
                if (point.neighbor_num >= minimum_points) {
                    // priority_queue to store seeds sorted by their reachability_distance
                    std::priority_queue<Point, std::vector<Point>, Cmp> seeds;
                    Update(point, seeds);

                    while (!seeds.empty()) {
                        Point seed = seeds.top();
                        seeds.pop();
                        seed.processed = PROCESSED;
                        order_list.emplace_back(seed);
                        if (seed.neighbor_num >= minimum_points) {
                            Update(seed, seeds);
                        }
                    }
                }
            }
        }
        return SUCCESS;
    }

    void OPTICS::Update(Point& point, std::priority_queue<Point, std::vector<Point>, Cmp>& seeds) {
        std::vector<int> neighbors = CalculateCluster(point);
        for (int neighbor_index : neighbors) {
            Point& neighbor_point = points[neighbor_index];
            if (neighbor_point.processed == UNPROCESSED) {
                double new_reachability_dist = std::max(point.core_dist, CalculateDistance(point, neighbor_point));
                if (neighbor_point.reachability_dist == UNREACHABLE) { // point not in seeds
                    neighbor_point.reachability_dist = new_reachability_dist;
                    seeds.push(neighbor_point);
                } else { // point in seeds, check for improvement
                    if (new_reachability_dist < neighbor_point.reachability_dist) {
                        neighbor_point.reachability_dist = new_reachability_dist;
                        MoveUpElement(neighbor_point, seeds);
                    }
                }
            }
        }
        return ;
    }

    void OPTICS::MoveUpElement(Point& point, std::priority_queue<Point, std::vector<Point>, Cmp>& seeds) {
        std::vector<Point> temp_point_list;
        while (!seeds.empty()) {
            const Point& top_point = seeds.top();
            if (top_point.tweet_id == point.tweet_id) {
                seeds.pop();
                seeds.push(point);
                break;
            } else {
                temp_point_list.emplace_back(top_point);
                seeds.pop();
            }
        }
        if (!temp_point_list.empty()) {
            for (auto& temp_point : temp_point_list) {
                seeds.push(temp_point);
            }
        }
        return ;
    }


}
