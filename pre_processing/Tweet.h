#pragma once
#ifndef TWEET_H
#define TWEET_H

#include <vector>
#include <string>
#include <unordered_set>
#include <utility>

#include "../common/file_io/lines.h"
#include "../common/config_handler/config_handler.h"

using common::file_io::FileReader;
using common::file_io::FileMode;
using common::config_handler::ConfigFileHandler;

namespace PreProcessing::TweetParser {
    const double INVALID_NUM = 0.;

	class Tweet {
	private:
		std::string tweet_id;

		std::string user_id;

		std::string create_time;

		struct Location {
			double longitude = INVALID_NUM;
			double latitude = INVALID_NUM;

			Location() {}
			Location(double _longitude, double _latitude) : longitude(_longitude), latitude(_latitude) {}
		} location;

        std::string context;

		std::unordered_multiset<std::string> word_bag;

        std::vector<std::pair<double, std::vector<double>>> word_embedding; // (word_weight, vectorization representation of word)

        std::vector<double> tweet_embedding;

        bool need_further_predict = false;

        int index = 0;

	public:
		Tweet() {
            need_further_predict = false;
			word_bag = { };
            word_embedding = { };
            tweet_embedding = { };
		}

		~Tweet() {
			tweet_id.clear();
            user_id.clear();
			create_time.clear();
            context.clear();
			word_bag.clear();
            word_embedding.clear();
            tweet_embedding.clear();
		}

		Tweet(const Tweet& tweet) {
			this->tweet_id = tweet.tweet_id;
			this->create_time = tweet.create_time;
			this->user_id = tweet.user_id;
            this->context = tweet.context;
			this->word_bag = tweet.word_bag;
            this->word_embedding = tweet.word_embedding;
            this->tweet_embedding = tweet.tweet_embedding;
			this->location.longitude = tweet.location.longitude;
			this->location.latitude = tweet.location.latitude;
			this->need_further_predict = tweet.need_further_predict;
		}

		Tweet& operator= (const Tweet& tweet) {
			if (this == &tweet) {
				return *this;
			}

			Tweet temp_tweet = tweet;
			std::swap(*this, temp_tweet);
			return *this;
		}

		Tweet(Tweet&& tweet) noexcept {
			this->tweet_id = std::exchange(tweet.tweet_id, "");
			this->user_id = std::exchange(tweet.user_id, "");
			this->create_time = std::exchange(tweet.create_time, "");
            this->context = std::exchange(tweet.context, "");
			this->word_bag = std::exchange(tweet.word_bag, { });
            this->word_embedding = std::exchange(tweet.word_embedding, { });
            this->tweet_embedding = std::exchange(tweet.tweet_embedding, { });
			this->location.longitude = std::exchange(tweet.location.longitude, 0.);
			this->location.latitude = std::exchange(tweet.location.latitude, 0.);
			this->need_further_predict = std::exchange(tweet.need_further_predict, false);
		}

		Tweet& operator= (Tweet&& tweet) noexcept {
			if (this == &tweet) {
				return *this;
			}

			this->tweet_id = std::exchange(tweet.tweet_id, "");
			this->user_id = std::exchange(tweet.user_id, "");
			this->create_time = std::exchange(tweet.create_time, "");
            this->context = std::exchange(tweet.context, "");
			this->word_bag = std::exchange(tweet.word_bag, { });
            this->word_embedding = std::exchange(tweet.word_embedding, { });
            this->tweet_embedding = std::exchange(tweet.tweet_embedding, { });
			this->location.longitude = std::exchange(tweet.location.longitude, 0.);
			this->location.latitude = std::exchange(tweet.location.latitude, 0.);
			this->need_further_predict = std::exchange(tweet.need_further_predict, false);

			return *this;
		}

	public:

        [[maybe_unused]] void SetTweetID(const std::string& _tweet_id) {
			this->tweet_id = _tweet_id;
		}

