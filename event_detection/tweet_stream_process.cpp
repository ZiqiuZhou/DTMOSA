#include <unordered_set>

#include "tweet_stream_process.h"

namespace EventTweet::TweetStream {
	TweetStreamProcess::TweetStreamProcess(ConfigFileHandler& config_file_handler) {
		this->time_interval = config_file_handler.GetValue("snapshot_interval", 1);
		auto start_time_str = config_file_handler.GetValue("start_time");
		this->start_time = time_from_string(start_time_str);
	}

    TweetStreamProcess::~TweetStreamProcess() {
        this->time_interval = 0;
        auto start_time_str = "";
        this->start_time = time_from_string(start_time_str);
    }

    time_duration::sec_type TweetStreamProcess::ToTimeDuration(std::string&& time_str_format) {
		ptime current_time = time_from_string(time_str_format);
		time_duration duration = current_time - start_time;
		return duration.total_seconds();
	}

    bool TweetStreamProcess::ProcessGLOVE(DataParser &json_parser, SnapShot &snapshot,
                                          ConfigFileHandler &config_file_handler) {
        bool rtn = false;

        std::string embedding_dict = config_file_handler.GetValue("embedding_dict");
        std::string filename = config_file_handler.GetValue("word_need_embedding");
        std::string embedded_filename = config_file_handler.GetValue("word_after_embedding");
        std::string python_file = config_file_handler.GetValue("process_word_embedding");

        // group filename for each snapshot
        std::string post_fix = "_" + std::to_string(snapshot.GetIndex());
        auto rtn_strings1 = SplitPath(filename);
        std::string need_embedding_file = rtn_strings1[0];
        std::string extension = rtn_strings1[1];
        auto rtn_strings2 = SplitPath(embedded_filename);
        std::string embedded_file = rtn_strings2[0];
        need_embedding_file += (post_fix + extension);
        embedded_file += (post_fix + extension);

        std::ifstream file_stream(need_embedding_file);
        FileWriterNormal file_writer;
        file_writer.open(need_embedding_file, FileMode::text);
        if (file_stream.peek() != std::ifstream::traits_type::eof()) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " Invalid file." << std::endl;
            return rtn;
        }

        auto& tweet_map = snapshot.GetTweetMap();
        for (auto& id_tweet: snapshot.GetTweetMap()) {
            Tweet& tweet = id_tweet.second;
            std::string json_string;
            json_parser.TweetToJSON(tweet, json_string);
            auto buffer = std::span(json_string.begin(), json_string.end());
            file_writer.write(buffer);
        }
        file_writer.close();

        // execute python script
        int argument_number = 4;
        int argc = argument_number;
        std::size_t argv_size = 1024;
        wchar_t* wargv[argument_number];
        wargv[0] = Py_DecodeLocale(python_file.c_str(), &argv_size);
        wargv[1] = Py_DecodeLocale(embedding_dict.c_str(), &argv_size);
        wargv[2] = Py_DecodeLocale(need_embedding_file.c_str(), &argv_size);
        wargv[3] = Py_DecodeLocale(embedded_file.c_str(), &argv_size);

        Py_Initialize();
        if (!Py_IsInitialized()) {
            return rtn;
        }
        PyRun_SimpleString("import sys");
        PySys_SetArgv(argc, wargv);
        FileReaderNormal file_reader;
        file_reader.open(python_file, FileMode::text);
        if (PyRun_SimpleFile(file_reader.GetFilePointer(), python_file.c_str()) != 0) {
            return rtn;
        }
        file_reader.close();
        Py_Finalize();

        // parse word_embedding result
        file_reader.open(embedded_file, FileMode::text);
        for (std::string_view line : linesInFile(std::move(file_reader))) {
            std::string json_tweet = std::string(line);
            if (!json_parser.WordEmbeddingParser(json_tweet, tweet_map)) {
                std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                          << " Parse word_embedding failed." << std::endl;
            }
        }

        // delete file
        std::remove(need_embedding_file.c_str());
        std::remove(embedded_file.c_str());

        rtn = true;
        return rtn;
    }

	bool TweetStreamProcess::StreamProcess(FileReader& file_reader, ConfigFileHandler& config_file_handler) {
		DataParser json_parser;

		HistorySequenceSet history_sequence_set(config_file_handler.GetValue("sequence_length", 200));
		Window sliding_window(config_file_handler.GetValue("window_size", 1));
		SnapShot snapshot(current_snapshot_index);

        std::vector<double> space_bounding_box(config_file_handler.GetVector("space_Houston"));
        Space space(space_bounding_box, 1.0);
		int const window_size = sliding_window.GetWindowSize();
		int const history_length = history_sequence_set.GetHistoryLength();

		// iterate all tweets
        for (std::string_view line : linesInFile(std::move(file_reader))) {
			std::string json_tweet = std::string(line);
			Tweet tweet;
			if (!json_parser.TweetParser(tweet, json_tweet)) {
				continue;
			}
			json_tweet.clear();

            if (!tweet.NeedPredictLocation()) {
                Point point(tweet.GetLongitude(), tweet.GetLatitude());
                if (!space.ContainsPoint(point)) {
                    continue;
                }
                Point new_point = space.ReGenerateCoordinates(point);
                tweet.SetLongitude(new_point.longitude);
                tweet.SetLatitude(new_point.latitude);
            }

			std::string timestamp = tweet.GetCreateTime();
			auto duration = ToTimeDuration(std::move(timestamp));
			// process the entire snapshot
			if (time_interval - duration <= 0) {
                std::cout << "process snapshot: " << snapshot.GetIndex() << std::endl;
				// construct history usage
				if (snapshot.GetIndex() < history_length) {
					history_sequence_set.ManipulateWordHistory(snapshot);
				} else {
                    // 1. get bursty words set at snapshot t
                    BurstyWords bursty_word_set;
                    if (!history_sequence_set.Burst(snapshot, bursty_word_set)) {
                        current_snapshot_index++;
                        snapshot.SetIndex(current_snapshot_index);
                        int step = duration / time_interval;
                        start_time += seconds(step * time_interval);
                        continue;
                    }
                    snapshot.SetBurstyWords(std::move(bursty_word_set));
                    // 2. compute tweet similarity and predict location
                    if (GLOVE) {
                        // word embedding using GLOVE
                        if (ProcessGLOVE(json_parser, snapshot, config_file_handler)) {

                        }
                    }
                    snapshot.GenerateWordIndexMap();
                    TweetSimilarityHandler similarity_handler(snapshot, config_file_handler);
                    similarity_handler.Init()
                                      .GenerateSimMap();
                    TweetLocationPredictor location_predictor(config_file_handler);
                    location_predictor.Predict(similarity_handler);

                    // 3. clustering
                    DBSCAN dbscan(snapshot, similarity_handler, config_file_handler);
                    dbscan.Cluster();
                    auto& points = dbscan.GetPoints();
                    for (auto point: points) {
                        std::cout << std::setprecision(10) << point.longitude << " " << std::setprecision(10) << point.latitude << " " << point.cluster_id << std::endl;
                    }
                }
				// 4. trigger sliding window to slide and switch to next snapshot
				sliding_window.Slide(snapshot);
				snapshot.Reset();

				current_snapshot_index++;
				snapshot.SetIndex(current_snapshot_index);
				int step = duration / time_interval;
				start_time += seconds(step * time_interval);
			}
			snapshot.GenerateUserTweetMap(tweet);
			snapshot.GenerateWordTweetPair(tweet);

            // last step: collect this tweet
            snapshot.CollectTweet(std::move(tweet));
        }

		return true;
	}
}