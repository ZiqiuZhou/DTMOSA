//
// Created by dietrich on 04.02.22.
//

#ifndef GEOBURST_OSM_SPATIAL_DATA_INTEGRATION_H
#define GEOBURST_OSM_SPATIAL_DATA_INTEGRATION_H

#include <unordered_map>
#include <vector>
#include <string>

#include "../common/geo_space/geo_space.h"
#include "../pre_processing/Tweet.h"

using common::geo_space::Space;
using PreProcessing::TweetParser::Tweet;

namespace OpenStreetMapAnnotation::SpatialDataIntegration {

    class GeoIntegrationHandler {
    private:
        Space& space;

        std::unordered_map<int, std::vector<Tweet>> tweet_location_map; // for each grid store tweets locate in it

    public:
        GeoIntegrationHandler(Space& _space);

        ~GeoIntegrationHandler();
    };
}
#endif //GEOBURST_OSM_SPATIAL_DATA_INTEGRATION_H