		void SetTweetID(std::string&& _tweet_id) {
			this->tweet_id = std::exchange(_tweet_id, "");
		}

		const std::string& GetTweetID() const {
			return this->tweet_id;
		}

        [[maybe_unused]] void SetUserID(const std::string& _user_id) {
			this->user_id = _user_id;
		}

		void SetUserID(std::string&& _user_id) {
			this->user_id = std::exchange(_user_id, "");
		}

		const std::string& GetUserID() const {
			return this->user_id;
		}

        [[maybe_unused]] void SetCreateTime(const std::string& _create_time) {
			this->create_time = _create_time;
		}

		void SetCreateTime(std::string&& _create_time) {
			this->create_time = std::exchange(_create_time, "");
		}

		const std::string& GetCreateTime() const {
			return this->create_time;
		}

		void SetLongitude(double longitude) {
			this->location.longitude = longitude;
		}

		void SetLatitude(double latitude) {
			this->location.latitude = latitude;
		}

		double GetLongitude() const {
			return this->location.longitude;
		}

		double GetLatitude() const {
			return this->location.latitude;
		}

        void SetContext(const std::string& _context) {
            std::string http = "http";
            std::size_t found = _context.find(http);
            if (found == std::string::npos) {
                this->context = _context;
            } else {
                this->context.clear();
                for (std::size_t i = 0; i < found; ++i) {
                    this->context.push_back(_context[i]);
                }
                if (!this->context.empty() && this->context.back() == ' ') {
                    this->context.pop_back();
                }
            }

            return ;
        }

        void SetContext(std::string&& _context) {
            std::string http = "http";
            std::size_t found = _context.find(http);
            if (found == std::string::npos) {
                this->context = std::move(_context);
            } else {
                this->context.clear();
                for (std::size_t i = 0; i < found; ++i) {
                    this->context.push_back(_context[i]);
                }
                if (!this->context.empty() && this->context.back() == ' ') {
                    this->context.pop_back();
                }
            }

            return ;
        }

        const std::string& GetContext() const {
            return this->context;
        }

		void SetWordBag(const std::unordered_multiset<std::string>& _word_bag) {
			if (_word_bag.empty()) {
				return;
			}
			this->word_bag = _word_bag;
		}

		void SetWordBag(std::unordered_multiset<std::string>&& _word_bag) {
			if (_word_bag.empty()) {
				return;
			}
			this->word_bag = std::exchange(_word_bag, { });
		}

		std::unordered_multiset<std::string>& GetWordBag() {
			return this->word_bag;
		}

        void SetWordEmbedding(const std::vector<std::pair<double, std::vector<double>>>& _word_embedding) {
            if (!_word_embedding.empty()) {
                this->word_embedding = _word_embedding;
            }
        }

        void SetWordEmbedding(std::vector<std::pair<double, std::vector<double>>>&& _word_embedding) {
            if (!_word_embedding.empty()) {
                this->word_embedding = std::exchange(_word_embedding, { });
            }
        }

        std::vector<std::pair<double, std::vector<double>>>& GetWordEmbedding() {
            return this->word_embedding;
        }

        void SetTweetEmbedding(const std::vector<double>& _tweet_embedding) {
            if (!_tweet_embedding.empty()) {
                this->tweet_embedding = _tweet_embedding;
            }
        }

        void SetTweetEmbedding(std::vector<double>&& _tweet_embedding) {
            if (!_tweet_embedding.empty()) {
                this->tweet_embedding = std::exchange(_tweet_embedding, { });
            }
        }

        std::vector<double>& GetTweetEmbedding() {
            return this->tweet_embedding;
        }

        void SetPredictFlag(bool flag) {
            this->need_further_predict = flag;
        }

        bool NeedPredictLocation() const {
            return this->need_further_predict;
        }

        void SetIndex(int _index) {
            this->index = _index;
        }

        int GetIndex() const {
            return this->index;
        }
	};
} // end namespace PreProcessing::TweetParser

#endif // !TWEET_H
