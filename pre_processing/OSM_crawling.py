import requests
import json
import pyproj
import shapely
import shapely.ops as ops
from shapely.geometry.polygon import Polygon
from functools import partial

URL = 'https://api.ohsome.org/v1/contributions/latest/geometry'
data = {"bboxes": "-95.575128, 29.534661, -95.165277, 29.893392", "time": "2017-09-03,2017-09-10", "showMetadata": "yes", "properties": "metadata,tags"}

osm_id_set = set()


def crawl_process(filter_data, f, building_flag):
    # request data
    request_data = data.copy()
    request_data['filter'] = filter_data
    response = requests.post(URL, data=request_data)
    response = response.json()
    if 'features' in response and isinstance(response['features'], list) and response['features']:
        raw_objects = response['features']
    
        if building_flag:
            parse_building(raw_objects, f)
        else:
            parse_road(raw_objects, f)
    return


def polygon_area(coordinates):
    geom = Polygon(coordinates)
    geom_area = ops.transform(
        partial(
            pyproj.transform,
            pyproj.Proj(init='EPSG:4326'),
            pyproj.Proj(
                proj='aea',
                lat_1=geom.bounds[1],
                lat_2=geom.bounds[3])), geom)
    return geom_area.area


def parse_building(raw_objects, f):
    for obj in raw_objects:
        if obj['geometry'] is None or obj['geometry'] == 'None':
            continue
        if obj['geometry']['type'] != 'Polygon':
            continue
        if len(obj['geometry']['coordinates']) != 1:
            continue
        
        print(obj)
        coordinates = obj['geometry']['coordinates'][0]
        if polygon_area(coordinates) < 1000:
            continue
        tags = {}
        osm_id = obj['properties']['@osmId']
        if osm_id in osm_id_set:
            continue
        else:
            osm_id_set.add(osm_id)
        create_time = ""
        for key, value in obj['properties'].items():
            if key == '@timestamp':
                create_time = value
            if key[0] != '@':
                tags[key] = value
        record = {"osm_id": osm_id, "timestamp": create_time, "type": obj['geometry']['type'],
                  "coordinates": coordinates, "tags": tags}
        f.write(json.dumps(record, ensure_ascii=False) + '\n')
    return


def parse_road(raw_objects, f):
    for obj in raw_objects:
        if obj['geometry'] is None or obj['geometry'] == 'None':
            continue
        if obj['geometry']['type'] != 'LineString':
            continue
        if len(obj['geometry']['coordinates']) == 0:
            continue
        
        print(obj)
        coordinates = obj['geometry']['coordinates']
        if len(coordinates) < 5:
            continue
        tags = {}
        osm_id = obj['properties']['@osmId']
        if osm_id in osm_id_set:
            continue
        else:
            osm_id_set.add(osm_id)
        create_time = ""
        for key, value in obj['properties'].items():
            if key == '@timestamp':
                create_time = value
            if key[0] != '@':
                tags[key] = value
        record = {"osm_id": osm_id, "timestamp": create_time, "type": obj['geometry']['type'],
                  "coordinates": coordinates, "tags": tags}
        f.write(json.dumps(record, ensure_ascii=False) + '\n')


if __name__ == "__main__":
    filter_buildings = "building=* and geometry:polygon"
    filter_roads = "highway=* and type:way"
    
    file_building_path = "/home/dietrich/master_thesis/GeoBurst_OSM/data/osm_buildings.json"
    file_road_path = "/home/dietrich/master_thesis/GeoBurst_OSM/data/osm_roads.json"
    file_building = open(file_building_path, 'a', encoding="utf-8")
    file_road = open(file_road_path, 'a', encoding="utf-8")
    crawl_process(filter_buildings, file_building, True)
    crawl_process(filter_roads, file_road, False)

    print("finished")
    
