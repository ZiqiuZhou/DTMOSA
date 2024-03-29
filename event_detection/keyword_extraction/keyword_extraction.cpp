#include "keyword_extraction.h"

namespace EventTweet::KeywordExtraction {

    const static int occurrence_frequency = 5;

	double WordUsageBaseline::GetMean() {
		if (usage_history.empty()) {
			return 0.;
		}

		return std::accumulate(usage_history.begin(), usage_history.end(), 0.) / usage_history.size();
	}

	double WordUsageBaseline::GetStandardDeviation() {
		if (usage_history.empty()) {
			return 0.;
		}
		double mean = GetMean();
		std::vector<double> temp_usage(usage_history.size());
		std::copy(usage_history.begin(), usage_history.end(), temp_usage.begin());
		std::for_each(temp_usage.begin(), temp_usage.end(), 
			[&mean](double& element) { 
				element -= mean;
				element = pow(element, 2);
			});

		double variance = std::accumulate(temp_usage.begin(), temp_usage.end(), 0.) / usage_history.size();
		return sqrt(variance);
	}

	bool HistorySequenceSet::Contains(const std::string& word) {
		if (!words_history.empty() && words_history.find(word) != words_history.end()) {
			return true;
		}
		return false;
	}

	void HistorySequenceSet::ConstructWordHistory(const std::string& word, SnapShot& snapshot) {
		if (!Contains(word)) {
			words_history[word] = { };
			AddWordUsage(word, snapshot);
		}
		return ;
	}

	void HistorySequenceSet::AddWordUsage(const std::string& word, SnapShot& snapshot) {
		if (Contains(word)) {
			WordUsageBaseline& word_baseline = words_history[word];
			WordTweetPair& word_tweet_pair = snapshot.GetWordTweetPair();
			auto& tweetID_set = word_tweet_pair.at(word);
			word_baseline.Append(tweetID_set.size());
		}
		return ;
	}

	void HistorySequenceSet::ManipulateWordHistory(SnapShot& snapshot) {
		WordTweetPair& word_tweet_pair = snapshot.GetWordTweetPair();
		if (word_tweet_pair.empty()) {
			return ;
		}

		auto iter = word_tweet_pair.begin();
		for (; iter != word_tweet_pair.end(); ++iter) {
			const std::string& word = (*iter).first;
			if (Contains(word)) {
				AddWordUsage(word, snapshot);
			} else {
				ConstructWordHistory(word, snapshot);
			}
		}
		return ;
	}

	bool HistorySequenceSet::Burst(SnapShot& snapshot, std::unordered_set<std::string>& bursty_word_set) {
		auto& word_tweet_pair = snapshot.GetWordTweetPair();
		if (word_tweet_pair.empty()) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " word_tweet_pair is empty." << std::endl;
			return false;
		}

		for (auto iter = word_tweet_pair.begin(); iter != word_tweet_pair.end(); ++iter) {
			std::string word = (*iter).first;
			const std::unordered_set<std::string>& tweetID_set = (*iter).second;
            if (tweetID_set.size() >= occurrence_frequency) {
                bursty_word_set.insert(std::move(word));
            } else if (Contains(word)) {
                WordUsageBaseline& word_baseline = words_history[word];
                int num_occurrence = tweetID_set.size();
//                if ((num_occurrence - word_baseline.GetMean()) / word_baseline.GetStandardDeviation() < 0.) {
//                    bursty_word_set.insert(std::move(word));
//                }
                if (num_occurrence >= occurrence_frequency) {
                    bursty_word_set.insert(std::move(word));
                }
            }
		}

        if(bursty_word_set.empty()) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " bursty_word_set is empty." << std::endl;
            return false;
        }
		return true;
	}
}