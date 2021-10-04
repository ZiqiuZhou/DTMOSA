#include <stdexcept>      // for runtime_error
#include <string_view>
#include <vector>
#include <iostream>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "tweet_stream_process.h"
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
    //process.StreamProcess(*FileReader::open(filename, FileMode::text), config_file_handler);
    
    std::vector<double> cor = {};
    cor = config_file_handler.GetValue("space_NY", cor);
    for (auto a : cor) {
        std::cout << a << " ";
    }
}
catch (std::runtime_error const& e)
{
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
}



