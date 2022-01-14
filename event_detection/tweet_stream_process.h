#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <span>
#include <fstream>
#include <cstdio>
#include <iterator>
#include <python3.8/Python.h>

#include "../common/file_io/lines.h"
#include "../common/config_handler/config_handler.h"
#include "../common/geo_space/geo_space.h"
#include "../pre_processing/Tweet.h"
#include "../pre_processing/data_parser.h"
#include "sliding_window/sliding_window.h"
#include "keyword_extraction/keyword_extraction.h"
#include "tweet_similarity/similarity_handler.h"
#include "clustering/dbscan.h"

using PreProcessing::TweetParser::Tweet;
using PreProcessing::JsonParser::DataParser;
using boost::posix_time::ptime;
using boost::posix_time::time_duration;
using boost::posix_time::time_from_string;
using boost::posix_time::time_duration;
using boost::posix_time::seconds;
using common::file_io::FileReader;
using common::file_io::FileWriter;
using common::file_io::FileMode;
using common::file_io::read_lines::linesInFile;
using common::file_io::FileReaderNormal;
using common::file_io::FileWriterNormal;
using common::file_io::SplitPath;
using common::config_handler::ConfigFileHandler;
using common::geo_space::Point;
using common::geo_space::Space;
using EventTweet::SlidingWindow::WordTweetPair;
using EventTweet::SlidingWindow::BurstyWords;
using EventTweet::SlidingWindow::SnapShot;
using EventTweet::SlidingWindow::Window;
using EventTweet::KeywordExtraction::HistorySequenceSet;
using EventTweet::KeywordExtraction::WordUsageBaseline;
using EventTweet::TweetSimilarity::TweetSimilarityHandler;
using EventTweet::TweetSimilarity::TweetLocationPredictor;
using EventTweet::Clustering::DBSCAN;

namespace EventTweet::TweetStream {

	class TweetStreamProcess {
	private:
		int current_snapshot_index = 0;

		int time_interval; // time interval in seconds

		ptime start_time;

    public:
        bool GLOVE = false;

        bool OPTICS = false;

	public:
		TweetStreamProcess(ConfigFileHandler& config_file_handler);

        ~TweetStreamProcess();

		time_duration::sec_type ToTimeDuration(std::string&& time_str_format);

        bool ProcessGLOVE(DataParser& json_parser, SnapShot& snapshot, ConfigFileHandler& config_file_handler);

		bool StreamProcess(FileReader& fileReader, ConfigFileHandler& config_file_handler);
	};
}
