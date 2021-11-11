import re
import sys
import json
import time
import spacy
import numpy as np
import geonamescache
import haversine as hs
from enum import Enum
from datetime import datetime
from functools import cmp_to_key
from geopy.geocoders import Nominatim

class Disambiguity(object):
    def __init__(self):
        self.nlp = spacy.load('en_core_web_sm')
        self.gc = geonamescache.GeonamesCache()
        self.cities = self.gc.get_cities()
        self.states = self.gc.get_us_states()
        
        def generate_geo_political_entity_set(cities, states):
            # gets nested dictionary for cities    
            def gen_dict_extract(var, key):
                if isinstance(var, dict):
                    for k, v in var.items():
                        if k == key:
                            yield v
                        if isinstance(v, (dict, list)):
                            yield from gen_dict_extract(v, key)
                elif isinstance(var, list):
                    for d in var:
                        yield from gen_dict_extract(d, key)
            
            city_entities_set = {*gen_dict_extract(cities, 'name')}
            states_entities_set = {*gen_dict_extract(states, 'name')}     
            return city_entities_set, states_entities_set
                        
        self.city_entities_set, self.states_entities_set = generate_geo_political_entity_set(self.cities, self.states)
            
    def parse_geo_political_entity(self, tweet):
        # find the names of higher administrative units in context
        success_parse_GPE = False
        unordered_GPE_set = set()
        if 'HTX' in tweet['context']:
            unordered_GPE_set.add('Houston')
            unordered_GPE_set.add('Texas')
	
        doc = self.nlp(tweet['context'])
        if doc.ents:
            for entity in doc.ents:
                text = entity.text
                label = entity.label_ 
                if label == 'GPE' and (text).lower() != 'Harvey'.lower():
                    if text in self.city_entities_set or text in self.states_entities_set:
                        unordered_GPE_set.add(entity.text)
	 
	 # not contain a GPE          
        if not unordered_GPE_set:
            return success_parse_GPE
	
        class GPE_LEVEL(Enum):
            CITY = 1
            STATE = 2
	    
            def __lt__(self, other):
                if self.__class__ is other.__class__:
                    return self.value < other.value
                return NotImplemented
                
        def cmp(x, y):
            lhs = GPE_LEVEL.CITY
            rhs = GPE_LEVEL.STATE
            if x in self.city_entities_set:
                lhs = GPE_LEVEL.CITY
            else:
                lhs = GPE_LEVEL.STATE
            if y in self.city_entities_set:
                rhs = GPE_LEVEL.CITY
            else:
                rhs = GPE_LEVEL.STATE
            return 1 if lhs > rhs else -1 if lhs < rhs else 0
	
        GPE_set = sorted(unordered_GPE_set, key=cmp_to_key(cmp))
        geo_political_entity = ""
        for gpe in GPE_set:
            geo_political_entity = geo_political_entity + gpe + ", "
	    
        if not tweet['has_address_name']:    
            # only GPE in NER_loc, not valuable information
            if len(tweet['loc_NER']) == 1 and tweet['loc_NER'][0] in unordered_GPE_set:
                return success_parse_GPE	
            for loc in tweet['loc_NER']:
                if loc not in GPE_set:
                    tweet['locations'].append(loc + ", " + geo_political_entity[:-2])
                    success_parse_GPE = True
                    tweet['has_GPE'] = True
        else:
            for loc in tweet['loc_address']:   
                tweet['locations'].append(loc + ", " + geo_political_entity[:-2])
                success_parse_GPE = True	
                tweet['has_GPE'] = True    
        return success_parse_GPE
	
	
    def disambiguity_process(self, tweet):
        # 1. check if there exists the names of higher administrative units in context
        if tweet.get('loc_NER') != None:
            if not tweet['loc_NER']:
                tweet['can_predict'] = False # no NER location extracted
                return
            else:
                success_parse_GPE = self.parse_geo_political_entity(tweet)
        elif tweet.get('loc_address') != None:
            if not tweet['loc_address']:
                tweet['can_predict'] = False # no address location extracted
                return
            else:
                success_parse_GPE = self.parse_geo_political_entity(tweet)
                    
        # 2. Those don't have GPE mentioned need further consideration
        
        if not success_parse_GPE:
            if tweet.get('loc_NER') != None and tweet['loc_NER']:
                tweet['locations'] = tweet['loc_NER']
            elif tweet.get('loc_address') != None and tweet['loc_address']:
                tweet['locations'] = tweet['loc_address']
            tweet['has_GPE'] = False
                
        return
        
