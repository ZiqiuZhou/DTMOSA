#include "event_detection/EventTweet/sliding_window.h"

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

	void SnapShot::SetBurstyWords(BurstyWords&& bursty_word_set) {
		bursty_words_ptr.reset();
		bursty_words_ptr = std::make_unique<BurstyWords>(std::move(bursty_word_set));
		return;
	}

	BurstyWords& SnapShot::GetBurstyWords() {
		return *bursty_words_ptr;
	}

	SnapShot& Window::GetOldest() {
		return sliding_window.front();
	}

	SnapShot& Window::GetLatest() {
		return sliding_window.back();
	}

	void Window::Slide(SnapShot&& snapshot) {
		if (sliding_window.size() >= window_size) {
			sliding_window.pop_front();
		}
		sliding_window.emplace_back(std::move(snapshot));

		return;
	}
}