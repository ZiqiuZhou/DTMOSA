#include <iostream>

#include "data_parser.h"

namespace PreProcessing::JsonParser {

	bool DataParser::TweetParser(Tweet& tweet, std::string& json_tweet) {
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

        if ((document.HasMember("need_further_predict") && document["need_further_predict"].IsBool())) {
            tweet.SetPredictFlag(document["need_further_predict"].GetBool());
        }

		return true;
	}

    bool DataParser::WordEmbeddingParser(std::string& json_tweet, std::unordered_map<std::string, Tweet>& tweet_map) {
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
        }

        const std::string& id = document["tweet_id"].GetString();
        if (tweet_map.find(id) == tweet_map.end()) {
            return false;
        }
        Tweet& tweet = tweet_map[id];

        if (!(document.HasMember("word_embedding") && document["word_embedding"].IsObject() &&
              !document["word_embedding"].IsNull())) {
            std::cout << "file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " Invalid word embedding element in json." << std::endl;
            return false;
        }

        const Value& doc_embedding = document["word_embedding"];
        auto& word_embedding = tweet.GetWordEmbedding();
        for (Value::ConstMemberIterator iter = doc_embedding.MemberBegin(); iter != doc_embedding.MemberEnd(); ++iter) {
            const Value& weight_vector= iter->value;
            if (weight_vector.IsObject() && !weight_vector.IsNull()) {
                if ((weight_vector.HasMember("weight") && weight_vector["weight"].IsNumber() &&
                     !weight_vector["weight"].IsNull()) &&
                    (weight_vector.HasMember("vectorization") && weight_vector["vectorization"].IsArray() &&
                     !weight_vector["vectorization"].IsNull())) {
                    double weight = weight_vector["weight"].GetDouble();

                    std::vector<double> vectorization;
                    for (auto& ele : weight_vector["vectorization"].GetArray()) {
                        vectorization.push_back(ele.GetDouble());
                    }
                    word_embedding.emplace_back(std::make_pair(weight, vectorization));
                    std::cout << "S";
                }
            } else {
                return false;
            }
        }
       return true;
    }

    void DataParser::TweetToJSON(Tweet& tweet, std::string& str) {
        std::string tweet_id = tweet.GetTweetID();
        std::unordered_multiset<std::string>& word_bag = tweet.GetWordBag();

        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();
        // set tweet_id
        Value tweet_id_value;
        tweet_id_value.SetString(tweet_id.c_str(), allocator);
        document.AddMember("tweet_id", tweet_id_value, allocator);
        // set word bag
        Value array(kArrayType);
        for (std::string word : word_bag) {
            Value word_value;
            word_value.SetString(word.c_str(), allocator);
            array.PushBack(word_value, allocator);
        }
        document.AddMember("word_bag", array, allocator);

        // Stringify the DOM
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        document.Accept(writer);
        str = buffer.GetString();
        str.push_back('\n');
        return ;
    }


}