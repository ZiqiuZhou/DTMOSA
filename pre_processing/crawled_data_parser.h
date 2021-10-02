#pragma once

#include "Tweet.h"
#include "include/rapidjson/document.h"
#include "include/rapidjson/error/en.h"

using PreProcessing::TweetParser::Tweet;
using rapidjson::Document;
using rapidjson::Value;
using rapidjson::SizeType;
using rapidjson::ParseResult;
using rapidjson::GetParseErrorFunc;


namespace PreProcessing::JsonParser {
	class DataParser {
	private:
		Document document;

	public:
		DataParser() {
			document.SetObject();
		}

		~DataParser() {
			document.SetObject();
		}

		bool CrawledTweetParser(Tweet& tweet, std::string& json_tweet);
	};
}