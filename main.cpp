#include <iostream>
#include <eigen3/Eigen/SparseCore>
#include <eigen3/Eigen/SparseCholesky>
#include "include/rapidjson/document.h"
#include "common/file_io/lines.h"
#include "common/config_handler/config_handler.h"
#include "common/geo_space/geo_space.h"
#include "pre_processing/Tweet.h"
#include "pre_processing/crawled_data_parser.h"
#include "event_detection/tweet_stream_process.h"

using common::file_io::FileReader;
using common::file_io::FileMode;
using common::file_io::read_lines::LineRange;
using common::config_handler::ConfigFileHandler;
using common::geo_space::Space;
using PreProcessing::TweetParser::Tweet;
using PreProcessing::JsonParser::DataParser;
using EventTweet::TweetStream::TweetStreamProcess;

int main() {
    ConfigFileHandler config_file_handler = *ConfigFileHandler::GetInstance();
    config_file_handler.Load("/home/dietrich/master_thesis/GeoBurst_OSM/config.conf");
    auto& config_items = config_file_handler.config_items;
    auto filename = config_items["crawled_data"];
    std::cout << filename << std::endl;

    PreProcessing::TweetParser::Tweet tweet;
    DataParser parser;
    std::string json_tweet = "{\"tweet_id\": \"903044642740035584\", \"user_id\": \"2452134079\", \"time\": \"2017-08-30 23:59:55\", \"longitude\": -95.41837, \"latitude\": 29.7891, \"context\": \"Wish the circumstances were different, but glad I got to spend my day working alongside my https://t.co/ggpHjuRUTN\", \"word_bag\": [\"wish\", \"circumstances\", \"different\", \"glad\", \"spend\", \"working\", \"alongside\"]}";
    parser.CrawledTweetParser(tweet, json_tweet);
    std::cout << tweet.GetTweetID() << " "<< tweet.GetUserID() << " " << tweet.GetCreateTime() << " " << tweet.GetContext();
    config_file_handler.GetVector("space_Houston");
    std::vector<double> coordinates = config_file_handler.GetVector("space_Houston");
    Space space(coordinates, 1.1);
    std::cout << space.GetLength() << " " << space.GetWidth() << std::endl;
    std::cout << space.NumOfCells() << std::endl;


    TweetStreamProcess process(config_file_handler);
    //process.StreamProcess(*FileReader::open(filename, FileMode::text), config_file_handler);
    std::cout << "finished" << std::endl;
    return 0;
}