class LocationPredictor(object):
    def __init__(self):
        self.geolocator = Nominatim(user_agent="GeoBurst")
        self.request_rate = 1.
        self.start_time = time.time()
        self.end_time = time.time()
        self.state = 'Texas'
        self.bounding_box = [-95.565128, 29.544661, -95.185277, 29.883392]  # Houston City
        self.width = hs.haversine((self.bounding_box[1], self.bounding_box[0]), (self.bounding_box[1], self.bounding_box[2]))
        self.height = hs.haversine((self.bounding_box[1], self.bounding_box[0]), (self.bounding_box[3], self.bounding_box[0]))
        
    def request_geolocator(self, loc_str):
        self.start_time = time.time()
        location = self.geolocator.geocode(loc_str)
        self.end_time = time.time()  
        if self.end_time - self.start_time < self.request_rate:
            time.sleep(1)         
        return location
        
    def is_valid_location(self, longitude, latitude):
        is_valid = False
        is_longitude_valid = False
        is_latitude_valid = False
        
        if longitude < self.bounding_box[2] and longitude > self.bounding_box[0]:
             is_longitude_valid = True   
        if latitude < self.bounding_box[3] and latitude > self.bounding_box[1]:
            is_latitude_valid = True
            
        if is_longitude_valid and is_latitude_valid:
            is_valid = True
        else:
            centroid_longitude = (self.bounding_box[0] + self.bounding_box[2]) / 2.
            centroid_latitude = (self.bounding_box[1] + self.bounding_box[3]) / 2.
            if hs.haversine((centroid_latitude, centroid_longitude), (latitude, longitude)) < np.sqrt((2 * self.width)**2 + (2 * self.height)**2):
                is_valid = True
        return is_valid
            
    def predict(self, tweet):
        if tweet.get('locations') == None or not tweet['locations']:
            return []

        tweet_list = []           
        for i, loc in enumerate(tweet['locations']):
            if loc == "":
                continue
            if tweet['has_GPE'] == False:
                loc = loc + ", " + self.state
            location = self.request_geolocator(loc)
            if location and location.longitude and location.latitude and self.is_valid_location(location.longitude, location.latitude):
                tweet_temp = tweet
                tweet_temp["tweet_id"] = tweet["tweet_id"] + str(i)
                tweet_temp["longitude"] = location.longitude
                tweet_temp["latitude"] = location.latitude
                tweet_list.append(tweet_temp)
        return tweet_list
                    
                    
def generate_final_data(input_file, output_file, merged_tweet_list):    
    num_lines_merged = len(merged_tweet_list)
    int idx = 0
    for line_raw in input_file:
        tweet_raw = json.loads(line_raw)
        time_raw = datetime.strptime(tweet_raw['time'], '%Y-%m-%d %H:%M:%S')
        
        if idx < num_lines_merged:
            tweet_merged = merged_tweet_list[idx]
            time_merged = datetime.strptime(tweet_merged['time'], '%Y-%m-%d %H:%M:%S')
            
            if time_merged > time_raw:
                output_file.write(json.dumps(tweet_raw, ensure_ascii=False) + '\n')
            elif time_merged == time_raw:
                while time_merged == time_raw:
                    output_file.write(json.dumps(tweet_merged, ensure_ascii=False) + '\n') 
                    idx = idx + 1
                    if idx >= num_lines_merged:
                        break
                    tweet_merged = merged_tweet_list[idx]
                    time_merged = datetime.strptime(tweet_merged['time'], '%Y-%m-%d %H:%M:%S')
        else:
            output_file.write(json.dumps(tweet_raw, ensure_ascii=False) + '\n')      
    return
                    
if __name__ == "__main__":
    RAW_DATA_FILE = '/home/dietrich/master_thesis/GeoBurst_OSM/data/tweets_Houston.json'
    FILE_PATH = '/home/dietrich/master_thesis/GeoBurst_OSM/data/tweets_after_address_net_predict.json'
    OUTPUT_FILE_PATH = '/home/dietrich/master_thesis/GeoBurst_OSM/data/tweets_after_location_prediction.json'
    try:        
        with open(OUTPUT_FILE_PATH, 'a+', encoding='utf-8') as output_f:
            tweet_list = []
            with open(FILE_PATH, 'r', encoding='utf-8') as f:
                line_index = 0
                disambiguity = Disambiguity()
                predictor = LocationPredictor()
                while True:
                    line = f.readline()
                    line_index = line_index + 1
                    print("predict line: {}".format(line_index))
                    if not line:
                        break
                    tweet = json.loads(line)
                    tweet['locations'].clear()
                    tweet['can_predict'] = True

                    disambiguity.disambiguity_process(tweet)
                    if not tweet['can_predict'] or tweet['locations'] == []:
                        continue
                    tweets = predictor.predict(tweet)
                    if not tweets:
                        tweet['can_predict'] = False
                        tweet_list.append(tweet)
                    else:
                        for tweet_gen in tweets:
                            tweet_list.append(tweet_gen) 
            f.close()                   
                      
            f = open(RAW_DATA_FILE, 'r', encoding='utf-8')
            generate_final_data(f, output_f, tweet_list)       
        sys.exit(0)         
    except:
        sys.exit(1)
