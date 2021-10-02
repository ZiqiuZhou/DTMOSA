#pragma once
#ifndef TWEET_H
#define TWEET_H

#include <string>
#include <unordered_set>

#include "common/file_io/lines.h"
#include "common/config_handler/config_handler.h"

using common::file_io::FileReader;
using common::file_io::FileMode;
using common::config_handler::ConfigFileHandler;

namespace PreProcessing::TweetParser {
	class Tweet {
	private:
		std::string tweet_id;

		std::string user_name;

		std::string create_time;

		struct Location {
			double longitude = 0.;
			double latitude = 0.;

			Location() {}
			Location(double _longitude, double _latitude) : longitude(_longitude), latitude(_latitude) {}
		} location;

		std::unordered_multiset<std::string> word_bag;

		int snapshot_index = 0;

	public:
		Tweet() {
			word_bag = { };
		}

		~Tweet() {
			tweet_id.clear();
			user_name.clear();
			create_time.clear();
			word_bag.clear();
		}

		Tweet(const Tweet& tweet) {
			this->tweet_id = tweet.tweet_id;
			this->create_time = tweet.create_time;
			this->user_name = tweet.user_name;
			this->word_bag = tweet.word_bag;
			this->location.longitude = tweet.location.longitude;
			this->location.latitude = tweet.location.latitude;
		}

		const Tweet& operator= (const Tweet& tweet) {
			if (this == &tweet) {
				return *this;
			}

			Tweet temp_tweet = tweet;
			std::swap(*this, temp_tweet);
			return *this;
		}

		Tweet(Tweet&& tweet) noexcept {
			this->tweet_id = std::move(std::exchange(tweet.tweet_id, ""));
			this->user_name = std::move(std::exchange(tweet.user_name, ""));
			this->create_time = std::move(std::exchange(tweet.create_time, ""));
			this->word_bag = std::move(std::exchange(tweet.word_bag, { }));
			this->location.longitude = std::exchange(tweet.location.longitude, 0.);
			this->location.latitude = std::exchange(tweet.location.latitude, 0.);
		}

		Tweet& operator= (Tweet&& tweet) noexcept {
			if (this == &tweet) {
				return *this;
			}

			this->tweet_id = std::move(std::exchange(tweet.tweet_id, ""));
			this->tweet_id = std::move(std::exchange(tweet.user_name, ""));
			this->create_time = std::move(std::exchange(tweet.create_time, ""));
			this->word_bag = std::move(std::exchange(tweet.word_bag, { }));
			this->location.longitude = std::exchange(tweet.location.longitude, 0.);
			this->location.latitude = std::exchange(tweet.location.latitude, 0.);

			return *this;
		}

	public:

		void SetTweetID(const std::string& tweet_id) {
			this->tweet_id = tweet_id;
		}

		void SetTweetID(std::string&& tweet_id) {
			this->tweet_id = std::move(std::exchange(tweet_id, ""));
		}

		const std::string GetTweetID() const {
			return this->tweet_id;
		}

		void SetUserName(const std::string& user_name) {
			this->user_name = user_name;
		}

		void SetUserName(std::string&& user_name) {
			this->user_name = std::move(std::exchange(user_name, ""));
		}

		const std::string GetUserName() const {
			return this->user_name;
		}

		void SetCreateTime(const std::string& create_time) {
			this->create_time = create_time;
		}

		void SetCreateTime(std::string&& create_time) {
			this->create_time = std::move(std::exchange(create_time, ""));
		}

		const std::string GetCreateTime() const {
			return this->create_time;
		}

		void SetLongitude(double longitude) {
			this->location.longitude = longitude;
		}

		void SetLatitude(double latitude) {
			this->location.latitude = latitude;
		}

		double GetLongitude() {
			return this->location.longitude;
		}

		double GetLatitude() {
			return this->location.latitude;
		}

		void SetWordBag(const std::unordered_multiset<std::string>& word_bag) {
			if (word_bag.empty()) {
				return;
			}
			this->word_bag = word_bag;
		}

		void SetWordBag(std::unordered_multiset<std::string>&& word_bag) {
			if (word_bag.empty()) {
				return;
			}
			this->word_bag = std::move(std::exchange(word_bag, { }));
		}

		std::unordered_multiset<std::string> GetWordBag() {
			return this->word_bag;
		}

		void SetSnapShotIndex(int t) {
			this->snapshot_index = t;
		}

		int GetSnapShotIndex() {
			return this->snapshot_index;
		}
	};
}; // end namespace PreProcessing::TweetParser

#endif // !TWEET_H
