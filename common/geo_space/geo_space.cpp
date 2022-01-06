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
        return Distance(southwest_corner, southeast_corner);
	}

	double Space::GetWidth() {
		Point southwest_corner = GetSouthWestCorner();
		Point northwest_corner = GetNorthWestCorner();
        return Distance(southwest_corner, northwest_corner);
	}

    int Space::GetCellIndex(double longitude, double latitude) {
		Point northwest_corner = GetNorthWestCorner();
        Point point1(longitude, northwest_corner.latitude);
		double col = Distance(northwest_corner, point1);
        Point point2(northwest_corner.longitude, latitude);
		double row = Distance(northwest_corner,point2);
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
        cell_bounding_box.push_back(southwest_corner.longitude + long_index * cell_size);
        cell_bounding_box.push_back(northwest_corner.latitude - (lat_index + 1) * cell_size);
        cell_bounding_box.push_back(southwest_corner.longitude + (long_index + 1) * cell_size);
        cell_bounding_box.push_back(northwest_corner.latitude - lat_index * cell_size);

        Point rtn((cell_bounding_box[0] + cell_bounding_box[2]) / 2, (cell_bounding_box[1] + cell_bounding_box[3]) / 2);
        if (!ContainsPoint(rtn)) {
            if (rtn.longitude >= bounding_box.northeast_corner.longitude) {
                rtn.longitude = bounding_box.northeast_corner.longitude;
            }
            if (rtn.latitude <= bounding_box.southwest_corner.latitude) {
                rtn.latitude = bounding_box.southwest_corner.latitude;
            }
        }
        return rtn;
    }

    using spherical_point = boost::geometry::model::point<
            double, 2, boost::geometry::cs::spherical_equatorial<boost::geometry::degree> >;

    double Space::Distance(Point& lhs, Point& rhs) {
        return boost::geometry::distance(spherical_point(lhs.longitude, lhs.latitude),
                                         spherical_point(rhs.longitude, rhs.latitude)) * EARTH_RADIUS;
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

    Point Space::ReGenerateCoordinates(Point& point) {
        double centroid_lon = point.longitude;
        double centroid_lat = point.latitude;

        std::mt19937 random_generator{std::random_device{}()};
        std::uniform_real_distribution<double> random_number{0., 1.};
        double distance = RADIUS * sqrt(random_number(random_generator));
        double theta = random_number(random_generator) * (2 * M_PI);
        double new_lon = distance * cos(theta) / DIST + centroid_lon;
        double new_lat = distance * sin(theta) / DIST + centroid_lat;
        return Point{new_lon, new_lat};
    }
}