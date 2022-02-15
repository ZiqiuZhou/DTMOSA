//
// Created by dietrich on 03.02.22.
//

#include <vector>
#include "osm_annotation_process.h"

namespace OpenStreetMapAnnotation::AnnotationProcess {

    const int MIN_NUM_CAND = 5;

    OSM_AnnotationProcess::OSM_AnnotationProcess(std::vector<Tweet> &_tweet_corpus)
    : tweet_corpus(_tweet_corpus){

    }

    void OSM_AnnotationProcess::AnnotateProcess(FileReader& file_reader, ConfigFileHandler& config_file_handler) {
        DataParser json_parser;

        std::vector<double> space_bounding_box(config_file_handler.GetVector("space_Houston"));
        Space space(space_bounding_box, 1.0);

        GeoIntegrationHandler spatial_integration_handler(space, config_file_handler, tweet_corpus);
        spatial_integration_handler.CreateTweetLocationMap();

        for (std::string_view line : linesInFile(std::move(file_reader))) {
            std::string json_osm = std::string(line);
            OpenStreetMap osm_object;

            if (!json_parser.OpenStreetMapParser(osm_object, json_osm)) {
                continue;
            }
            json_osm.clear();

            if (!spatial_integration_handler.IsValidObject(osm_object)) {
                continue;
            }

            // for polygon based osm object
            if (osm_object.GetOSMType() == "Polygon") {
                std::vector<Tweet> candidate_tweets = spatial_integration_handler.FindCandidateTweetsForPolygon(osm_object);
                std::cout << "candidate_tweets number: " << candidate_tweets.size() << " " << osm_object.GetOSMID()
                          << " " << osm_object.GetOSMType() << std::endl;
                if (!candidate_tweets.empty() && candidate_tweets.size() >= MIN_NUM_CAND) {
                    AnnotationHandler annotation_handler(osm_object, candidate_tweets);
                    annotation_handler.Rank();
                    std::vector<WordScoreType>& word_score_list = annotation_handler.GetAnnotations();
                    for (auto& [word, score] : word_score_list) {
                        std::cout << word << ":" << score << " ";
                    }
                    std::cout << std::endl;
                }
            }

            // for line based osm object
            if (osm_object.GetOSMType() == "LineString") {
                std::vector<Tweet> candidate_tweets = spatial_integration_handler.FindCandidateTweetsForLine(osm_object);
                std::cout << "candidate_tweets number: " << candidate_tweets.size() << " " << osm_object.GetOSMID()
                          << " " << osm_object.GetOSMType() << std::endl;
                if (!candidate_tweets.empty() && candidate_tweets.size() >= MIN_NUM_CAND) {
                    AnnotationHandler annotation_handler(osm_object, candidate_tweets);
                    annotation_handler.Rank();
                    std::vector<WordScoreType>& word_score_list = annotation_handler.GetAnnotations();
                    for (auto& [word, score] : word_score_list) {
                        std::cout << word << ":" << score << " ";
                    }
                    std::cout << std::endl;
                }
            }
        }
    }
}
