#include <iostream>
#include <string>
#include <eigen3/Eigen/SparseCore>
#include <eigen3/Eigen/Dense>
#include "include/rapidjson/document.h"
#include "common/file_io/lines.h"
#include "common/config_handler/config_handler.h"
#include "common/geo_space/geo_space.h"
#include "pre_processing/Tweet.h"
#include "pre_processing/data_parser.h"
#include "event_detection/tweet_stream_process.h"

using common::file_io::FileReader;
using common::file_io::FileMode;
using common::file_io::read_lines::LineRange;
using common::config_handler::ConfigFileHandler;
using common::geo_space::Space;
using PreProcessing::TweetParser::Tweet;
using PreProcessing::JsonParser::DataParser;
using EventTweet::TweetStream::TweetStreamProcess;

int main(int argc, char* argv[]) {
    ConfigFileHandler config_file_handler = *ConfigFileHandler::GetInstance();
    config_file_handler.Load("/home/dietrich/master_thesis/GeoBurst_OSM/config.conf");
    auto& config_items = config_file_handler.config_items;
    auto filename = config_items["crawled_data"];
    std::cout << filename << std::endl;

    PreProcessing::TweetParser::Tweet tweet;
    DataParser parser;
    std::string json_tweet = "{\"tweet_id\": \"903044642740035584\", \"user_id\": \"2452134079\", \"time\": \"2017-08-30 23:59:55\", \"longitude\": -95.41837, \"latitude\": 29.7891, \"context\": \"Wish the circumstances were different, but glad I got to spend my day working alongside my https://t.co/ggpHjuRUTN\", \"word_bag\": [\"wish\", \"circumstances\", \"different\", \"glad\", \"spend\", \"working\", \"alongside\"]}";
    parser.TweetParser(tweet, json_tweet);
    std::cout << tweet.GetTweetID() << " "<< tweet.GetUserID() << " " << tweet.GetCreateTime() << " " << tweet.GetContext() << std::endl;
    std::string s;
    parser.TweetToJSON(tweet, s);
    std::cout << s << std::endl;
    config_file_handler.GetVector("space_Houston");
    std::vector<double> coordinates = config_file_handler.GetVector("space_Houston");
    Space space(coordinates, 1.0);

    TweetStreamProcess process(config_file_handler);
    if (argc == 2 && argv[1] == std::string("GLOVE")) {
        process.GLOVE = true;
    }
    if (argc == 2 && argv[1] == std::string("OPTICS")) {
        process.OPTICS = true;
    }
    if (argc == 3) {
        if (argv[1] == std::string("GLOVE")) {
            process.GLOVE = true;
        }
        if (argv[1] == std::string("OPTICS")) {
            process.OPTICS = true;
        }
        if (argv[2] == std::string("GLOVE")) {
            process.GLOVE = true;
        }
        if (argv[2] == std::string("OPTICS")) {
            process.OPTICS = true;
        }
    }
    //process.StreamProcess(*FileReader::open(filename, FileMode::text), config_file_handler);
//    auto& reader1 = *FileReader::open(filename, FileMode::text);
//    auto filename1 = "/home/dietrich/master_thesis/GeoBurst_OSM/event_detection/GloVe";
//    FileReader& reader = *FileReader::open(filename1, FileMode::text);
//    std::cout << &reader << " " << &reader1 << std::endl;
    std::vector<double> a;
    std::vector<double> b{1., 2., 3.};
    std::transform(a.begin(), a.end(), b.begin(), b.end(), std::plus<double>());
    std::cout << "finish.";
    return 0;
}
