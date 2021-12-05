//
// Created by dietrich on 15.11.21.
//

#ifndef GEOBURST_OSM_SIMILARITY_HANDLER_H
#define GEOBURST_OSM_SIMILARITY_HANDLER_H

#include <vector>
#include <string>
#include <iterator>
#include <cmath>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include "../pre_processing/Tweet.h"
#include "../sliding_window/sliding_window.h"
#include "../common/config_handler/config_handler.h"
#include "../common/geo_space/geo_space.h"
#include "co_occurrence_graph.h"
#include "random_walk_with_restart.h"

using common::config_handler::ConfigFileHandler;
using common::geo_space::Point;
using common::geo_space::Space;
using PreProcessing::TweetParser::Tweet;
using EventTweet::Co_Occurrence_Graph::KeyWordGraph;
using EventTweet::SlidingWindow::SnapShot;
using EventTweet::RWR::RandomWalkWithRestart;
using Eigen::SparseVector;
using Eigen::ColMajor;

namespace EventTweet::TweetSimilarity {

    using WordIndexMap = std::unordered_multimap<std::string, int>;
    using IndexWordMap = std::unordered_map<int, std::string>;
    using TweetPair = std::pair<std::string, double>;
    using TweetNeighbourMap = std::unordered_map<std::string, WordIndexMap>;
    using TextualSimilarityScoreList = SparseVector<double, ColMajor>;
    using TweetDistanceMap = std::vector<std::vector<std::pair<std::string, double>>>; // pair: (tweet_id, sim_score)


    class SpatialSimilarityHandler {
    public:
        double constant = 1.;

        double kernel_bandwidth = 1.;

    public:
        SpatialSimilarityHandler() = default;
    };

    class TextualSimilarityHandler {
    private:
        KeyWordGraph co_occurrence_graph;

        std::unordered_map<std::string, bool> vertex_flag;

        std::unordered_map<std::string, TextualSimilarityScoreList> word_scores_map;

    public:
        explicit TextualSimilarityHandler(SnapShot& snapshot);

        ~TextualSimilarityHandler();

        TextualSimilarityHandler& Init();

        void Reset();

        void InitVertexFlag();

        KeyWordGraph& GetCoOccurrenceGraph();

        std::unordered_map<std::string, bool>& GetVertexFlag();

        std::unordered_map<std::string, TextualSimilarityScoreList>& GetWordScoreMap();

    public:
        double restart_probability = 0.;

        int iterations = 0;
    };


    class TweetSimilarityHandler {
    private:
        TextualSimilarityHandler textual_similarity_handler;

        SpatialSimilarityHandler spatial_similarity_handler;

        SnapShot snapshot;

        TweetDistanceMap tweet_dist_map;

        TweetNeighbourMap tweet_neighbour_map;

    public:
        TweetSimilarityHandler(SnapShot &snapshot, ConfigFileHandler& config_file_handler);

        ~TweetSimilarityHandler();

        TweetSimilarityHandler& Init();

        WordIndexMap GenerateValidWordBag(Tweet& tweet);

        IndexWordMap GenerateInvertedWordBag(Tweet& tweet);

        TweetDistanceMap& GenerateTextualSimMap();

        double TextualImpact(WordIndexMap& word_index_map,
                             std::unordered_map<std::string, bool>& vertex_flag,
                             std::unordered_map<std::string, TextualSimilarityScoreList>& word_scores_map,
                             IndexWordMap& inverted_word_bag);

        double TextualImpactProcess(Tweet& tweet_lhs, Tweet& tweet_rhs);

        double GeographicalImpactProcess(Tweet& tweet_lhs, Tweet& tweet_rhs);

    public:
        friend class TweetLocationPredictor;
    };


    class TweetLocationPredictor {
    private:
        double alpha = 1.;

        std::size_t top_k = 20;

        std::vector<double> space_bounding_box;

        Space space; // grid cell size 1km

    public:
        TweetLocationPredictor() = default;

        explicit TweetLocationPredictor(ConfigFileHandler &config_file_handler)
                : top_k(config_file_handler.GetValue("top_k", 0)),
                  alpha(config_file_handler.GetValue("weight_combination", 1.0)),
                  space_bounding_box(config_file_handler.GetVector("space_Houston")), space(space_bounding_box, 1.0) {
        }

        std::vector<std::pair<std::string, double>> TopKRetrieval(std::vector<TweetPair> &element_similarities,
                                         std::unordered_map<std::string, Tweet>& tweet_map,
                                         std::unordered_map<std::string, Tweet>& tweet_need_predict) const;

        void WeightedMajorityVoting(std::vector<std::string> &top_k_tweets,
                                    std::vector<std::pair<std::string, int>> &tweet_cell_list,
                                    std::unordered_map<int, double> &cell_indices,
                                    std::unordered_map<std::string, Tweet>& tweet_map);

        void LocationPredict(TweetSimilarityHandler& tweet_similarity_handler);

    };
}

#endif //GEOBURST_OSM_SIMILARITY_HANDLER_H
