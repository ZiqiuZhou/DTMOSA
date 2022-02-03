//
// Created by dietrich on 03.02.22.
//

#ifndef GEOBURST_OSM_OPENSTREETMAP_H
#define GEOBURST_OSM_OPENSTREETMAP_H

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

namespace PreProcessing::OpenStreetMapParser {

    class OpenStreetMap {
    private:
        std::string osm_id = "";

        std::string create_time = "";

        std::string osm_type = "";

        std::vector<std::pair<double, double> > coordinates;

        std::unordered_map<std::string, std::string> tags;

        int grid_idx = 0;

    public:
        OpenStreetMap() {
            coordinates = {};
            tags = {};
        }

        ~OpenStreetMap() {
            coordinates.clear();
            tags.clear();
        }

        OpenStreetMap(const OpenStreetMap& lhs) {
            osm_id = lhs.osm_id;
            create_time = lhs.create_time;
            osm_type = lhs.osm_type;
            coordinates = lhs.coordinates;
            tags = lhs.tags;
            grid_idx = lhs.grid_idx;
        }

        OpenStreetMap& OpenStreetMap(const OpenStreetMap& lhs) {
            if (&lhs == this) {
                return *this;
            }

            osm_id = lhs.osm_id;
            create_time = lhs.create_time;
            osm_type = lhs.osm_type;
            coordinates = lhs.coordinates;
            tags = lhs.tags;
            grid_idx = lhs.grid_idx;
            return *this;
        }

        OpenStreetMap(OpenStreetMap&& rhs) {
            osm_id = std::exchange(lhs.osm_id, "");
            create_time = std::exchange(lhs.create_time, "");
            osm_type = std::exchange(lhs.osm_type, "");
            coordinates = std::exchange(lhs.coordinates, {});
            tags = std::exchange(lhs.tags, {});
            grid_idx = std::exchange(lhs.grid_idx, 0);
        }

        OpenStreetMap& OpenStreetMap(OpenStreetMap&& rhs) {
            if (&rhs == this) {
                return *this;
            }

            osm_id = std::exchange(lhs.osm_id, "");
            create_time = std::exchange(lhs.create_time, "");
            osm_type = std::exchange(lhs.osm_type, "");
            coordinates = std::exchange(lhs.coordinates, {});
            tags = std::exchange(lhs.tags, {});
            grid_idx = std::exchange(lhs.grid_idx, 0);
            return *this;
        }

        void SetOSMID(std::string& _id) {
            this->osm_id = _id;
        }

        void SetCreateTime(std::string& _create_time) {
            this->create_time = _create_time;
        }

        void SetOSMType(std::string& _type) {
            this->osm_type = _type;
        }

        void SetCoordinates(std::vector<std::pair<double, double> >& _coordinates) {
            this->coordinates = _coordinates;
        }

        void SetCoordinates(std::vector<std::pair<double, double> >&& _coordinates) {
            this->coordinates = std::move(_coordinates);
        }

        void SetTags(std::unordered_map<std::string, std::string>& _tags) {
            this->tags = _tags;
        }

        void SetTags(std::unordered_map<std::string, std::string>&& _tags) {
            this->tags = std::move(_tags);
        }

        void SetGridIndex(int index) {
            this->grid_idx = index;
        }
    };
}
#endif //GEOBURST_OSM_OPENSTREETMAP_H

