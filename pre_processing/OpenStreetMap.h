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

        OpenStreetMap& operator=(const OpenStreetMap& lhs) {
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
            osm_id = std::exchange(rhs.osm_id, "");
            create_time = std::exchange(rhs.create_time, "");
            osm_type = std::exchange(rhs.osm_type, "");
            coordinates = std::exchange(rhs.coordinates, {});
            tags = std::exchange(rhs.tags, {});
            grid_idx = std::exchange(rhs.grid_idx, 0);
        }

        OpenStreetMap& operator=(OpenStreetMap&& rhs) {
            if (&rhs == this) {
                return *this;
            }

            osm_id = std::exchange(rhs.osm_id, "");
            create_time = std::exchange(rhs.create_time, "");
            osm_type = std::exchange(rhs.osm_type, "");
            coordinates = std::exchange(rhs.coordinates, {});
            tags = std::exchange(rhs.tags, {});
            grid_idx = std::exchange(rhs.grid_idx, 0);
            return *this;
        }

        void SetOSMID(const std::string& _id) {
            this->osm_id = _id;
        }

        void SetOSMID(std::string&& _id) {
            this->osm_id = std::move(_id);
        }

        std::string& GetOSMID() {
            return this->osm_id;
        }

        void SetCreateTime(const std::string& _create_time) {
            this->create_time = _create_time;
        }

        void SetCreateTime(std::string&& _create_time) {
            this->create_time = std::move(_create_time);
        }

        void SetOSMType(const std::string& _type) {
            this->osm_type = _type;
        }

        void SetOSMType(std::string&& _type) {
            this->osm_type = std::move(_type);
        }

        std::string& GetOSMType() {
            return this->osm_type;
        }

        void SetCoordinates(const std::vector<std::pair<double, double> >& _coordinates) {
            this->coordinates = _coordinates;
        }

        void SetCoordinates(std::vector<std::pair<double, double> >&& _coordinates) {
            this->coordinates = std::move(_coordinates);
        }

        std::vector<std::pair<double, double> >& GetCoordinates() {
            return this->coordinates;
        }

        void SetTags(const std::unordered_map<std::string, std::string>& _tags) {
            this->tags = _tags;
        }

        void SetTags(std::unordered_map<std::string, std::string>&& _tags) {
            this->tags = std::move(_tags);
        }

        std::unordered_map<std::string, std::string>& GetTags() {
            return this->tags;
        }

        void SetGridIndex(int index) {
            this->grid_idx = index;
        }

        int GetGridIndex() {
            return this->grid_idx;
        }
    };
}
#endif //GEOBURST_OSM_OPENSTREETMAP_H

