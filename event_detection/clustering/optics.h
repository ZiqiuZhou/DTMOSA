//
// Created by dietrich on 16.01.22.
//

#ifndef GEOBURST_OSM_OPTICS_H
#define GEOBURST_OSM_OPTICS_H

#include <vector>
#include <queue>
#include <algorithm>
#include <functional>

#include "clustering.h"
#include "tweet_similarity/similarity_handler.h"
#include "sliding_window/sliding_window.h"
#include "../pre_processing/Tweet.h"
#include "../common/config_handler/config_handler.h"

namespace EventTweet::Clustering {

    struct Cmp {
        bool operator()(Point& lhs, Point& rhs) {
            return lhs.reachability_dist > rhs.reachability_dist;
        }
    };

    class OPTICS : public BaseClustering {
    private:
        std::vector<Point> order_list; // store output points

    public:
        OPTICS(SnapShot &_snapshot, TweetSimilarity::TweetSimilarityHandler &tweet_similarity_handler,
               ConfigFileHandler &config_file_handler);

        ~OPTICS() override;

    public:
        int Cluster() override;

        void GetCorePoints();

        void Update(Point& point, std::priority_queue<Point, std::vector<Point>, Cmp>& seeds);

        void MoveUpElement(Point& point, std::priority_queue<Point, std::vector<Point>, Cmp>& seeds);
    };

}

#endif //GEOBURST_OSM_OPTICS_H
