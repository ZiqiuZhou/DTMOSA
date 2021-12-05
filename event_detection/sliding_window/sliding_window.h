#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <iterator>

#include "../pre_processing/Tweet.h"

using PreProcessing::TweetParser::Tweet;

namespace EventTweet::SlidingWindow {

	using WordTweetPair = std::unordered_map<std::string, std::unordered_set<std::string> >;
	using BurstyWords = std::unordered_set<std::string>;
	using UserTweetMap = std::unordered_map<std::string, std::string>;
    using WordIndexMap = std::unordered_map<std::string, int>;

	class SnapShot {
	private:
		int t; // snapshot index
		std::shared_ptr<WordTweetPair> word_pair_ptr;
		std::shared_ptr<BurstyWords> bursty_words_ptr;
        std::shared_ptr<WordIndexMap> word_index_ptr;
		std::shared_ptr<UserTweetMap> user_tweet_map_ptr;
        std::shared_ptr<std::unordered_map<std::string, Tweet>> tweet_ptr; // {tweet_id, Tweet}
        std::shared_ptr<std::unordered_map<std::string, Tweet>> tweet_need_predict_ptr; // {tweet_id, Tweet}

	public:
		SnapShot() {
			t = 0;
			word_pair_ptr.reset();
			word_pair_ptr = std::make_shared<WordTweetPair>();
			bursty_words_ptr.reset();
			bursty_words_ptr = std::make_shared<BurstyWords>();
            word_index_ptr.reset();
            word_index_ptr = std::make_shared<WordIndexMap>();
            user_tweet_map_ptr.reset();
            user_tweet_map_ptr = std::make_shared<UserTweetMap>();
            tweet_ptr.reset();
            tweet_ptr = std::make_shared<std::unordered_map<std::string, Tweet>>();
            tweet_need_predict_ptr.reset();
            tweet_need_predict_ptr = std::make_shared<std::unordered_map<std::string, Tweet>>();
		}

		SnapShot(int _t) : t(_t) {
            word_pair_ptr.reset();
            word_pair_ptr = std::make_shared<WordTweetPair>();
            bursty_words_ptr.reset();
            bursty_words_ptr = std::make_shared<BurstyWords>();
            word_index_ptr.reset();
            word_index_ptr = std::make_shared<WordIndexMap>();
            user_tweet_map_ptr.reset();
            user_tweet_map_ptr = std::make_shared<UserTweetMap>();
            tweet_ptr.reset();
            tweet_ptr = std::make_shared<std::unordered_map<std::string, Tweet>>();
            tweet_need_predict_ptr.reset();
            tweet_need_predict_ptr = std::make_shared<std::unordered_map<std::string, Tweet>>();
		};

		~SnapShot() {
            t = 0;
			word_pair_ptr.reset();
			bursty_words_ptr.reset();
            word_index_ptr.reset();
            user_tweet_map_ptr.reset();
            tweet_ptr.reset();
            tweet_need_predict_ptr.reset();
		}

		SnapShot(const SnapShot& snapshot) {
            this->t = snapshot.t;
            this->word_pair_ptr = snapshot.word_pair_ptr;
            this->bursty_words_ptr = snapshot.bursty_words_ptr;
            this->word_index_ptr = snapshot.word_index_ptr;
            this->user_tweet_map_ptr = snapshot.user_tweet_map_ptr;
            this->tweet_ptr = snapshot.tweet_ptr;
            this->tweet_need_predict_ptr = snapshot.tweet_need_predict_ptr;
        }

		SnapShot& operator= (const SnapShot& snapshot) {
            if (this == &snapshot) {
                return *this;
            }
            SnapShot temp_snapshot = snapshot;
            std::swap(*this, temp_snapshot);
            return *this;
        }

		SnapShot(SnapShot&& snapshot) noexcept {
			this->t = snapshot.t;
			this->word_pair_ptr = std::exchange(snapshot.word_pair_ptr, nullptr);
			this->bursty_words_ptr = std::exchange(snapshot.bursty_words_ptr, nullptr);
            this->word_index_ptr = std::exchange(snapshot.word_index_ptr, nullptr);
			this->user_tweet_map_ptr = std::exchange(snapshot.user_tweet_map_ptr, nullptr);
            this->tweet_ptr = std::exchange(snapshot.tweet_ptr, nullptr);
            this->tweet_need_predict_ptr = std::exchange(snapshot.tweet_need_predict_ptr, nullptr);
		}

		SnapShot& operator= (SnapShot&& snapshot) noexcept {
			if (this == &snapshot) {
				return *this;
			}

			this->t = snapshot.t;
			this->word_pair_ptr = std::exchange(snapshot.word_pair_ptr, nullptr);
			this->bursty_words_ptr = std::exchange(snapshot.bursty_words_ptr, nullptr);
            this->word_index_ptr = std::exchange(snapshot.word_index_ptr, nullptr);
			this->user_tweet_map_ptr = std::exchange(snapshot.user_tweet_map_ptr, nullptr);
            this->tweet_ptr = std::exchange(snapshot.tweet_ptr, nullptr);
            this->tweet_need_predict_ptr = std::exchange(snapshot.tweet_need_predict_ptr, nullptr);

			return *this;
		}

	public:
		void Reset() {
            word_pair_ptr.reset();
            word_pair_ptr = std::make_shared<WordTweetPair>();
            bursty_words_ptr.reset();
            bursty_words_ptr = std::make_shared<BurstyWords>();
            word_index_ptr.reset();
            word_index_ptr = std::make_shared<WordIndexMap>();
            user_tweet_map_ptr.reset();
            user_tweet_map_ptr = std::make_shared<UserTweetMap>();
            tweet_ptr.reset();
            tweet_ptr = std::make_shared<std::unordered_map<std::string, Tweet>>();
            tweet_need_predict_ptr.reset();
            tweet_need_predict_ptr = std::make_shared<std::unordered_map<std::string, Tweet>>();

			return ;
		}

		void SetIndex(int index) {
			this->t = index;
			return ;
		}

		int GetIndex() {
			return this->t;
		}

		WordTweetPair& GetWordTweetPair() {
			return *(this->word_pair_ptr);
		}

		void GenerateWordTweetPair(Tweet& tweet);

		void GenerateUserTweetMap(Tweet& tweet);

		void SetBurstyWords(BurstyWords&& bursty_word_set);

		BurstyWords& GetBurstyWords();

        void GenerateWordIndexMap();

		UserTweetMap& GetUserTweetMap();

        std::unordered_map<std::string, Tweet>& GetTweetMap();

        std::unordered_map<std::string, Tweet>& GetNeedPredictTweetMap();

        void CollectTweet(Tweet&& tweet);
	};

	class Window {
	private:
		const int window_size;
		std::deque<SnapShot> sliding_window;

	public:
		explicit Window(const int size) : window_size(size) {
			sliding_window.clear();
		}

		~Window() {
			sliding_window.clear();
		}

	public:
		int GetCurrentSize() {
			return sliding_window.size();
		}

		const int GetWindowSize() const{
			return window_size;
		}

		SnapShot& GetOldest();

		SnapShot& GetLatest();

		void Slide(SnapShot& snapshot);
	};
}

