//
// Created by dietrich on 15.11.21.
//

#include "similarity_handler.h"

namespace EventTweet::TweetSimilarity {

    TextualSimilarityHandler::TextualSimilarityHandler(SnapShot &snapshot)
    : co_occurrence_graph(snapshot){
        restart_probability = 0.;
        iterations = 0;

        vertex_flag.clear();
        word_scores_map.clear();
    }

    TextualSimilarityHandler::~TextualSimilarityHandler() {
        Reset();
    }

    TextualSimilarityHandler& TextualSimilarityHandler::Init() {
        co_occurrence_graph.Init()
                .GenerateVertexList()
                .GenerateVertexIndexMap()
                .GenerateAdjacentMatrix();

        InitVertexFlag();
        return *this;
    }

    void TextualSimilarityHandler::Reset() {
        co_occurrence_graph.Reset();
        vertex_flag.clear();
        word_scores_map.clear();
        return ;
    }

    KeyWordGraph& TextualSimilarityHandler::GetCoOccurrenceGraph() {
        return this->co_occurrence_graph;
    }

    void TextualSimilarityHandler::InitVertexFlag() {
        auto& vertex_list = co_occurrence_graph.GetVertexList();
        for (auto& vertex: vertex_list) {
            std::string word = vertex.GetVertex();
            vertex_flag[word] = false;
        }
        return ;
    }

    std::unordered_map<std::string, bool>& TextualSimilarityHandler::GetVertexFlag() {
        return this->vertex_flag;
    }

    std::unordered_map<std::string, TextualSimilarityScoreList>& TextualSimilarityHandler::GetWordScoreMap() {
        return this->word_scores_map;
    }

    TweetSimilarityHandler::TweetSimilarityHandler(SnapShot &_snapshot, ConfigFileHandler &config_file_handler)
            : textual_similarity_handler(_snapshot) {
        snapshot = _snapshot;
        tweet_dist_map.clear();
        tweet_neighbour_map.clear();
        textual_similarity_handler.restart_probability = config_file_handler.GetValue("restart_probability", 0.);
        textual_similarity_handler.iterations = config_file_handler.GetValue("iterations", 0);
        spatial_similarity_handler.kernel_bandwidth = config_file_handler.GetValue("kernel_bandwidth", 1.0);
    }

    TweetSimilarityHandler::~TweetSimilarityHandler() {
        textual_similarity_handler.Reset();
        snapshot.Reset();
        tweet_dist_map.clear();
        tweet_neighbour_map.clear();
    }

    TweetSimilarityHandler& TweetSimilarityHandler::Init() {
        textual_similarity_handler.Init();

        auto& tweet_map = snapshot.GetTweetMap();
        tweet_dist_map.resize(tweet_map.size(), std::vector<TweetPair>(tweet_map.size()));
        return *this;
    }

