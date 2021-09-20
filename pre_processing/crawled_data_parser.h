#pragma once

#include "Tweet.h"
#include "include/rapidjson/document.h"
#include "include/rapidjson/error/en.h"

using PreProcessing::TweetStream::Tweet;
using rapidjson::Document;
using rapidjson::Value;
using rapidjson::SizeType;
using rapidjson::ParseResult;
using rapidjson::GetParseErrorFunc;


namespace PreProcessing::JsonParser {
	class DataParser {
	public:

		Document document;

		void SplitSegment(std::unordered_multiset<std::string>& word_bag, const std::string& segment, char delimiter);

		bool CrawledTweetParser(Tweet& tweet, std::string& json_tweet);
	};
}