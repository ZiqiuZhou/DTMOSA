#include "geo_space.h"

namespace common::geo_space {

	Point Space::GetSouthWestCorner() {
		return bounding_box.southwest_corner;
	}

	Point Space::GetSouthEastCorner() {
		Point southeast_corner;
		southeast_corner.longitude = bounding_box.northeast_corner.longitude;
		southeast_corner.latitude = bounding_box.southwest_corner.latitude;		
		return southeast_corner;
	}

	Point Space::GetNorthEastCorner() {
		return bounding_box.northeast_corner;
	}

	Point Space::GetNorthWestCorner() {
		Point northwest_corner;
		northwest_corner.longitude = bounding_box.southwest_corner.longitude;
		northwest_corner.latitude = bounding_box.northeast_corner.latitude;
		return northwest_corner;
	}

	double Space::GetLength() {
		Point southwest_corner = GetSouthWestCorner();
		Point southeast_corner = GetSouthEastCorner();
		return boost::geometry::distance(
			point(southwest_corner.longitude,
				  southwest_corner.latitude),
			point(southeast_corner.longitude,
				  southeast_corner.latitude)) / 1e3;
	}

	double Space::GetWidth() {
		Point southwest_corner = GetSouthWestCorner();
		Point northwest_corner = GetNorthWestCorner();
		return boost::geometry::distance(
			point(southwest_corner.longitude,
				southwest_corner.latitude),
			point(northwest_corner.longitude,
				northwest_corner.latitude)) / 1e3;
	}

	std::pair<int, int> Space::GetCellIndex(double longitude, double latitude) {
		Point northwest_corner = GetNorthWestCorner();
		double col = boost::geometry::distance(point(northwest_corner.longitude, northwest_corner.latitude),
											   point(longitude, northwest_corner.latitude)) / 1e3;
		double row = boost::geometry::distance(point(northwest_corner.longitude, northwest_corner.latitude),
											   point(northwest_corner.longitude, latitude)) / 1e3;
		return std::make_pair(row / cell_size, col / cell_size);
	}
}