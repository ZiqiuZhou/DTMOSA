//
// Created by dietrich on 03.02.22.
//

#include <vector>
#include "osm_annotation_process.h"

namespace OpenStreetMapAnnotation::AnnotationProcess {

    OSM_AnnotationProcess::OSM_AnnotationProcess(std::vector<Tweet> &_tweet_corpus)
    : tweet_corpus(_tweet_corpus){

    }

    void OSM_AnnotationProcess::AnnotateProcess(FileReader& file_reader, ConfigFileHandler& config_file_handler) {
        DataParser json_parser;

        std::vector<double> space_bounding_box(config_file_handler.GetVector("space_Houston"));
        Space space(space_bounding_box, 1.0);

        for (std::string_view line : linesInFile(std::move(file_reader))) {
            std::string json_osm = std::string(line);
            OpenStreetMap osm_object;

            if (!json_parser.OpenStreetMapParser(osm_object, json_osm)) {
                continue;
            }
            json_osm.clear();


        }
    }
}
