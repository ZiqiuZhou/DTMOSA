#pragma once

#include <unordered_set>
#include <string>

#include "Tweet.h"
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/writer.h"
#include "../include/rapidjson/stringbuffer.h"
#include "../include/rapidjson/error/en.h"

using PreProcessing::TweetParser::Tweet;
using rapidjson::Document;
using rapidjson::Value;
using rapidjson::SizeType;
using rapidjson::kArrayType;
using rapidjson::kObjectType;
using rapidjson::ParseResult;
using rapidjson::StringBuffer;
using rapidjson::Writer;
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

		bool TweetParser(Tweet& tweet, std::string& json_tweet);

        void TweetToJSON(Tweet& tweet, std::string& str);
	};
}