    WordIndexMap TweetSimilarityHandler::GenerateValidWordBag(Tweet& tweet) {
        auto& candidate_set = snapshot.GetBurstyWords();
        auto& vertex_index_map = textual_similarity_handler.GetCoOccurrenceGraph().GetVertexIndexMap();
        if (candidate_set.empty() || vertex_index_map.empty() || (candidate_set.size() != vertex_index_map.size())) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " generate wordbag failed." << std::endl;
            return { };
        }
        WordIndexMap valid_map = {};
        std::unordered_multiset<std::string>& word_bag = tweet.GetWordBag();
        for (auto& word: word_bag) {
            if (candidate_set.find(word) != candidate_set.end()) {
                valid_map.insert(std::make_pair(word, vertex_index_map[word]));
            }
        }
        return valid_map;
    }

    IndexWordMap TweetSimilarityHandler::GenerateInvertedWordBag(Tweet& tweet) {
        WordIndexMap word_index_map = GenerateValidWordBag(tweet);
        if (word_index_map.empty()) {
            return { };
        }

        IndexWordMap index_word_map = {};
        for (auto& word_index_pair: word_index_map) {
            std::string word = word_index_pair.first;
            int index = word_index_pair.second;
            index_word_map[index] = word;
        }
        return index_word_map;
    }

    double TweetSimilarityHandler::TextualImpact(WordIndexMap& word_index_map,
                         std::unordered_map<std::string, bool>& vertex_flag,
                         std::unordered_map<std::string, TextualSimilarityScoreList>& word_scores_map,
                         IndexWordMap& inverted_word_bag) {
        double scores = 0.;
        for (auto& vertex : word_index_map) {
            std::string word = vertex.first;
            int vertex_index = vertex.second;
            if (vertex_flag.find(word) != vertex_flag.end()) {
                continue;
            }
            if (!vertex_flag[word]) {
                auto& vertex_list = textual_similarity_handler.GetCoOccurrenceGraph().GetVertexList();
                auto& adjacent_matrix = textual_similarity_handler.GetCoOccurrenceGraph().GetAdjacentMatrix();

                RandomWalkWithRestart random_walk(textual_similarity_handler.restart_probability,
                                                  textual_similarity_handler.iterations,
                                                  vertex_list.size(),
                                                  vertex_index,
                                                  adjacent_matrix);
                random_walk.InitScore();
                // random walk with restart iterations
                SparseVector<double, ColMajor>& rwr_score_list = random_walk.Iterate();
                word_scores_map[word] = rwr_score_list;
                vertex_flag[word] = true;
            }
            SparseVector<double, ColMajor>& rwr_score_list = word_scores_map[word];
            for (SparseVector<double, ColMajor>::InnerIterator iter(rwr_score_list); iter; ++iter) {
                int index = iter.index();
                if (inverted_word_bag.find(index) != inverted_word_bag.end()) {
                    double score = iter.value();
                    scores += score;
                }
            }
        }
        return scores;
    }

    double TweetSimilarityHandler::TextualImpactProcess(Tweet& tweet_lhs, Tweet& tweet_rhs) {
        double similarity_score = 0.;

        WordIndexMap word_index_map_lhs = GenerateValidWordBag(tweet_lhs);
        WordIndexMap word_index_map_rhs = GenerateValidWordBag(tweet_rhs);
        IndexWordMap inverted_word_bag_lhs = GenerateInvertedWordBag(tweet_lhs);
        IndexWordMap inverted_word_bag_rhs = GenerateInvertedWordBag(tweet_rhs);
        if (word_index_map_lhs.empty() || word_index_map_rhs.empty()) {
            return similarity_score;
        }

        std::unordered_map<std::string, TextualSimilarityScoreList>& word_scores_map = textual_similarity_handler.GetWordScoreMap();

        int count = 0;
        for (auto& word_index_pair: word_index_map_lhs) {
            if (word_index_map_rhs.find(word_index_pair.first) != word_index_map_rhs.end()) {
                ++count;
                break;
            }
        }
        if (count == 0) {
            return similarity_score;
        }

        std::unordered_map<std::string, bool>& vertex_flag = textual_similarity_handler.GetVertexFlag();
        similarity_score = 0.;
        double score_for_tweet_left = TextualImpact(word_index_map_lhs, vertex_flag, word_scores_map,
                                                    inverted_word_bag_rhs);
        double score_for_tweet_right = TextualImpact(word_index_map_rhs, vertex_flag, word_scores_map,
                                                     inverted_word_bag_lhs);
        similarity_score = (score_for_tweet_left * score_for_tweet_right) /
                           (word_index_map_lhs.size() * word_index_map_rhs.size());
        return similarity_score;
    }

    double TweetSimilarityHandler::GeographicalImpactProcess(Tweet& tweet_lhs, Tweet& tweet_rhs) {
        Space space;
        Point point1(tweet_lhs.GetLongitude(), tweet_lhs.GetLatitude());
        Point point2(tweet_rhs.GetLongitude(), tweet_rhs.GetLatitude());
        double distance = space.Distance(point1, point2);

        double similarity_score = 0.;
        double bandwidth = spatial_similarity_handler.kernel_bandwidth;
        if (distance > bandwidth) {
            return similarity_score;
        } else {
            similarity_score = spatial_similarity_handler.constant * (1 - std::pow(distance, 2) / std::pow(bandwidth, 2));
        }
        return similarity_score;
    }

    TweetDistanceMap& TweetSimilarityHandler::GenerateTextualSimMap() {
        if (tweet_dist_map.empty()) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " Invalid tweet_dist_map." << std::endl;
            return tweet_dist_map;
        }
        auto& tweet_map = snapshot.GetTweetMap();
        for (auto iter_i = tweet_map.begin(); iter_i != tweet_map.end(); ++iter_i) {
            std::size_t index_i = std::distance(tweet_map.begin(), iter_i);
            std::string tweet_id = (*iter_i).first;
            tweet_dist_map[index_i][index_i] = std::make_pair(tweet_id, 0.);

            auto iter_j = tweet_map.begin();
            std::advance(iter_j, index_i + 1);
            for (; iter_j != tweet_map.end(); ++iter_j) {
                std::string tweet_id_i = (*iter_i).first;
                Tweet tweet_lhs = (*iter_i).second;
                std::string tweet_id_j = (*iter_j).first;
                Tweet tweet_rhs = (*iter_j).second;
                double textual_score = TextualImpactProcess(tweet_lhs, tweet_rhs);
                std::size_t index_j = std::distance(tweet_map.begin(), iter_j);
                tweet_dist_map[index_i][index_j] = std::make_pair(tweet_id_j, textual_score);
                tweet_dist_map[index_j][index_i] = std::make_pair(tweet_id_i, textual_score);
            }
        }
        return tweet_dist_map;
    }

    std::vector<std::pair<std::string, double>> TweetLocationPredictor::TopKRetrieval(std::vector<TweetPair> &element_similarities,
                                                             std::unordered_map<std::string, Tweet> &tweet_map,
                                                             std::unordered_map<std::string, Tweet> &tweet_need_predict) const {
        struct CmpByValue {
            bool operator()(const TweetPair& lhs, const TweetPair& rhs) {
                return lhs.second > rhs.second;
            }
        };
        using HeapType = std::priority_queue<TweetPair, std::vector<TweetPair>, CmpByValue>;
        HeapType min_heap;

        auto iterator = tweet_map.begin();
        for (std::size_t idx = 0; idx < element_similarities.size(); ++idx) {
            iterator = tweet_map.begin();
            std::advance(iterator, idx);
            std::string tweet_id = (*iterator).first;
            if (tweet_need_predict.find(tweet_id) != tweet_need_predict.end()) {
                continue;
            }
            min_heap.push(std::make_pair(tweet_id, element_similarities[idx].second));
            if (min_heap.size() > top_k) {
                min_heap.pop();
            }
        }

        std::vector<std::pair<std::string, double>> top_k_tweets; // (tweet_id, sim_score)
        while (!min_heap.empty()) {
            std::string tweet_id = min_heap.top().first;
            double sim_score = min_heap.top().second;
            min_heap.pop();
            top_k_tweets.emplace_back(std::make_pair(tweet_id, sim_score));
        }
        return top_k_tweets;
    }

    void TweetLocationPredictor::WeightedMajorityVoting(std::vector<std::string> &top_k_tweets,
                                std::vector<std::pair<std::string, int>> &tweet_cell_list,
                                std::unordered_map<int, double> &cell_indices_score,
                                std::unordered_map<std::string, Tweet>& tweet_map) {
        if (tweet_cell_list.size() != top_k_tweets.size()) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " Invalid tweet_cell_list size." << std::endl;
            return ;
        }
        // assign grid index for each top-K element
        for (std::size_t i = 0; i < top_k_tweets.size(); ++i) {
            std::string &tweet_id = top_k_tweets[i];
            Tweet &geo_tagged_tweet = tweet_map[tweet_id];
            double longitude = geo_tagged_tweet.GetLongitude();
            double latitude = geo_tagged_tweet.GetLatitude();
            int cell_index = space.GetCellIndex(longitude, latitude);
            tweet_cell_list[i] = std::make_pair(tweet_id, cell_index);
            double vote = 1.0; // vote for this cell_index

        }
    }

    void TweetLocationPredictor::LocationPredict(TweetSimilarityHandler& tweet_similarity_handler) {
        SnapShot& snapshot = tweet_similarity_handler.snapshot;
        auto& tweet_map = snapshot.GetTweetMap();
        auto& tweets_need_predict = snapshot.GetNeedPredictTweetMap();
        if (tweets_need_predict.empty()) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " Invalid tweet_need_predict." << std::endl;
            return ;
        }

        for (auto iter = tweets_need_predict.begin(); iter != tweets_need_predict.end(); ++iter) {
            Tweet& tweet_predict = (*iter).second;
            auto iterator = tweet_map.find(tweet_predict.GetTweetID());
            if (iterator == tweet_map.end()) {
                continue;
            }
            std::size_t index = std::distance(tweet_map.begin(), iterator);
            // get all tweets' textual similarities towards the candidate one
            std::vector<TweetPair>& element_similarities = tweet_similarity_handler.tweet_dist_map[index];
            std::vector<std::pair<std::string, double>> top_k_tweets = TopKRetrieval(element_similarities, tweet_map, tweets_need_predict);
            if (top_k_tweets.empty()) {
                continue;
            }
            std::vector<std::pair<std::string, int>> tweet_cell_list(top_k_tweets.size()); // (tweet_id, cell_index)
            std::unordered_map<int, double> cell_indices_score = {}; // key: grid_index, value: weighted vote score

        }
    }
}