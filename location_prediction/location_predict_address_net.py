import re
import io
import sys
import json
from addressnet.predict import predict_one

NAME_RELATED_CLASSES = {"building_name", "level_type", "flat_type", "street_name", "street_suffix", "street_type", "locality_name", "state"}
NUMBER_RELATED_CLASSES = {"level_number_prefix", "level_number", "level_number_suffix", "flat_number", "flat_number_prefix", "flat_number", "flat_number_suffix", "number_first_suffix", "number_first", "number_first_suffix", "number_last_prefix", "number_last", "number_last_suffix", "postcode"}

def parse_address_location(location_list):
    if not location_list:
        return []
    
    address_location_list = []
    for loc in location_list:
        if bool(re.search(r'\d', loc)):
            output = predict_one(loc)
            is_valid = True
            for key, value in output.items():
                if key in NAME_RELATED_CLASSES and bool(re.search(r'\d', value)):
                    is_valid = False
                    break
                if key in NUMBER_RELATED_CLASSES and bool(re.search(r'[A-Za-z]', value)):
                    is_valid = False
                    break
            
            if is_valid:
                address_location_list.append(loc)
    
    return address_location_list

def location_prediction(FILE_PATH="", OUTPUT_FILE_PATH=""):
    with io.open(OUTPUT_FILE_PATH, 'a+', encoding='utf-8') as output_f:
        with io.open(FILE_PATH, 'r', encoding='utf-8') as f:
            line_index = 0
            while True:
                line = f.readline()
                line_index = line_index + 1
                print("predict line: {}".format(line_index))
                if not line:
                    break
                
                tweet = json.loads(line)
                if tweet.get('has_address_name') != None and tweet['has_address_name'] == True:
                    address_location_list = parse_address_location(tweet['locations'])
                    if not address_location_list:
                        tweet['loc_address'] = []
                    else:
                        tweet['loc_address'] = address_location_list
                    
                output_f.write(json.dumps(tweet, ensure_ascii=False) + '\n')
                
if __name__ == "__main__":
    FILE_PATH = '/home/dietrich/master_thesis/GeoBurst_OSM/data/tweets_after_NER_predict.json'
    OUTPUT_FILE_PATH = '/home/dietrich/master_thesis/GeoBurst_OSM/data/tweets_after_address_net_predict.json'
    try:
        location_prediction(FILE_PATH, OUTPUT_FILE_PATH)           
        sys.exit(0)
    except:
        sys.exit(1)
        
