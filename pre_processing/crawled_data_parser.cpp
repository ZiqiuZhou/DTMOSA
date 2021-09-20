#include <iostream>
#include <sstream>
#include <algorithm>
#include <typeinfo>

#include "crawled_data_parser.h"

namespace PreProcessing::JsonParser {

	void DataParser::SplitSegment(std::unordered_multiset<std::string>& word_bag, const std::string& segment, char delimiter) {
		std::string token;
		std::istringstream tokenStream(segment);
		while (std::getline(tokenStream, token, delimiter)) {
			// convert string to back to lower case
			if (!token.empty()) {
				std::for_each(token.begin(), token.end(), [](char& ch) {
					ch = ::tolower(ch);
					});

				word_bag.insert(token);
			}
		}
		return ;
	}

	bool DataParser::CrawledTweetParser(Tweet& tweet, std::string& json_tweet) {
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

		if (!(document.HasMember("user_name") && document["user_name"].IsString() && !document["user_name"].IsNull())) {
			std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
				<< " Invalid user_name element in json." << std::endl;
			return false;
		} else {
			tweet.SetUserName(document["user_name"].GetString());
		}

		if (!(document.HasMember("time") && document["time"].IsString() && !document["time"].IsNull())) {
			std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
				<< " Invalid time element in json." << std::endl;
			return false;
		} else {
			tweet.SetCreateTime(document["time"].GetString());
		}

		if (!(document.HasMember("longitude") && document["longitude"].IsNumber()
			&& document["longitude"].IsDouble() && !document["longitude"].IsNull())) {
			std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
				<< " Invalid longitude element in json." << std::endl;
			return false;
		} else {
			tweet.SetLongitude(document["longitude"].GetDouble());
		}

		if (!(document.HasMember("latitude") && document["latitude"].IsNumber()
			&& document["latitude"].IsDouble() && !document["latitude"].IsNull())) {
			std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
				<< " Invalid latitude element in json." << std::endl;
			return false;
		}
		else {
			tweet.SetLatitude(document["latitude"].GetDouble());
		}

		if (!(document.HasMember("context") && document["context"].IsArray())) {
			std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
				<< " Invalid context element in json." << std::endl;
			return false;
		} else {
			const Value& context = document["context"];

			auto word_bag = tweet.GetWordBag();
			// parse each element in context 
			for (SizeType i = 0; i < context.Size(); ++i) {
				if (context[i].IsString() && !context[i].IsNull()) {
					auto segement = context[i].GetString();

					std::unordered_multiset<std::string> word_sub_bag;
					SplitSegment(word_sub_bag, segement, ' '); // split element into single words -- tokenization
					word_bag.insert(word_sub_bag.begin(), word_sub_bag.end());
					word_sub_bag.clear();
				}
			}
			tweet.SetWordBag(std::move(word_bag));
		}

		return true;
	}
};