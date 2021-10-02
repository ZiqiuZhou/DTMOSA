#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "TweetStreamProcess.h"

namespace EventTweet::TweetStream {
	TweetStreamProcess::TweetStreamProcess(ConfigFileHandler& config_file_handler) {
		this->time_interval = config_file_handler.GetValue("snapshot_interval", 1);
		auto start_time_str = config_file_handler.GetValue("start_time");
		this->start_time = time_from_string(start_time_str);
	}

	time_duration::sec_type TweetStreamProcess::ToTimeDuration(std::string&& time_str_format) {
		ptime current_time = time_from_string(std::move(time_str_format));
		time_duration duration = current_time - start_time;
		return duration.total_seconds();
	}

	bool TweetStreamProcess::StreamProcess(FileReader& fileReader, ConfigFileHandler& config_file_handler) {
		DataParser json_parser;

		Window sliding_window(config_file_handler.GetValue("window_size", 1));
		int window_size = sliding_window.GetWindowSize();
		SnapShot snapshot(current_snapshot_index);

		// iterate all tweets
        for (std::string_view line : linesInFile(std::move(fileReader))) {
			std::string json_tweet = std::string(line);

			Tweet tweet;
			if (!json_parser.CrawledTweetParser(tweet, json_tweet)) {
				std::cout << "file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
					<< " Invalid parser." << std::endl;
				continue;
			}
			json_tweet.clear();
			// measure tweet created time
			std::string timestamp = tweet.GetCreateTime();
			auto duration = ToTimeDuration(std::move(timestamp));
			if (time_interval - duration <= 0) { 
				sliding_window.Slide(std::move(snapshot)); // trigger sliding window to slide
				snapshot.Reset();

				// switch to next snapshot
				current_snapshot_index++;
				snapshot.SetIndex(current_snapshot_index);
				start_time += seconds(time_interval);
			} 

			tweet.SetSnapShotIndex(current_snapshot_index);
			snapshot.GenerateWordTweetPair(tweet);
        }

		auto& current_snapshot = sliding_window.GetBack();
		auto& first_snapshot = sliding_window.GetFront();
		std::cout << current_snapshot.GetIndex() << std::endl;
		std::cout << first_snapshot.GetIndex() << std::endl;
		std::cout << current_snapshot.GetWordTweetPair().size() << std::endl;
		std::cout << first_snapshot.GetWordTweetPair().size() << std::endl;
		return true;
	}
}