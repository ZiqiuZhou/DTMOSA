#include <stdexcept>      // for runtime_error
#include <string_view>
#include <vector>
#include <iostream>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "TweetStreamProcess.h"
#include "common/file_io/lines.h"
#include "common/config_handler/config_handler.h"
#include "Tweet.h"
#include "crawled_data_parser.h"

using common::file_io::FileReader;
using common::file_io::FileMode;
using common::file_io::read_lines::LineRange;
using common::config_handler::ConfigFileHandler;
using PreProcessing::TweetParser::Tweet;
using PreProcessing::JsonParser::DataParser;
using boost::gregorian::date;
using boost::posix_time::seconds;
using boost::gregorian::from_simple_string;
using EventTweet::TweetStream::TweetStreamProcess;


int main(int argc, char* argv[])
try {
    ConfigFileHandler config_file_handler = *ConfigFileHandler::GetInstance();
    config_file_handler.Load("D:\\Heidelberg\\master_thesis\\GeoBurst_OSM\\config.conf");
    auto& config_items = config_file_handler.config_items;
    auto filename = config_items["crawled_data"];

    TweetStreamProcess process(config_file_handler);
    process.StreamProcess(*FileReader::open(filename, FileMode::text), config_file_handler);


   /* Tweet tweet;
    DataParser data_parser;
    data_parser.CrawledTweetParser(tweet, preprocessedLines[2]);
    auto time_str = tweet.GetCreateTime();
    std::cout << time_str << std::endl;
    auto time_str2 = "2021-09-29 08:35:44";
    boost::posix_time::ptime d2_1 = boost::posix_time::time_from_string(time_str);
    boost::posix_time::ptime d2_2 = boost::posix_time::time_from_string(time_str2);
    std::cout << d2_1 << std::endl;
    boost::posix_time::time_duration diff = d2_1 - d2_2;
    std::cout << diff.total_seconds() << std::endl;
    std::cout << d2_1 + seconds(3600) << std::endl;*/
    
    
}
catch (std::runtime_error const& e)
{
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
}



