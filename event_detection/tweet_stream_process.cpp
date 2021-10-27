#include <unordered_set>

#include "tweet_stream_process.h"

namespace EventTweet::TweetStream {
	TweetStreamProcess::TweetStreamProcess(ConfigFileHandler& config_file_handler) {
		this->time_interval = config_file_handler.GetValue("snapshot_interval", 1);
		auto start_time_str = config_file_handler.GetValue("start_time");
		this->start_time = time_from_string(start_time_str);
	}

	time_duration::sec_type TweetStreamProcess::ToTimeDuration(std::string&& time_str_format) {
		ptime current_time = time_from_string(time_str_format);
		time_duration duration = current_time - start_time;
		return duration.total_seconds();
	}

	bool TweetStreamProcess::StreamProcess(FileReader& fileReader, ConfigFileHandler& config_file_handler) {
		DataParser json_parser;

		HistorySequenceSet history_sequence_set(config_file_handler.GetValue("sequence_length", 200));
		Window sliding_window(config_file_handler.GetValue("window_size", 1));
		SnapShot snapshot(current_snapshot_index);

		int const window_size = sliding_window.GetWindowSize();
		int const history_length = history_sequence_set.GetHistoryLength();
		// iterate all tweets
        for (std::string_view line : linesInFile(std::move(fileReader))) {
			std::string json_tweet = std::string(line);

			Tweet tweet;
			if (!json_parser.CrawledTweetParser(tweet, json_tweet)) {
				continue;
			}
			json_tweet.clear();

			std::string timestamp = tweet.GetCreateTime();
			auto duration = ToTimeDuration(std::move(timestamp));
			// process the entire snapshot
			if (time_interval - duration <= 0) { 
				// construct history usage
				if (snapshot.GetIndex() < history_length) {
					history_sequence_set.ManipulateWordHistory(snapshot);
				}

				// get bursty words set at snapshot t
				if (snapshot.GetIndex() >= history_length) {
					BurstyWords bursty_word_set;
					history_sequence_set.Burst(snapshot, bursty_word_set);
					snapshot.SetBurstyWords(std::move(bursty_word_set));
				}

				// trigger sliding window to slide
				sliding_window.Slide(std::move(snapshot)); 
				snapshot.Reset();

				// switch to next snapshot
				current_snapshot_index++;
				snapshot.SetIndex(current_snapshot_index);
				int step = duration / time_interval;
				start_time += seconds(step * time_interval);
			} 

			tweet.SetSnapShotIndex(current_snapshot_index);
			snapshot.GenerateUserTweetMap(tweet);
			snapshot.GenerateWordTweetPair(tweet);
        }

		return true;
	}
}