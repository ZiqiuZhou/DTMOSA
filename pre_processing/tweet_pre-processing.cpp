#include <stdexcept>      // for runtime_error
#include <string_view>

#include "common/file_io/lines.h"
#include "common/config_handler/config_handler.h"
#include "Tweet.h"
#include "crawled_data_parser.h"

using common::file_io::FileReader;
using common::file_io::FileMode;
using common::file_io::read_lines::linesInFile;
using common::config_handler::ConfigFileHandler;
using PreProcessing::TweetStream::Tweet;
using PreProcessing::JsonParser::DataParser;

std::vector<std::string>
preprocess2(
    FileReader& fileReader,
    std::vector<std::string> const& commentPrefixes)
{
    std::vector<std::string> result;
    for (std::string_view line : linesInFile(std::move(fileReader)))
    {
        if (std::none_of(commentPrefixes.begin(), commentPrefixes.end(),
            [line](std::string const& commentPrefix)
            {
                return line.starts_with(commentPrefix);
            }))
        {
            result.emplace_back(std::move(std::string(line)));
        }
    }
    return result;
}


int main(int argc, char* argv[])
try {
    ConfigFileHandler* config_file_handler = ConfigFileHandler::GetInstance();
    config_file_handler->Load("D:\\Heidelberg\\master_thesis\\GeoBurst_OSM\\config.conf");
    auto& config_items = config_file_handler->config_items;
    auto commentPrefixes = std::vector<std::string>{ "//", "#" };
    auto filename = config_items["crawled_data"];
    auto preprocessedLines = preprocess2(
        *FileReader::open(filename, FileMode::text),
        commentPrefixes);

    Tweet tweet;
    DataParser data_parser;
    data_parser.CrawledTweetParser(tweet, preprocessedLines[2]);
    std::cout << tweet.GetTweetID() << std::endl;
    std::cout << tweet.GetCreateTime() << std::endl;
    std::cout << tweet.GetUserName() << std::endl;
    std::cout << tweet.GetLongitude() << std::endl;
    std::cout << tweet.GetLatitude() << std::endl;
    auto bags = tweet.GetWordBag();
    for (auto& word : bags) {
        std::cout << word << " ";
    }
}
catch (std::runtime_error const& e)
{
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
}



