//
// Created by dietrich on 16.12.21.
//
#include "dbscan.h"

namespace EventTweet::DBSCAN {

    DBSCAN::DBSCAN(TweetSimilarity::TweetSimilarityHandler& tweet_similarity_handler) {
        auto& spatial_map = tweet_similarity_handler.GetSpatialDistMap();
        auto& textual_map = tweet_similarity_handler.GetTextualDistMap();
        dist_map = 0.5 * spatial_map;
        dist_map += (0.5 * textual_map);
    }
}