#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>

#include "pre_processing/Tweet.h"

using PreProcessing::TweetParser::Tweet;

namespace EventTweet::SlidingWindow {

	using WordTweetPair = std::unordered_map<std::string, std::unordered_set<std::string> >;
	using BurstyWords = std::unordered_set<std::string>;

	class SnapShot {
	private:
		int t; // snapshot index
		std::unique_ptr<WordTweetPair> word_pair_ptr;
		std::unique_ptr<BurstyWords> bursty_words_ptr;

	public:
		SnapShot() {
			t = 0;
			word_pair_ptr.reset();
			word_pair_ptr = std::make_unique<WordTweetPair>();
			bursty_words_ptr.reset();
			bursty_words_ptr = std::make_unique<BurstyWords>();
		}

		SnapShot(int _t) : t(_t) {
			word_pair_ptr.reset();
			word_pair_ptr = std::make_unique<WordTweetPair>();
			bursty_words_ptr.reset();
			bursty_words_ptr = std::make_unique<BurstyWords>();
		};

		~SnapShot() {
			word_pair_ptr.reset();
			bursty_words_ptr.reset();
		}

		SnapShot(const SnapShot& snapshot) = delete;

		SnapShot& operator= (const SnapShot& snapshot) = delete;

		SnapShot(SnapShot&& snapshot) noexcept {
			this->t = snapshot.t;
			this->word_pair_ptr = std::move(std::exchange(snapshot.word_pair_ptr, nullptr));
			this->bursty_words_ptr = std::move(std::exchange(snapshot.bursty_words_ptr, nullptr));
		}

		SnapShot& operator= (SnapShot&& snapshot) noexcept {
			if (this == &snapshot) {
				return *this;
			}

			this->t = snapshot.t;
			this->word_pair_ptr = std::move(std::exchange(snapshot.word_pair_ptr, nullptr));
			this->bursty_words_ptr = std::move(std::exchange(snapshot.bursty_words_ptr, nullptr));

			return *this;
		}

	public:
		void Reset() {
			word_pair_ptr.reset();
			word_pair_ptr = std::make_unique<WordTweetPair>();
			bursty_words_ptr.reset();
			bursty_words_ptr = std::make_unique<BurstyWords>();
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

		void SetBurstyWords(BurstyWords&& bursty_word_set);

		BurstyWords& GetBurstyWords();
	};

	class Window {
	private:
		const int window_size;
		std::deque<SnapShot> sliding_window;

	public:
		Window(const int size) : window_size(size) {
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

