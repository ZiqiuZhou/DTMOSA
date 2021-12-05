#pragma once

#include <vector>
#include <cmath>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>

using point = boost::geometry::model::point<double, 2, boost::geometry::cs::geographic<boost::geometry::degree> >;

namespace common::geo_space {

	struct Point {
		double longitude = 0.;
		double latitude = 0.;

		Point() {}

		Point(double lon, double lat) : longitude(lon), latitude(lat) {}
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
		double cell_size = 0.; // km resolution
		BoundingBox bounding_box;

	public:
		Space() {}

		Space(std::vector<double>& coordinates_list) 
			: bounding_box(coordinates_list){}

		Space(Point& sc, Point& nc)
			: bounding_box(sc, nc){}

		Space(std::vector<double>& coordinates_list, double _cell_size)
			: cell_size(_cell_size), bounding_box(coordinates_list) {}

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
			return (int)(length * width / pow(cell_size, 2));
		}

        int NumOfRows() {
            double width = GetWidth();
            return (int)(width / cell_size);
        }

        int NumOfCols() {
            double length = GetLength();
            return (int)(length / cell_size);
        }

        int GetCellIndex(double longitude, double latitude);

        double Distance(Point& lhs, Point& rhs);
	};
}
