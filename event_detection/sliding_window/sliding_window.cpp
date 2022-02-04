#include "sliding_window.h"

namespace EventTweet::SlidingWindow {

	void SnapShot::GenerateWordTweetPair(Tweet& tweet) {
		auto word_bag = tweet.GetWordBag();
		WordTweetPair& word_tweet_map = GetWordTweetPair();

		if (!word_bag.empty()) {
			for (auto& word : word_bag) {
				if (word_tweet_map.empty() || (word_tweet_map.find(word) == word_tweet_map.end())) {
					word_tweet_map[word] = { };
				}
				word_tweet_map[word].insert(tweet.GetTweetID());
			}
		} 
		return ;
	}

	void SnapShot::GenerateUserTweetMap(Tweet& tweet) {
		auto& user_tweet_map = GetUserTweetMap();
		const std::string& user_id = tweet.GetUserID();
		const std::string& tweet_id = tweet.GetTweetID();
        user_tweet_map[user_id].insert(tweet_id);
		return;
	}

	void SnapShot::SetBurstyWords(BurstyWords&& bursty_word_set) {
		bursty_words_ptr.reset();
		bursty_words_ptr = std::make_unique<BurstyWords>(std::move(bursty_word_set));
		return ;
	}

	BurstyWords& SnapShot::GetBurstyWords() {
		return *bursty_words_ptr;
	}

    void SnapShot::GenerateWordIndexMap() {
        auto& word_set = GetBurstyWords();
        if (word_set.empty()) {
            return ;
        }

        WordIndexMap word_index_map;
        for (auto iter = word_set.begin(); iter != word_set.end(); ++iter) {
            std::string word = *iter;
            word_index_map[word] = std::distance(word_set.begin(), iter);
        }

        this->word_index_ptr.reset();
        this->word_index_ptr = std::make_shared<WordIndexMap>(std::move(word_index_map));
        return ;
    }

	UserTweetMap& SnapShot::GetUserTweetMap() {
		return *user_tweet_map_ptr;
	}

    std::unordered_map<std::string, Tweet>& SnapShot::GetTweetMap() {
        return *tweet_ptr;
    }

    std::unordered_map<std::string, Tweet>& SnapShot::GetNeedPredictTweetMap() {
        return *tweet_need_predict_ptr;
    }

    void SnapShot::CollectTweet(Tweet& tweet) {
        auto& tweet_map = GetTweetMap();
        auto& need_predict_map = GetNeedPredictTweetMap();
        const std::string& tweet_id = tweet.GetTweetID();
        if (tweet.NeedPredictLocation()) {
            need_predict_map[tweet_id] = tweet;
        }
        tweet_map[tweet_id] = tweet;
        return ;
    }

    void SnapShot::ComputeTweetVectorization(ConfigFileHandler& config_file_handler) {
        int embedding_dim = config_file_handler.GetValue("embedding_dim", 25);
        auto& tweet_map = GetTweetMap();
        for (auto& id_tweet : tweet_map) {
            Tweet& tweet = id_tweet.second;
            if (!tweet.GetWordEmbedding().empty()) {
                auto& word_embedding_list = tweet.GetWordEmbedding();
                std::vector<double> tweet_vectorization(embedding_dim, 0.);

                if (word_embedding_list.size() == 1) {
                    tweet_vectorization = word_embedding_list[0].second;
                } else {
                    for (auto& word_vector : word_embedding_list) {
                        double weight = word_vector.first;
                        std::vector<double>& word_vectorization = word_vector.second;
                        std::for_each(word_vectorization.begin(), word_vectorization.end(),
                                      [&weight](double &ele) { ele *= weight; });
                        std::transform(tweet_vectorization.begin(), tweet_vectorization.end(),
                                       word_vectorization.begin(), tweet_vectorization.begin(), std::plus<double>());
                    }
                }
                tweet.SetTweetEmbedding(std::move(tweet_vectorization));
            }
        }
    }

	SnapShot& Window::GetOldest() {
		return sliding_window.front();
	}

	SnapShot& Window::GetLatest() {
		return sliding_window.back();
	}

	void Window::Slide(SnapShot& snapshot) {
		if (sliding_window.size() >= window_size) {
			sliding_window.pop_front();
		}
		sliding_window.emplace_back(snapshot);

		return;
	}
}