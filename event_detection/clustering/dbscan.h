//
// Created by dietrich on 16.12.21.
//

#ifndef GEOBURST_OSM_DBSCAN_H
#define GEOBURST_OSM_DBSCAN_H

#include <vector>
#include <string>
#include <unordered_set>

#include "clustering.h"
#include "tweet_similarity/similarity_handler.h"
#include "sliding_window/sliding_window.h"
#include "../pre_processing/Tweet.h"
#include "../common/config_handler/config_handler.h"

namespace EventTweet::Clustering {

    class DBSCAN : public BaseClustering {
    private:

    public:
        DBSCAN(SnapShot &_snapshot, TweetSimilarity::TweetSimilarityHandler &tweet_similarity_handler,
               ConfigFileHandler &config_file_handler);

        ~DBSCAN() override;

    public:
        int Cluster() override;

        int ExpandCluster(Point& point);
    };
}
#endif //GEOBURST_OSM_DBSCAN_H
