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
#include "osm_semantic_annotation/osm_annotation_process.h"

using common::file_io::FileReader;
using common::file_io::FileMode;
using common::file_io::read_lines::LineRange;
using common::config_handler::ConfigFileHandler;
using common::geo_space::Space;
using PreProcessing::TweetParser::Tweet;
using PreProcessing::JsonParser::DataParser;
using EventTweet::TweetStream::TweetStreamProcess;
using OpenStreetMapAnnotation::AnnotationProcess::OSM_AnnotationProcess;

int main(int argc, char* argv[]) {
    ConfigFileHandler config_file_handler = *ConfigFileHandler::GetInstance();
    config_file_handler.Load("/home/dietrich/master_thesis/GeoBurst_OSM/config.conf");
    auto& config_items = config_file_handler.config_items;
    auto filename_tweet = config_items["crawled_tweet"];
    auto filename_osm = config_items["crawled_osm"];

    TweetStreamProcess stream_process(config_file_handler);
    if (argc == 2 && argv[1] == std::string("GLOVE")) {
        stream_process.has_GLOVE = true;
    }
    if (argc == 2 && argv[1] == std::string("OPTICS")) {
        stream_process.has_OPTICS = true;
    }
    if (argc == 3) {
        if (argv[1] == std::string("GLOVE")) {
            stream_process.has_GLOVE = true;
        }
        if (argv[1] == std::string("OPTICS")) {
            stream_process.has_OPTICS = true;
        }
        if (argv[2] == std::string("GLOVE")) {
            stream_process.has_GLOVE = true;
        }
        if (argv[2] == std::string("OPTICS")) {
            stream_process.has_OPTICS = true;
        }
    }

    FileReader file_reader;
    file_reader.open(filename_tweet, FileMode::text);
    stream_process.StreamProcess(file_reader, config_file_handler);

    file_reader.open(filename_osm, FileMode::text);
    OSM_AnnotationProcess annotate_process(stream_process.tweet_corpus);
    annotate_process.AnnotateProcess(file_reader, config_file_handler);
    std::cout << "finish.";
    return 0;
}
