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
		std::string user_id = tweet.GetUserID();
		std::string tweet_id = tweet.GetTweetID();
        user_tweet_map[user_id] = tweet_id;
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

    void SnapShot::CollectTweet(Tweet&& tweet) {
        auto& tweet_map = GetTweetMap();
        auto& need_predict_map = GetNeedPredictTweetMap();
        const std::string& tweet_id = tweet.GetTweetID();
        if (tweet.GetPredictFlag()) {
            need_predict_map[tweet_id] = tweet;
        }
        tweet_map[tweet_id] = std::move(tweet);
        return ;
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