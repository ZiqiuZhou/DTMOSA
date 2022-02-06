//
// Created by dietrich on 04.02.22.
//
#include "spatial_data_integration.h"

namespace OpenStreetMapAnnotation::SpatialDataIntegration {

    namespace detail {
        int Compare(double x) {
            if (std::fabs(x) < EPSILON) {
                return 0;
            }
            return x < 0 ? -1 : 1;
        }

        bool OnLineSegment(Point& p1, Point& p2, Point& Q) {
            return (Compare((p1 - Q) ^ (p2 - Q)) == 0) && (Compare((p1 - Q) * (p2 - Q)) <= 0);
        }

        void BresenhamLine(Point point1, Point point2, Space& space, std::unordered_set<int>& grids) {
            double dx = point2.longitude - point1.longitude;
            double dy = point2.latitude - point2.latitude;
            double m = std::fabs(dy / dx);

            const int x_symbol = dx > 0 ? 1 : -1;
            const int y_symbol = dy > 0 ? 1 : -1;

            double x_i = x_symbol * point1.longitude;
            double y_i = y_symbol * point1.latitude;

            if (m <= 1) {
                double p_i = 2 * y_symbol * dy - x_symbol * dx;
                while (x_i < x_symbol * point2.longitude + 1) {
                    if (p_i < 0) {
                        p_i += 2 * y_symbol * dy;
                    } else {
                        p_i += 2 * y_symbol * dy - 2 * x_symbol * dx;
                        y_i++;
                    }
                    if (space.ContainsPoint(x_symbol * x_i, y_symbol * y_i)) {
                        grids.insert(space.GetCellIndex(x_symbol * x_i, y_symbol * y_i));
                    }
                    x_i++;
                }
            } else {
                double p_i = 2 * dx - dy;
                while (y_i < y_symbol * point2.latitude + 1) {
                    if (p_i < 0) {
                        p_i += 2 * x_symbol * dx;
                    } else {
                        p_i += 2 * x_symbol * dx - 2 * y_symbol * dy;
                        x_i++;
                    }
                    if (space.ContainsPoint(x_symbol * x_i, y_symbol * y_i)) {
                        grids.insert(space.GetCellIndex(x_symbol * x_i, y_symbol * y_i));
                    }
                    y_i++;
                }
            }
            return ;
        }

