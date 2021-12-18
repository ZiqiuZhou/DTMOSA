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

    int Space::GetCellIndex(double longitude, double latitude) {
		Point northwest_corner = GetNorthWestCorner();
		double col = boost::geometry::distance(point(northwest_corner.longitude, northwest_corner.latitude),
											   point(longitude, northwest_corner.latitude)) / 1e3;
		double row = boost::geometry::distance(point(northwest_corner.longitude, northwest_corner.latitude),
											   point(northwest_corner.longitude, latitude)) / 1e3;
        int row_idx = (int)(row / cell_size);
        int col_idx = (int)(col / cell_size);
		return row_idx * NumOfCols() + col_idx;
	}

    Point Space::GetCentralLocationOfCell(int cell_index) {
        std::vector<double> cell_bounding_box;
        int long_index = cell_index % NumOfCols();
        int lat_index = cell_index / NumOfCols();
        Point southwest_corner = GetSouthWestCorner();
        Point southeast_corner = GetSouthEastCorner();
        Point northwest_corner = GetNorthWestCorner();
        double col_step = (southeast_corner.longitude - southwest_corner.longitude) / NumOfCols();
        double row_step = (northwest_corner.latitude - southwest_corner.latitude) / NumOfRows();
        cell_bounding_box.push_back(southwest_corner.longitude + long_index * col_step);
        cell_bounding_box.push_back(northwest_corner.latitude - (lat_index + 1) * row_step);
        cell_bounding_box.push_back(southwest_corner.longitude + (long_index + 1) * col_step);
        cell_bounding_box.push_back(northwest_corner.latitude - lat_index * col_step);

        Point rtn((cell_bounding_box[0] + cell_bounding_box[2]) / 2, (cell_bounding_box[1] + cell_bounding_box[3]) / 2);
        return rtn;
    }

    double Space::Distance(Point& lhs, Point& rhs) {
        return boost::geometry::distance(point(lhs.longitude, lhs.latitude),
                                         point(rhs.longitude, lhs.latitude)) / 1e3;
    }

    bool Space::ContainsPoint(Point& point) {
        double& longitude = point.longitude;
        double& latitude = point.latitude;
        Point& southwest_corner = bounding_box.southwest_corner;
        Point& northeast_corner = bounding_box.northeast_corner;
        if (longitude >= southwest_corner.longitude && longitude <= northeast_corner.longitude &&
            latitude >= southwest_corner.latitude && latitude <= northeast_corner.latitude) {
            return true;
        }
        return false;
    }
}