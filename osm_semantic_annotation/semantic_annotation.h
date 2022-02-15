//
// Created by dietrich on 08.02.22.
//

#ifndef GEOBURST_OSM_SEMANTIC_ANNOTATION_H
#define GEOBURST_OSM_SEMANTIC_ANNOTATION_H

#include <algorithm>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <cmath>
#include <functional> // std::ref
#include <thread>
#include <mutex>

#include "../pre_processing/Tweet.h"
#include "../pre_processing/OpenStreetMap.h"

using PreProcessing::TweetParser::Tweet;
using PreProcessing::OpenStreetMapParser::OpenStreetMap;

namespace OpenStreetMapAnnotation::SemanticAnnotation {

    using DocumentType = std::unordered_multiset<std::string>;
    using WordScoreType = std::pair<std::string, double>;

    class AnnotationHandler {
    private:
        OpenStreetMap& osm_object;

        std::vector<DocumentType> corpus;

        std::vector<WordScoreType> annotation_candidates;

        std::size_t rank_num = 30;

    public:
        AnnotationHandler(OpenStreetMap& _osm_object, std::vector<Tweet>& candidate_tweets);

        ~AnnotationHandler();

        void Rank();

        std::vector<WordScoreType>& GetAnnotations();
    };

    void MATF(int block, int thread_num, double& avg_doc_length, double& score, std::string& word, std::vector<DocumentType>& corpus);
}
#endif //GEOBURST_OSM_SEMANTIC_ANNOTATION_H
