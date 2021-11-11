import sys
import re
import json
import nltk
from nltk.tokenize import word_tokenize 
from nltk.tag.stanford import StanfordNERTagger

def parse_tagged_location(tagged):
    if not tagged:
        return []

    location_list = []
    loc_name = ""

    index = 0
    while index < len(tagged):
        while index < len(tagged) and (tagged[index][1] == 'LOCATION' or tagged[index][1] == 'ORGANIZATION'):
            item = tagged[index]
            loc_name = loc_name + item[0] + ' '
            index = index + 1
        if loc_name:
            location_list.append(loc_name[:-1])
        loc_name = ""
        index = index + 1

    return location_list
        
def check_address_name(loc_list):
    if not loc_list:
        return false
    
    for loc in loc_list:
        if bool(re.search(r'\d', loc)):
            return True
    return False

def location_prediction(tagger, FILE_PATH="", OUTPUT_FILE_PATH=""):
    with open(OUTPUT_FILE_PATH, 'a+', encoding='utf-8') as output_f:
        with open(FILE_PATH, 'r', encoding='utf-8') as f:
            line_index = 0
            while True:
                line = f.readline()
                line_index = line_index + 1
                print("predict line: {}".format(line_index))
                if not line:
                    break
                if 'NASAs' in  tweet['context']:
                    continue
                has_address_name = False
                tweet = json.loads(line)
                reference_loc_list = tweet["locations"]
                # check weather tweet contains address name (e.g. contains numbers)
                has_address_name = check_address_name(reference_loc_list)
                if not has_address_name:
                    tweet['has_address_name'] = False
                    tagged = tagger.tag(word_tokenize(tweet['context']))
                    NER_location_list = parse_tagged_location(tagged)
                    tweet['loc_NER'] = NER_location_list
                else:
                    tweet['has_address_name'] = True
                
                output_f.write(json.dumps(tweet, ensure_ascii=False) + '\n')
                
if __name__ == "__main__":
    PATH_TO_JAR = '/home/dietrich/stanford-ner-4.2.0/stanford-ner-2020-11-17/stanford-ner-4.2.0.jar'
    PATH_TO_MODEL = '/home/dietrich/stanford-ner-4.2.0/stanford-ner-2020-11-17/classifiers/english.all.3class.distsim.crf.ser.gz'
    FILE_PATH = '/home/dietrich/master_thesis/GeoBurst_OSM/data/tweets_need_predict_loc.json'
    OUTPUT_FILE_PATH = '/home/dietrich/master_thesis/GeoBurst_OSM/data/tweets_after_NER_predict.json'

    tagger = StanfordNERTagger(model_filename=PATH_TO_MODEL,path_to_jar=PATH_TO_JAR, encoding='utf-8')
    
    try:
        location_prediction(tagger, FILE_PATH, OUTPUT_FILE_PATH)           
        sys.exit(0)
    except:
        sys.exit(1)
                
