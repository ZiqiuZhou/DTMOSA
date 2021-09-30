#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <deque>

#include "pre_processing/Tweet.h"

using PreProcessing::TweetStream::Tweet;

namespace EventTweet::SlidingWindow {

	class WordTweetPair {
	private:
		std::string word;
		std::unique_ptr<std::unordered_set<std::string> > tweet_set_ptr;

	public:
		WordTweetPair() {}

		WordTweetPair(const std::string& _word) : word(_word) {
			tweet_set_ptr.reset();
		}

		WordTweetPair(std::string&& _word) : word(std::move(_word)) {
			tweet_set_ptr.reset();
		}

		~WordTweetPair() {
			word.clear();
			tweet_set_ptr.release();
		}

		WordTweetPair(WordTweetPair&& word_tweet_pair) noexcept {
			this->word = std::move(std::exchange(word_tweet_pair.word, ""));
			this->tweet_set_ptr = std::move(std::exchange(word_tweet_pair.tweet_set_ptr, nullptr));
		}

		WordTweetPair& operator= (WordTweetPair&& word_tweet_pair) noexcept {
			if (this == &word_tweet_pair) {
				return *this;
			}

			this->word = std::move(std::exchange(word_tweet_pair.word, ""));
			this->tweet_set_ptr = std::move(std::exchange(word_tweet_pair.tweet_set_ptr, nullptr));

			return *this;
		}

	public:

	};

	class SnapShot {
	private:
		int t; // snapshot index
		std::unique_ptr<std::unordered_set<WordTweetPair> > word_set_ptr;

	public:
		SnapShot() {
			t = 0;
			word_set_ptr.reset();
		}

		SnapShot(int _t) : t(_t) {
			word_set_ptr.reset();
		};

		~SnapShot() {
			word_set_ptr.release();
		}

		SnapShot(SnapShot&& snapshot) noexcept {
			this->t = snapshot.t;
			this->word_set_ptr = std::move(std::exchange(snapshot.word_set_ptr, nullptr));
		}

		SnapShot& operator= (SnapShot&& snapshot) noexcept {
			if (this == &snapshot) {
				return *this;
			}

			this->t = snapshot.t;
			this->word_set_ptr = std::move(std::exchange(snapshot.word_set_ptr, nullptr));

			return *this;
		}

	public:

	};

	class SlidingWindow {
	private:
		const int WINDOW_SIZE;
		std::deque<SnapShot> sliding_window;

	public:
		SlidingWindow(const int size) : WINDOW_SIZE(size) { }

		~SlidingWindow() {
			sliding_window.clear();
		}

	public:
		void Slide(SnapShot&& snapshot);

		void GenerateWordTweetPair(std::vector<Tweet>& tweet_stream);
	};
}

