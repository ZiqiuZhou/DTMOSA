import re
import requests
import json
import time
from datetime import datetime
import random
import nltk
from nltk.corpus import stopwords, wordnet
from nltk.tokenize import word_tokenize
from nltk.stem import WordNetLemmatizer
    
    
class PipeLine(object):
    """PipeLine is a class to do pre-processing work for tweet context .
    """

    def __init__(self, lemmatizer):
        self.lemmatizer = lemmatizer

    def lower_case(self, context):
        context = context.lower()
        return context

    def url_remove(self, context):
        context = re.sub(r"http\S+", "", context)
        return context

    def punctuation_remove(self, context):
        context = re.sub(r'[^\w\s]', '', context)
        return context

    def numbers_remove(self, context):
        context = re.sub(r"[0-9]", "", context)
        return context

    def tokenize(self, context):
        context = context.replace("\n", "")
        word_bag = word_tokenize(context)
        return word_bag

    def stop_words_remove(self, word_bag):
        stop_words_list = stopwords.words("english")
        stop_words_set = set(stop_words_list)
        word_bag = [word for word in word_bag if word != "" and (word not in stop_words_set) and len(word) > 3]
        return word_bag

    def remove_multiple_whitespace(self, word):
        return re.sub(' +', ' ', word).strip()

    def get_wordnet_pos(self, word):
        pos_tag= nltk.pos_tag([word])[0][1][0].upper()
        tag_dict = {"J": wordnet.ADJ,
                    "N": wordnet.NOUN,
                    "V": wordnet.VERB,
                    "R": wordnet.ADV}
        return tag_dict.get(pos_tag, wordnet.NOUN)

    def lemmatization(self, word):
        return self.lemmatizer.lemmatize(word, self.get_wordnet_pos(word))

    def word_processing(self, word_bag):
        for word in word_bag:
            word = self.remove_multiple_whitespace(word)
            word = self.lemmatization(word)
        return word_bag

    def text_preprocessing(self, context):
        context_lower = self.lower_case(context)
        context_no_url = self.url_remove(context_lower)
        context_without_punctuation = self.punctuation_remove(context_no_url)
        context_without_number = self.numbers_remove(context_without_punctuation)
        word_bag = self.tokenize(context_without_number)
        word_bag = self.stop_words_remove(word_bag)
        word_bag = self.word_processing(word_bag)

        return word_bag


class DataParser(object):
    def __init__(self):
        pass
    
    def on_data(self, tweet_json, place_dict, f="", f_location_predict=""):
        tweet_id = str(tweet_json['id'])
        created_at = tweet_json['created_at']
        timestamp = datetime.strftime(datetime.strptime(created_at, '%Y-%m-%dT%H:%M:%S.000Z'),
                                      '%Y-%m-%d %H:%M:%S')
        user_id = tweet_json['author_id']
        context = tweet_json['text'].encode("ascii", "ignore").decode()  # filter out non-ascii characters

        lemmatizer = WordNetLemmatizer()
        pre_processing_pipeline = PipeLine(lemmatizer)
        word_bag = pre_processing_pipeline.text_preprocessing(context)

        # process the geo-tags
        if tweet_json.get('geo') == None:
            record = {"tweet_id": tweet_id, "user_id": user_id, "time": timestamp, "context": context, "word_bag": word_bag}
            self.regex_filter(record, f, f_location_predict)    
            return
        
        if tweet_json['geo'].get('coordinates') == None:
            place_id = tweet_json['geo']['place_id']
            if place_dict.get(place_id) == None:
                return

            place_json = place_dict[place_id]
            if place_json['place_type'] == "neighborhood":
                bbox = place_json['geo']['bbox']
                lng = random.uniform(bbox[0], bbox[2])
                lat = random.uniform(bbox[1], bbox[3])
            elif place_json['place_type'] == "poi":
                bbox = place_json['geo']['bbox']
                lng = (bbox[0] + bbox[2]) / 2.0
                lat = (bbox[1] + bbox[3]) / 2.0
            else:
                record = {"tweet_id": tweet_id, "user_id": user_id, "time": timestamp, "context": context, "word_bag": word_bag}
                self.regex_filter(record, f, f_location_predict)              
                return
                
        else:
            lng = tweet_json['geo']['coordinates']['coordinates'][0]
            lat = tweet_json['geo']['coordinates']['coordinates'][1]

        record = {"tweet_id": tweet_id, "user_id": user_id, "time": timestamp, "longitude": lng,
                      "latitude": lat, "context": context, "word_bag": word_bag}
        print(record)
        f.write(json.dumps(record, ensure_ascii=False) + '\n')
        
        return 
        
    def regex_filter(self, record, f="", f_location_predict=""):
    	regex= r'[0-9]*\s?[0-9A-Za-z]+\s(rd|beach|ave|airport|Airport|Port|Street|Avenue|Center|Road|Yard|Lane|Court|Hill|Highwalk|Way|Square|Walk|Park|Underground|Passage|Alley|Close|Gardens|Hall|Circle|Row|Buildings|Crescent|Market|Drive|Arcade|Esplanade|Grove|Garden|Bridge|Ridge|Terrace|Boulevard|Inn|Wharf|St|Ave|Rd|Yd|Ct|Pl|Sq|Bld|Beach|Blvd|Cres|Dr|Esp|Grn|Gr|Tce|Bvd|Bayou|Parkway|bayou|street|avenue|road|yard|lane|court|square|park|underground|building|Wall|way|wall|port|crescent|drive|esplanade|garden|bridge|ridge|terrace|boulevard|Building|grove|underground|School|school|Highschool|highschool|high school|center)\s?([0-9])*\b'
    	
    	listOfStrings = ['his' , 'the', 'a', 'my', 'never', 'from','in',r'that''s','called','for','to',
                    'at','with','of','minor','own','against','front','that','make','grave','were',
                    'busy','apartment','not','worst','watering','temporary','are','is','and','about',
                    'know','flooded','your','access','service','secret','gotta','whole','this','their',
                    'shit','save','reports','posted','possible','parallel','outside','our','or','observe',
                    'one','on','no','neighbours','multiple','localized','like','its','impacted','her',
                    'hazardous','every','empty','dear','come','by','gotta','of','stop','much','don\'t',
                    'reported','before','after','hurricane']
        
    	locations = re.finditer(regex, record['context'])
    	loc = []
    	for m in locations:
    	    loc_name = m.group().title().lower().strip()
    	    if loc_name.partition(' ')[0].lower() not in listOfStrings:
    	        loc.append(loc_name.strip())
    	if loc:
    	    print(record)
    	    f.write(json.dumps(record, ensure_ascii=False) + '\n')
    	    predict_record = record
    	    predict_record["longitude"] = 0.
    	    predict_record["latitude"] = 0.
    	    predict_record["locations"] = loc
    	    f_location_predict.write(json.dumps(predict_record, ensure_ascii=False) + '\n')
    	    
    	return


