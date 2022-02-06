#pragma once

#include <vector>
#include <cmath>
#include <random>
#include <utility>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>

using point = boost::geometry::model::point<double, 2, boost::geometry::cs::geographic<boost::geometry::degree> >;

namespace common::geo_space {

    const double RADIUS = 0.005;
    const double DIST = 111.;
    double const EARTH_RADIUS = 6371.0;

	class Point {
    public:
		double longitude = 0.;
		double latitude = 0.;

		Point() {}

		Point(double lon, double lat) : longitude(lon), latitude(lat) {}

        Point(const Point& _point) {
            this->longitude = _point.longitude;
            this->latitude = _point.latitude;
        }

        Point& operator=(const Point& _point) {
            if (this == &_point) {
                return *this;
            }
            Point temp_point = _point;
            std::swap(*this, temp_point);
            return *this;
        }

        Point(Point&& _point) noexcept {
            this->longitude = std::exchange(_point.longitude, 0.);
            this->latitude = std::exchange(_point.latitude, 0.);
        }

        Point& operator=(Point&& _point) noexcept {
            if (this == &_point) {
                return *this;
            }
            this->longitude = std::exchange(_point.longitude, 0.);
            this->latitude = std::exchange(_point.latitude, 0.);
            return *this;
        }

        Point operator+(const Point& _point) const {
            return {longitude + _point.longitude, latitude + _point.latitude};
        }

        Point operator-(const Point& _point) const {
            return {longitude - _point.longitude, latitude - _point.latitude};
        }

        // dot product
        double operator*(const Point& _point) const {
            return longitude * _point.longitude + latitude * _point.latitude;
        }

        // cross product
        double operator^(const Point& _point) const {
            return longitude * _point.latitude - _point.longitude * latitude;
        }
	};

	struct BoundingBox {
		Point southwest_corner;
		Point northeast_corner;

		BoundingBox() {}

		BoundingBox(std::vector<double>& coordinates_list) {
			if (!coordinates_list.empty() && coordinates_list.size() == 4) {
				southwest_corner.longitude = coordinates_list[0];
				southwest_corner.latitude = coordinates_list[1];
				northeast_corner.longitude = coordinates_list[2];
				northeast_corner.latitude = coordinates_list[3];
			}
		}

        BoundingBox(std::vector<double>&& coordinates_list) {
            if (!coordinates_list.empty() && coordinates_list.size() == 4) {
                southwest_corner.longitude = coordinates_list[0];
                southwest_corner.latitude = coordinates_list[1];
                northeast_corner.longitude = coordinates_list[2];
                northeast_corner.latitude = coordinates_list[3];
            }
        }

		BoundingBox(Point& sc, Point& nc) {
			southwest_corner.longitude = sc.longitude;
			southwest_corner.latitude = sc.latitude;
			northeast_corner.longitude = nc.longitude;
			northeast_corner.latitude = nc.latitude;
		}
	};

	// define a geographical space G grouping from bounding-box
	class Space {
	private:
		double cell_size = 1.; // km resolution
		BoundingBox bounding_box;

	public:
		Space() {}

		Space(std::vector<double>& coordinates_list) 
			: bounding_box(coordinates_list){}

        Space(std::vector<double>&& coordinates_list)
                : bounding_box(std::move(coordinates_list)){}

		Space(Point& sc, Point& nc)
			: bounding_box(sc, nc){}

		Space(std::vector<double>& coordinates_list, double _cell_size)
			: cell_size(_cell_size), bounding_box(coordinates_list) {}

        Space(std::vector<double>&& coordinates_list, double _cell_size)
                : cell_size(_cell_size), bounding_box(std::move(coordinates_list)) {}

		Space(Point& sc, Point& nc, double _cell_size)
			: cell_size(_cell_size), bounding_box(sc, nc) {}

	public:
		Point GetSouthWestCorner();

		Point GetSouthEastCorner();

		Point GetNorthEastCorner();

		Point GetNorthWestCorner();

		double GetLength(); // distance between two longitudes (km)

		double GetWidth(); // distance between two latitudes (km)

		void SetCellSize(double size) {
			cell_size = size;
			return ;
		}

		int NumOfCells() {
			double length = GetLength();
			double width = GetWidth();
			return (int)((int)(length + 1) * (int)(width + 1) / pow(cell_size, 2));
		}

        int NumOfRows() {
            double width = GetWidth() + 1;
            return (int)(width / cell_size);
        }

        int NumOfCols() {
            double length = GetLength() + 1;
            return (int)(length / cell_size);
        }

        int GetCellIndex(double longitude, double latitude);

        Point GetCentralLocationOfCell(int cell_index);

        double Distance(Point& lhs, Point& rhs);

        bool ContainsPoint(Point& point);

        bool ContainsPoint(double longitude, double latitude);

        Point ReGenerateCoordinates(Point& point);
	};
}