        bool InPolygon(Point& point, std::vector<std::pair<double, double>>& polygon_vertexes) {
            std::size_t n = polygon_vertexes.size();
            bool flag = false;
            for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
                Point P1(polygon_vertexes[i].first, polygon_vertexes[i].second);
                Point P2(polygon_vertexes[j].first, polygon_vertexes[j].second);
                // point on a line segment of polygon, see as inside the polygon
                if (OnLineSegment(P1, P2, point)) {
                    return true;
                }
                if ((Compare(P1.latitude - point.latitude) > 0 != Compare(P2.latitude - point.latitude)) &&
                    (Compare(point.longitude - (point.latitude - P1.latitude) * (P1.longitude - P2.longitude)
                                                             / (P1.latitude - P2.latitude) - P1.longitude) < 0)) {
                    flag = !flag;
                }
            }
            if (!flag) {
                return false;
            }
            return true;
        }
    }

    GeoIntegrationHandler::GeoIntegrationHandler(Space& _space, ConfigFileHandler& config_file_handler,
                                                 std::vector<Tweet>& tweet_corpus)
    : space(_space), tweet_list(tweet_corpus) {
        tweet_location_map = {};
        distance = config_file_handler.GetValue("distance", 1.0);
    }

    GeoIntegrationHandler::~GeoIntegrationHandler() {
        tweet_location_map.clear();
    }

    GeoIntegrationHandler& GeoIntegrationHandler::CreateTweetLocationMap() {
        for (auto& tweet: tweet_list) {
            int index = space.GetCellIndex(tweet.GetLongitude(), tweet.GetLatitude());
            if (tweet_location_map.find(index) == tweet_location_map.end()) {
                tweet_location_map[index] = {};
            }
            tweet_location_map[index].emplace_back(tweet);
        }
        return *this;
    }

    bool GeoIntegrationHandler::IsValidObject(OpenStreetMap& osm_object) {
        auto& coordinates = osm_object.GetCoordinates();
        for (auto& [longitude, latitude] : coordinates) {
            if (!space.ContainsPoint(longitude, latitude)) {
                return false;
            }
        }
        return true;
    }

    Space GeoIntegrationHandler::CreateMinimalBoundingRectangle(OpenStreetMap& osm_polygon_object) {
        using coordinate_type = std::pair<double, double>;
        std::vector<coordinate_type>& coordinates = osm_polygon_object.GetCoordinates();

        auto [southwest_longitude, northeast_longitude] = std::minmax_element(coordinates.begin(), coordinates.end(),
                                                       [](const coordinate_type &lhs, const coordinate_type &rhs) {
                                                           return lhs.first < rhs.first;
                                                       });
        auto [southwest_latitude, northeast_latitude] = std::minmax_element(coordinates.begin(), coordinates.end(),
                                                       [](const coordinate_type &lhs, const coordinate_type &rhs) {
                                                           return lhs.second < rhs.second;
                                                       });

        return Space({southwest_longitude->first, southwest_latitude->second,
                            northeast_longitude->first, northeast_latitude->second});
    }

    std::unordered_set<int> GeoIntegrationHandler::FindBelongingGrids(Space& min_bounding_rectangle) {
        Point northwest_corner = min_bounding_rectangle.GetNorthWestCorner();
        Point northeast_corner = min_bounding_rectangle.GetNorthEastCorner();
        Point southwest_corner = min_bounding_rectangle.GetSouthWestCorner();

        std::unordered_set<int> grids = {};
        int upper_left_index = space.GetCellIndex(northwest_corner.longitude, northwest_corner.latitude);
        int upper_right_index = space.GetCellIndex(northeast_corner.longitude, northeast_corner.latitude);
        int bottom_left_index = space.GetCellIndex(southwest_corner.longitude, southwest_corner.latitude);
        int horizon_grid_number = upper_right_index - upper_left_index + 1;
        int vertical_grid_number = (bottom_left_index - upper_left_index) / space.NumOfRows() + 1;
        for (int i = 0; i < horizon_grid_number; ++i) {
            for (int j = 0; j < vertical_grid_number; ++j) {
                grids.insert(upper_left_index + j * space.NumOfCols() + i);
            }
        }
        return grids;
    }

    std::vector<Tweet> GeoIntegrationHandler::FindCandidateTweetsForPolygon(OpenStreetMap& osm_polygon_object) {
        Space min_bounding_rectangle = CreateMinimalBoundingRectangle(osm_polygon_object);
        std::unordered_set<int> grid_set = FindBelongingGrids(min_bounding_rectangle);
        if (grid_set.empty()) {
            return {};
        }

        std::vector<Tweet> naive_candidates = {};
        for (int grid_index : grid_set) {
            if (tweet_location_map.find(grid_index) != tweet_location_map.end()) {
                auto& tweets = tweet_location_map[grid_index];
                for (auto& tweet : tweets) {
                    Point point(tweet.GetLongitude(), tweet.GetLatitude());
                    if (min_bounding_rectangle.ContainsPoint(point)) {
                        naive_candidates.emplace_back(tweet);
                    }
                }
            }
        }
        if (naive_candidates.empty()) {
            return naive_candidates;
        }

        // check if a tweet point locates inside an osm polygon
        std::vector<Tweet> final_candidates = {};
        std::vector<std::pair<double, double>>& polygon_vertexes = osm_polygon_object.GetCoordinates();
        for (Tweet& tweet : naive_candidates) {
            Point tweet_point(tweet.GetLongitude(), tweet.GetLatitude());
            if (detail::InPolygon(tweet_point, polygon_vertexes)) {
                final_candidates.emplace_back(tweet);
            }
        }

        return final_candidates;
    }

    std::unordered_set<int> GeoIntegrationHandler::FindBelongingGrids(std::vector<Point>& points) {
        std::unordered_set<int> grids = {};
        for (std::size_t i = 0; i < points.size() - 1; ++i) {
            Point& point1 = points[i];
            Point& point2 = points[i + 1];
            int cell_index1 = space.GetCellIndex(point1.longitude, point1.latitude);
            int cell_index2 = space.GetCellIndex(point2.longitude, point2.latitude);
            if (cell_index1 == cell_index2) {
                grids.insert(cell_index1);
                continue;
            }
            detail::BresenhamLine(point1, point2, this->space, grids);
        }
        return grids;
    }

    std::vector<Tweet> GeoIntegrationHandler::FindCandidateTweetsForLine(OpenStreetMap& osm_line_object) {
        auto& coordinates = osm_line_object.GetCoordinates();
        std::vector<Point> points = {};
        for (auto& [longitude, latitude] : coordinates) {
            points.emplace_back(longitude, latitude);
        }

        std::unordered_set<int> grid_set = FindBelongingGrids(points);
        std::vector<Tweet> naive_candidates = {};
        for (int grid_index : grid_set) {
            if (tweet_location_map.find(grid_index) != tweet_location_map.end()) {
                auto& tweets = tweet_location_map[grid_index];
                for (auto& tweet : tweets) {
                    naive_candidates.emplace_back(tweet);
                }
            }
        }
        if (naive_candidates.empty()) {
            return naive_candidates;
        }

        std::vector<Tweet> final_candidates = {};
        std::unordered_set<std::string> helper_set = {};

        for (std::size_t i = 0; i < points.size() - 1; ++i) {
            Point &point1 = points[i];
            Point &point2 = points[i + 1];

            // compute for each tweet the distance to a line segment
            for (Tweet& tweet : naive_candidates) {
                Point tweet_point(tweet.GetLongitude(), tweet.GetLatitude());
                std::vector<std::pair<double, double>> polygon_vertexes;

                double new_lat1 = point1.latitude - distance / DIST;
                polygon_vertexes.emplace_back(std::make_pair(point1.longitude, new_lat1)); // southwest_corner
                double new_lat2 = point1.latitude + distance / DIST;
                polygon_vertexes.emplace_back(std::make_pair(point1.longitude, new_lat2)); // northwest_corner
                double new_lat3 = point2.latitude + distance / DIST;
                polygon_vertexes.emplace_back(std::make_pair(point2.longitude, new_lat3)); // northeast_corner
                double new_lat4 = point2.latitude - distance / DIST;
                polygon_vertexes.emplace_back(std::make_pair(point2.longitude, new_lat4)); // southeast_corner
                if (detail::InPolygon(tweet_point, polygon_vertexes)) {
                    if (helper_set.find(tweet.GetTweetID()) == helper_set.end()) {
                        helper_set.insert(tweet.GetTweetID());
                        final_candidates.emplace_back(tweet);
                    }
                }
            }
        }
        return final_candidates;
    }
}
