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
        tweet_textual_dist_map.setZero();
        tweet_spatial_dist_map.setZero();
        blank_position_list = {};
        blank_position_id = {};
        tweet_position_map = {};
        textual_similarity_handler.restart_probability = config_file_handler.GetValue("restart_probability", 0.);
        textual_similarity_handler.iterations = config_file_handler.GetValue("iterations", 0);
        spatial_similarity_handler.kernel_bandwidth = config_file_handler.GetValue("kernel_bandwidth", 1.0);
    }

    TweetSimilarityHandler::~TweetSimilarityHandler() {
        textual_similarity_handler.Reset();
        snapshot.Reset();
        tweet_textual_dist_map.setZero();
        tweet_spatial_dist_map.setZero();
        blank_position_list.clear();
        blank_position_id.clear();
        tweet_position_map.clear();
    }

    TweetSimilarityHandler& TweetSimilarityHandler::Init() {
        textual_similarity_handler.Init();

        auto& tweet_map = snapshot.GetTweetMap();
        long SIZE = static_cast<long>(tweet_map.size());
        tweet_textual_dist_map.resize(SIZE, SIZE);
        tweet_spatial_dist_map.resize(SIZE, SIZE);
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
            if (vertex_flag.find(word) == vertex_flag.end()) {
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
        if (word_index_map_lhs.empty() || word_index_map_rhs.empty()) {
            return similarity_score;
        }
        IndexWordMap inverted_word_bag_lhs = GenerateInvertedWordBag(tweet_lhs);
        IndexWordMap inverted_word_bag_rhs = GenerateInvertedWordBag(tweet_rhs);

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

        std::unordered_map<std::string, TextualSimilarityScoreList>& word_scores_map = textual_similarity_handler.GetWordScoreMap();
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

    double TweetSimilarityHandler::GeographicalImpactProcess(Tweet& tweet_lhs, Tweet& tweet_rhs) const {
        double similarity_score = 0.;
        if (tweet_lhs.NeedPredictLocation() || tweet_rhs.NeedPredictLocation()) {
            return -1.0;
        }
        Space space;
        Point point1(tweet_lhs.GetLongitude(), tweet_lhs.GetLatitude());
        Point point2(tweet_rhs.GetLongitude(), tweet_rhs.GetLatitude());
        double distance = space.Distance(point1, point2);

        double bandwidth = spatial_similarity_handler.kernel_bandwidth;
        if (distance < bandwidth) {
            similarity_score = spatial_similarity_handler.constant * (1 - std::pow(distance, 2) / std::pow(bandwidth, 2));
        }
        return 1.0 - similarity_score;
    }

    TweetSimilarityHandler& TweetSimilarityHandler::GenerateSimMap() {
        if (tweet_textual_dist_map.isVector()) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " Invalid tweet_dist_map." << std::endl;
            return *this;
        }
        auto& tweet_map = snapshot.GetTweetMap();
        std::vector<Eigen::Triplet<double>> tripleList;
        for (auto iter_i = tweet_map.begin(); iter_i != tweet_map.end(); ++iter_i) {
            std::size_t index_i = std::distance(tweet_map.begin(), iter_i);
            std::string tweet_id = (*iter_i).first;
            tweet_position_map[tweet_id] = index_i;
            tweet_spatial_dist_map(index_i, index_i) = 0.;

            auto iter_j = tweet_map.begin();
            std::advance(iter_j, index_i + 1);
            for (; iter_j != tweet_map.end(); ++iter_j) {
                std::size_t index_j = std::distance(tweet_map.begin(), iter_j);
                std::string tweet_id_i = (*iter_i).first;
                Tweet tweet_lhs = (*iter_i).second;
                std::string tweet_id_j = (*iter_j).first;
                Tweet tweet_rhs = (*iter_j).second;
                double textual_score = TextualImpactProcess(tweet_lhs, tweet_rhs);

                double spatial_score = GeographicalImpactProcess(tweet_lhs, tweet_rhs);
                tweet_spatial_dist_map(index_i, index_j) = spatial_score;
                tweet_spatial_dist_map(index_j, index_i) = spatial_score;
                if (spatial_score < 0.) {
                    auto pos_i = blank_position_id.find(tweet_id_i);
                    auto pos_j = blank_position_id.find(tweet_id_j);
                    if (pos_i == blank_position_id.end() || pos_j == blank_position_id.end()) {
                        // store blank spatial distance for such tweets absent with long / lat
                        auto pair1 = std::make_pair(tweet_id_i, index_i);
                        auto pair2 = std::make_pair(tweet_id_j, index_j);
                        blank_position_list.emplace_back(std::make_pair(pair1, pair2));

                        blank_position_id.insert(tweet_id_i);
                        blank_position_id.insert(tweet_id_j);
                    }
                }

                if (textual_score > 1e-5) {
                    tripleList.emplace_back(index_i, index_j, textual_score);
                    tripleList.emplace_back(index_j, index_i, textual_score);
                }
            }
        }
        tweet_textual_dist_map.setFromTriplets(tripleList.begin(), tripleList.end());
        return *this;
    }

    TweetTextualDistMap& TweetSimilarityHandler::GetTextualDistMap() {
        return this->tweet_textual_dist_map;
    }

    TweetSpatialDistMap& TweetSimilarityHandler::GetSpatialDistMap() {
        return this->tweet_spatial_dist_map;
    }

    std::unordered_map<std::string, int>& TweetSimilarityHandler::GetTweetPositionMap() {
        return this->tweet_position_map;
    }


    double TweetLocationPredictor::validation_ratio = 0.1;

    struct CmpTweetPair {
        bool operator()(const TweetPair& lhs, const TweetPair& rhs) {
            return lhs.second > rhs.second;
        }
    };

    std::vector<TweetPair> TweetLocationPredictor::TopKRetrieval(SparseVector<double, ColMajor> &element_similarities,
                                                             std::unordered_map<std::string, Tweet> &tweet_map,
                                                             std::unordered_map<std::string, Tweet> &tweet_need_predict,
                                                             std::unordered_map<std::string, Tweet>& valid_tweet_map) const {
        using HeapType = std::priority_queue<TweetPair, std::vector<TweetPair>, CmpTweetPair>;
        HeapType min_heap;

        auto iterator = tweet_map.begin();
        for (Eigen::SparseVector<double, ColMajor>::InnerIterator it(element_similarities); it; ++it) {
            iterator = tweet_map.begin();
            std::size_t idx = it.index();
            std::advance(iterator, idx);
            std::string tweet_id = (*iterator).first;
            if (tweet_need_predict.find(tweet_id) != tweet_need_predict.end()) {
                continue;
            }
            if (valid_tweet_map.find(tweet_id) != valid_tweet_map.end()) {
                min_heap.push(std::make_pair(tweet_id, it.value()));
                if (min_heap.size() > top_k) {
                    min_heap.pop();
                }
            }
        }

        std::vector<TweetPair> top_k_tweets; // (tweet_id, textual_sim_score)
        while (!min_heap.empty()) {
            std::string tweet_id = min_heap.top().first;
            double sim_score = min_heap.top().second;
            min_heap.pop();
            top_k_tweets.emplace_back(std::make_pair(tweet_id, sim_score));
        }
        return top_k_tweets;
    }

    TweetLocationPredictor &
    TweetLocationPredictor::GenerateTrainValidationTweets(std::unordered_map<std::string, Tweet> &tweet_map) {
        std::size_t SIZE = tweet_map.size();
        int validation_size = static_cast<int>(SIZE * validation_ratio);
        int train_size = SIZE - validation_size;
        std::vector<int> random_sequence(SIZE);
        std::iota(random_sequence.begin(), random_sequence.end(), 0);

        //random generator
        std::shuffle(random_sequence.begin(), random_sequence.end(), std::mt19937{std::random_device{}()});
        std::unordered_set<int> validation_indices;
        for (int i = 0; i < validation_size; ++i) {
            validation_indices.insert(random_sequence[i]);
        }
        for (auto iter = tweet_map.begin(); iter != tweet_map.end(); ++iter) {
            int index = std::distance(tweet_map.begin(), iter);
            std::string tweet_id = (*iter).first;
            Tweet tweet = (*iter).second;
            if (validation_indices.find(index) != validation_indices.end() && !tweet.NeedPredictLocation()) {
                tweet_validation_map[tweet_id] = tweet;
            } else {
                tweet_train_map[tweet_id] = tweet;
            }
        }
        return *this;
    }

    TweetLocationPredictor &
    TweetLocationPredictor::FindTextualSimilarTweetsForValidationMap(TweetSimilarityHandler &tweet_similarity_handler) {
        SnapShot& snapshot = tweet_similarity_handler.snapshot;
        auto& tweet_map = snapshot.GetTweetMap();
        auto& tweets_need_predict = snapshot.GetNeedPredictTweetMap();

        SparseVector<double, ColMajor> element_similarities;
        element_similarities.resize(tweet_map.size());
        for (auto& tweet_info: tweet_validation_map) {
            element_similarities.setZero();
            const std::string& validation_tweet_id = tweet_info.first;
            Tweet& validation_tweet = tweet_info.second;

            std::unordered_set<std::string> similar_tweet_ids = {};
            const auto& iterator = tweet_map.find(validation_tweet_id);
            if (iterator == tweet_map.end()) {
                continue;
            }
            std::size_t index = std::distance(tweet_map.begin(), iterator);
            element_similarities = tweet_similarity_handler.tweet_textual_dist_map.col(index);
            std::vector<TweetPair> top_k_tweets = TopKRetrieval(element_similarities, tweet_map, tweets_need_predict,
                                                                tweet_map);
            if (top_k_tweets.empty()) {
                continue;
            }
            for (TweetPair& tweet_pair: top_k_tweets) {
                std::string tweet_id = tweet_pair.first;
                similar_tweet_ids.insert(std::move(tweet_id));
            }
            similar_tweets_for_validation.emplace_back(std::make_pair(validation_tweet_id, similar_tweet_ids));
            top_k_tweets.clear();
            similar_tweet_ids.clear();
        }
        return *this;
    }

    int TweetLocationPredictor::WeightedMajorityVoting(std::vector<TweetPair> &top_k_tweets,
                                std::unordered_map<int, double> &cell_indices_score,
                                std::unordered_map<std::string, Tweet>& tweet_map,
                                SnapShot& snapshot) {
        UserTweetMap& user_tweet_map = snapshot.GetUserTweetMap();
        for (std::size_t i = 0; i < top_k_tweets.size(); ++i) {
            std::string &geo_tagged_tweet_id = top_k_tweets[i].first;
            double textual_sim_score = top_k_tweets[i].second;
            Tweet &geo_tagged_tweet = tweet_map[geo_tagged_tweet_id];
            double longitude = geo_tagged_tweet.GetLongitude();
            double latitude = geo_tagged_tweet.GetLatitude();
            int cell_index = space.GetCellIndex(longitude, latitude); // get grid index for each top-K element
            double vote = 1.0; // vote for this cell_index

            // compute credibility of geo_tagged_tweet's user
            double credibility = 0.;
            int count = 0;
            int valid_count = 0;
            const std::string& user_id = geo_tagged_tweet.GetUserID();
            if (user_tweet_map.find(user_id) != user_tweet_map.end()) {
                std::unordered_set<std::string>& tweet_set = user_tweet_map[user_id];
                for (auto& tweet_id: tweet_set) {
                    Tweet& tweet1 = tweet_map[tweet_id];
                    if (tweet1.NeedPredictLocation()) {
                        continue;
                    }
                    for (auto& validation_tweet_sim_set: similar_tweets_for_validation) {
                        std::string& validation_tweet_id = validation_tweet_sim_set.first;
                        std::unordered_set<std::string>& sim_tweets = validation_tweet_sim_set.second;
                        if (sim_tweets.find(tweet_id) != sim_tweets.end()) {
                            count++;
                            Tweet& tweet2 = tweet_map[validation_tweet_id];
                            Point point1(tweet1.GetLongitude(), tweet1.GetLatitude());
                            Point point2(tweet2.GetLongitude(), tweet2.GetLatitude());
                            double distance = space.Distance(point1, point2);
                            if (distance <= 1.) {
                                valid_count++;
                            }
                        }
                    }
                }
            }
            if (count > 0) {
                credibility = (double)(valid_count / count);
            }
            double weight = credibility * alpha + textual_sim_score * (1.0 - alpha);
            cell_indices_score[cell_index] += vote * weight; // weighted vote score
        }

        auto pos = std::max_element(cell_indices_score.begin(), cell_indices_score.end(),
                         [](const std::pair<int, double> &lhs, const std::pair<int, double> &rhs) {
                             return lhs.second < rhs.second;
                         });
        return (*pos).first;
    }

    void TweetLocationPredictor::Predict(TweetSimilarityHandler& tweet_similarity_handler) {
        SnapShot& snapshot = tweet_similarity_handler.snapshot;
        auto& tweet_map = snapshot.GetTweetMap();
        auto& tweets_need_predict = snapshot.GetNeedPredictTweetMap();
        if (tweets_need_predict.empty()) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " No tweet needs location prediction." << std::endl;
            return ;
        }

        GenerateTrainValidationTweets(tweet_map); // split tweet_map into train / validation parts
        FindTextualSimilarTweetsForValidationMap(tweet_similarity_handler);

        SparseVector<double, ColMajor> element_similarities;
        for (auto iter = tweets_need_predict.begin(); iter != tweets_need_predict.end(); ++iter) {
            element_similarities.setZero();
            element_similarities.resize(tweet_map.size());
            Tweet& tweet_predict = (*iter).second;
            const auto& iterator = tweet_map.find(tweet_predict.GetTweetID());
            if (iterator == tweet_map.end()) {
                continue;
            }
            std::size_t index = std::distance(tweet_map.begin(), iterator);
            // get all tweets' textual similarities towards the candidate one
            element_similarities = tweet_similarity_handler.tweet_textual_dist_map.col(index);
            std::vector<TweetPair> top_k_tweets = TopKRetrieval(element_similarities, tweet_map, tweets_need_predict,
                                                                tweet_train_map);
            Tweet& tweet = (*iterator).second;
            if (top_k_tweets.empty()) {
                // assign coordinates to city center
                tweet.SetLongitude((space_bounding_box[0] + space_bounding_box[2]) / 2);
                tweet.SetLatitude((space_bounding_box[1] + space_bounding_box[3]) / 2);
                continue;
            }
            std::unordered_map<int, double> cell_indices_score = {}; // key: grid_index, value: weighted vote score
            // compute grid index for predict_tweet
            int grid_index = WeightedMajorityVoting(top_k_tweets, cell_indices_score, tweet_map, snapshot);
            Point point = space.GetCentralLocationOfCell(grid_index);
            tweet.SetLongitude(point.longitude);
            tweet.SetLatitude(point.latitude);
        }

        // fill out blank spatial distances
        auto& blank_position_list = tweet_similarity_handler.blank_position_list;
        for (auto& pos_pair: blank_position_list) {
            auto& tweet_id_index_i = pos_pair.first;
            auto& tweet_id_index_j = pos_pair.second;
            std::string& tweet_id_i = tweet_id_index_i.first;
            int index_i = tweet_id_index_i.second;
            std::string& tweet_id_j = tweet_id_index_j.first;
            int index_j = tweet_id_index_j.second;
            Tweet& tweet_i = tweet_map[tweet_id_i];
            Tweet& tweet_j = tweet_map[tweet_id_j];
            tweet_i.SetPredictFlag(false);
            tweet_j.SetPredictFlag(false);

            double spatial_score = tweet_similarity_handler.GeographicalImpactProcess(tweet_i, tweet_j);
            tweet_similarity_handler.tweet_spatial_dist_map(index_i, index_j) = spatial_score;
            tweet_similarity_handler.tweet_spatial_dist_map(index_j, index_i) = spatial_score;
        }
        return ;
    }
}