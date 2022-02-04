//
// Created by dietrich on 04.02.22.
//
#include "spatial_data_integration.h"

namespace OpenStreetMapAnnotation::SpatialDataIntegration {

    GeoIntegrationHandler::GeoIntegrationHandler(Space& _space)
    : space(_space) {
        tweet_location_map = {};
    }

    GeoIntegrationHandler::~GeoIntegrationHandler() {
        tweet_location_map.clear();
    }
}
