//
// Created by dietrich on 03.02.22.
//

#ifndef GEOBURST_OSM_OSM_ANNOTATION_PROCESS_H
#define GEOBURST_OSM_OSM_ANNOTATION_PROCESS_H

#include <vector>

#include "../common/file_io/lines.h"
#include "../common/file_io/file_io.h"
#include "../common/config_handler/config_handler.h"
#include "../common/geo_space/geo_space.h"
#include "../pre_processing/OpenStreetMap.h"
#include "../pre_processing/Tweet.h"
#include "../pre_processing/data_parser.h"
#include "../event_detection/tweet_stream_process.h"

using PreProcessing::OpenStreetMapParser::OpenStreetMap;
using PreProcessing::TweetParser::Tweet;
using PreProcessing::JsonParser::DataParser;
using common::file_io::FileReader;
using common::file_io::FileMode;
using common::file_io::read_lines::linesInFile;
using common::config_handler::ConfigFileHandler;
using common::geo_space::Space;
using EventTweet::TweetStream::TweetStreamProcess;

namespace OpenStreetMapAnnotation::AnnotationProcess {

    class OSM_AnnotationProcess {
    private:
        std::vector<Tweet>& tweet_corpus;

    public:
        OSM_AnnotationProcess(std::vector<Tweet>& _tweet_corpus);

        void AnnotateProcess(FileReader& file_reader, ConfigFileHandler& config_file_handler);
    };
}

#endif //GEOBURST_OSM_OSM_ANNOTATION_PROCESS_H