class CrawlData(object):
    """
    crawl tweet raw data from twitter API
    """
    def __init__(self, bearer_token, 
                 bounding_box, 
                 start_time, 
                 end_time,
                 f="",
                 f_location_predict=""):
        self.bearer_token = bearer_token
        self.header = {'Authorization': "Bearer " + self.bearer_token}
        self.next_token = ""
        self.bounding_box = bounding_box
        self.start_time = start_time
        self.end_time = end_time
        self.has_next_round = True
        self.tweet_count = 0
        self.total_count = 1000000
        self.f = open(f, 'a', encoding="utf-8")
        self.f_location_predict = open(f_location_predict, 'a', encoding="utf-8")
        self.parser = DataParser()
    
    def crawl_data(self):
        search_url = "https://api.twitter.com/2/tweets/search/all?query=Hurricane "\
        "Hurricane OR Harvey OR flood OR Texas OR Houston OR storm OR has:geo lang:en bounding_box:["\
        + self.bounding_box[0] + " " + self.bounding_box[1] + " " + self.bounding_box[2] + " " + self.bounding_box[3] + \
        "]&&start_time=" + self.start_time + "&&end_time=" + self.end_time + \
        "&&tweet.fields=id,author_id,created_at,geo,text&&expansions=geo.place_id&&" + \
        "place.fields=contained_within,country,geo,name,place_type&&max_results=500"
        
        if self.next_token != "":
            search_url = search_url + "&&next_token=" + self.next_token
        response = requests.get(search_url, headers=self.header)
        searchjson = json.loads(response.content)
        
        if searchjson['meta'].get('next_token') != None:
            self.next_token = searchjson['meta']['next_token']
        else:
            self.has_next_round = False 
        
        datajson = searchjson['data']
        print(datajson)
        
        place_list = searchjson['includes']['places']
        place_dict = {}
        for place_object in place_list:
            key = place_object['id']
            place_dict[key] = place_object
            
        for data in datajson:
            self.tweet_count = self.tweet_count + 1           
            self.parser.on_data(data, place_dict, self.f, self.f_location_predict) # parse data
            
        return self.has_next_round
    
    def crawl_process(self):
        request_count_15min = 0
        start_time_15min = time.time()
        end_time_15min = start_time_15min
        while self.has_next_round == True and self.tweet_count < self.total_count:
            start_time = time.time()
            self.has_next_round = self.crawl_data()
            end_time = time.time()
            end_time_15min = end_time
            request_count_15min = request_count_15min + 1
            
            # 1 requests / second limit
            if end_time - start_time < 1.:
                time.sleep(1) 
            
            # 300 requests / 15 min limit
            if request_count_15min == 300 and end_time_15min - start_time_15min < 15 * 60:
                time.sleep(15 * 60 - (end_time_15min - start_time_15min) + 5)
                
                request_count_15min = 0
                start_time_15min = time.time()
                end_time_15min = start_time_15min
        
        self.f.close()
        print("finished")

if __name__ == "__main__":
    # These are provided to you through the Twitter API after you create a account
    # register a Twitter App to get the keys and access tokens.
    output_file = "/home/dietrich/master_thesis/GeoBurst_OSM/data/tweets_Houston.json"
    file_location_predict = "/home/dietrich/master_thesis/GeoBurst_OSM/data/tweets_need_predict_loc.json"
    output_file_temp = "/home/dietrich/master_thesis/GeoBurst_OSM/data/tweets_Houston_temp.json"

    # LOCATIONS are the longitude, latitude coordinate corners for a box that restricts the
    # geographic area from which you will stream tweets. The first two define the southwest
    # corner of the box and the second two define the northeast corner of the box.
    bounding_box = ["-95.565128", "29.544661", "-95.185277", "29.883392"]  # Houston City
    bearer_token = "AAAAAAAAAAAAAAAAAAAAAEsVSAEAAAAA2Vp0os7em9%2FTe8tUCBWbuP8kRmA%3D82PgW6sI4lZqRX4XApzcDBwmmGwvNy8o43h7SnlVqv16fqxX8w"
    start_time = "2017-08-20T00:00:01.000Z"
    end_time = "2017-09-05T23:59:59.000Z"

    crawler = CrawlData(bearer_token, bounding_box, start_time, end_time, output_file, file_location_predict)
    crawler.crawl_process()
    
    file = open(output_file, "r", encoding="utf-8")
    file_temp = open(output_file_temp, "a", encoding="utf-8")
    lines = file.readlines()
    for line in reversed(lines):
        file_temp.write(line)

    file.close()
    file_temp.close()
