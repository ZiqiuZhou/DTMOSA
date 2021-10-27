#include <iostream>

#include "crawled_data_parser.h"

namespace PreProcessing::JsonParser {

	bool DataParser::CrawledTweetParser(Tweet& tweet, std::string& json_tweet) {
		document.SetObject();

		ParseResult parse_result = document.Parse(json_tweet.c_str());

		if (!parse_result) {
			fprintf(stderr, "JSON parse error: %s (%u)",
				GetParseError_En(parse_result.Code()), parse_result.Offset());
			exit(EXIT_FAILURE);
		}

		if (!(document.HasMember("tweet_id") && document["tweet_id"].IsString() && !document["tweet_id"].IsNull())) {
			std::cout << "file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
				<< " Invalid tweet_id element in json." << std::endl;
			return false;
		} else {
			tweet.SetTweetID(document["tweet_id"].GetString());
		}

		if (!(document.HasMember("user_id") && document["user_id"].IsString() && !document["user_id"].IsNull())) {
			std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
				<< " Invalid user_id element in json." << std::endl;
			return false;
		} else {
			tweet.SetUserID(document["user_id"].GetString());
		}

		if (!(document.HasMember("time") && document["time"].IsString() && !document["time"].IsNull())) {
			std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
				<< " Invalid time element in json." << std::endl;
			return false;
		} else {
			tweet.SetCreateTime(document["time"].GetString());
		}

        if (!(document.HasMember("context") && document["context"].IsString() && !document["context"].IsNull())) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " Invalid context element in json." << std::endl;
            return false;
        } else {
            tweet.SetContext(document["context"].GetString());
        }

		if (document.HasMember("longitude") && document["longitude"].IsNumber()
			&& document["longitude"].IsDouble() && !document["longitude"].IsNull()) {
			tweet.SetLongitude(document["longitude"].GetDouble());
		}

		if (document.HasMember("latitude") && document["latitude"].IsNumber()
			&& document["latitude"].IsDouble() && !document["latitude"].IsNull()) {
			tweet.SetLatitude(document["latitude"].GetDouble());
		}

		if (!(document.HasMember("word_bag") && document["word_bag"].IsArray())) {
			std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
				<< " Invalid word_bag element in json." << std::endl;
			return false;
		} else {
			const Value& words = document["word_bag"];
			auto word_bag = tweet.GetWordBag();
			// parse each element in context 
			for (SizeType i = 0; i < words.Size(); ++i) {
				if (words[i].IsString() && !words[i].IsNull()) {
					auto segment = words[i].GetString();
					word_bag.insert(segment);
				}
			}
			if (word_bag.empty()) {
				return false;
			}
			tweet.SetWordBag(std::move(word_bag));
		}

		return true;
	}
}