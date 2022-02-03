import requests
import json

URL = 'https://api.ohsome.org/v1/contributions/latest/geometry'
data = {"bboxes": "-95.575128, 29.534661, -95.165277, 29.893392", "time": "2017-08-27,2017-09-10", "showMetadata": "yes", "properties": "metadata,tags"}

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


def parse_building(raw_objects, f):
    for obj in raw_objects:
        if obj['geometry'] is None or obj['geometry'] == 'None':
            continue
        if obj['geometry']['type'] != 'Polygon':
            continue
        if len(obj['geometry']['coordinates']) != 1:
            continue
        
        print(obj)
        coordinates = obj['geometry']['coordinates']
        tags = {}
        osm_id = obj['properties']['@osmId']
        if osm_id_set in osm_id_set:
            continue
        else:
            osm_id_set.add(osm_id)
        for key, value in obj['properties'].items():
            if key == '@timestamp':
                tags[key] = value
            if key[0] != '@':
                tags[key] = value
        record = {"osm_id": osm_id, "coordinates": coordinates, "tags": tags}
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
        tags = {}
        osm_id = obj['properties']['@osmId']
        if osm_id_set in osm_id_set:
            continue
        else:
            osm_id_set.add(osm_id)
        for key, value in obj['properties'].items():
            if key == '@timestamp':
                tags[key] = value
            if key[0] != '@':
                tags[key] = value
        record = {"osm_id": osm_id, "coordinates": coordinates, "tags": tags}
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
    