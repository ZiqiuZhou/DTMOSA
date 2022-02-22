//
// Created by dietrich on 04.02.22.
//

#ifndef GEOBURST_OSM_SPATIAL_DATA_INTEGRATION_H
#define GEOBURST_OSM_SPATIAL_DATA_INTEGRATION_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

#include "../common/geo_space/geo_space.h"
#include "../pre_processing/Tweet.h"
#include "../pre_processing/OpenStreetMap.h"

using common::geo_space::DIST;
using common::geo_space::Point;
using common::geo_space::Space;
using common::geo_space::BoundingBox;
using PreProcessing::TweetParser::Tweet;
using PreProcessing::OpenStreetMapParser::OpenStreetMap;

namespace OpenStreetMapAnnotation::SpatialDataIntegration {

    const double EPSILON = 1e-10;

    namespace detail {
        int Compare(double x);

        bool OnLineSegment(Point& p1, Point& p2, Point& Q);

        void BresenhamLine(Point point1, Point point2, Space& space, std::unordered_set<int>& grids);

        bool InPolygon(Point& point, std::vector<std::pair<double, double>>& polygon_vertexes);
    }

    class GeoIntegrationHandler {
    private:
        double dist_threshold; // distance threshold of a point to a line

        Space& space;

        std::vector<Tweet>& tweet_list;

        std::unordered_map<int, std::vector<Tweet>> tweet_location_map; // for each grid store tweets locate in it

    public:
        GeoIntegrationHandler(Space& _space, ConfigFileHandler& config_file_handler, std::vector<Tweet>& tweet_corpus);

        ~GeoIntegrationHandler();

        GeoIntegrationHandler& CreateTweetLocationMap();

        bool IsValidObject(OpenStreetMap& osm_object);

        // for polygon osm objects
        Space CreateMinimalBoundingRectangle(OpenStreetMap& osm_polygon_object);

        std::unordered_set<int> FindBelongingGrids(Space& min_bounding_rectangle);

        std::vector<Tweet> FindCandidateTweetsForPolygon(OpenStreetMap& osm_polygon_object);

        // for line osm objects
        std::unordered_set<int> FindBelongingGrids(std::vector<Point>& points);

        std::vector<Tweet> FindCandidateTweetsForLine(OpenStreetMap& osm_line_object);

        // for point osm objects
        std::vector<Tweet> FindCandidateTweetsForPoint(OpenStreetMap& osm_line_object);
    };
}
#endif //GEOBURST_OSM_SPATIAL_DATA_INTEGRATION_H
