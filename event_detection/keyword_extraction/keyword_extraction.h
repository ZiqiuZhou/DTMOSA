#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

#include "sliding_window/sliding_window.h"

using EventTweet::SlidingWindow::WordTweetPair;
using EventTweet::SlidingWindow::SnapShot;

namespace EventTweet::KeywordExtraction {
	
	class WordUsageBaseline {
	private:
		std::vector<std::size_t> usage_history;
		double mean = 0.;
		double standard_deviation = 0.;

	public:
		WordUsageBaseline() {
			usage_history = { };
		}

		~WordUsageBaseline() {
			usage_history.clear();
		}

	public:
		void Append(std::size_t num) {
			usage_history.emplace_back(num);
			return ;
		}

		// estimate mean and standard deviation using maximum likelihood estimation
		double GetMean();

		double GetStandardDeviation();
	};

	class HistorySequenceSet {
	private:
		int sequence_length;
		std::unordered_map<std::string, WordUsageBaseline> words_history;

	public:
		HistorySequenceSet(int length) : sequence_length(length) {
			words_history = { };
		}

		~HistorySequenceSet() {
			words_history.clear();
		}

	public:
		int GetHistoryLength() {
			return sequence_length;
		}

		bool Contains(const std::string& word);

		void ConstructWordHistory(const std::string& word, SnapShot& snapshot);

		void AddWordUsage(const std::string& word, SnapShot& snapshot);

		void ManipulateWordHistory(SnapShot& snapshot);

		bool Burst(SnapShot& snapshot, std::unordered_set<std::string>& bursty_word_set);
	};
}