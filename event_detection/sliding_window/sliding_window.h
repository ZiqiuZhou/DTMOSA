#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>

#include "../pre_processing/Tweet.h"

using PreProcessing::TweetParser::Tweet;

namespace EventTweet::SlidingWindow {

	using WordTweetPair = std::unordered_map<std::string, std::unordered_set<std::string> >;
	using BurstyWords = std::unordered_set<std::string>;
	using UserTweetMap = std::unordered_map<std::string, std::string>;

	class SnapShot {
	private:
		int t; // snapshot index
		std::shared_ptr<WordTweetPair> word_pair_ptr;
		std::shared_ptr<BurstyWords> bursty_words_ptr;
		std::shared_ptr<UserTweetMap> user_tweet_map_ptr;

	public:
		SnapShot() {
			t = 0;
			word_pair_ptr.reset();
			word_pair_ptr = std::make_shared<WordTweetPair>();
			bursty_words_ptr.reset();
			bursty_words_ptr = std::make_shared<BurstyWords>();
            user_tweet_map_ptr.reset();
            user_tweet_map_ptr = std::make_shared<UserTweetMap>();
		}

		SnapShot(int _t) : t(_t) {
			word_pair_ptr.reset();
			word_pair_ptr = std::make_shared<WordTweetPair>();
			bursty_words_ptr.reset();
			bursty_words_ptr = std::make_shared<BurstyWords>();
            user_tweet_map_ptr.reset();
            user_tweet_map_ptr = std::make_shared<UserTweetMap>();
		};

		~SnapShot() {
            t = 0;
			word_pair_ptr.reset();
			bursty_words_ptr.reset();
            user_tweet_map_ptr.reset();
		}

		SnapShot(const SnapShot& snapshot) {
            this->t = snapshot.t;
            this->word_pair_ptr = snapshot.word_pair_ptr;
            this->bursty_words_ptr = snapshot.bursty_words_ptr;
            this->user_tweet_map_ptr = snapshot.user_tweet_map_ptr;
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
			this->user_tweet_map_ptr = std::exchange(snapshot.user_tweet_map_ptr, nullptr);
		}

		SnapShot& operator= (SnapShot&& snapshot) noexcept {
			if (this == &snapshot) {
				return *this;
			}

			this->t = snapshot.t;
			this->word_pair_ptr = std::exchange(snapshot.word_pair_ptr, nullptr);
			this->bursty_words_ptr = std::exchange(snapshot.bursty_words_ptr, nullptr);
			this->user_tweet_map_ptr = std::exchange(snapshot.user_tweet_map_ptr, nullptr);

			return *this;
		}

	public:
		void Reset() {
			word_pair_ptr.reset();
			word_pair_ptr = std::make_shared<WordTweetPair>();
			bursty_words_ptr.reset();
			bursty_words_ptr = std::make_shared<BurstyWords>();
            user_tweet_map_ptr.reset();
            user_tweet_map_ptr = std::make_shared<UserTweetMap>();

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

		UserTweetMap& GetUserTweetMap();

		bool HasDuplicateUser(Tweet& tweet);
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

		void Slide(SnapShot&& snapshot);
	};
}

