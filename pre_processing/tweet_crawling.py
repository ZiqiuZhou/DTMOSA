import tweepy
import re
import json
import time
from datetime import datetime
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


class StreamListener(tweepy.StreamListener):
    """tweepy.StreamListener is a class provided by tweepy used to access
    the Twitter Streaming API to collect tweets in real-time.
    """

    def __init__(self, time_limit=60, file=""):
        """class initialization"""
        self.start_time = time.time()
        self.limit = time_limit
        self.f = open(file, 'a', encoding="utf-8")
        super(StreamListener, self).__init__()

    def on_data(self, data):
        """This is called when data are streamed in."""
        if (time.time() - self.start_time) < self.limit:
            datajson = json.loads(data)
            if 'id' not in datajson.keys():
                time.sleep(1)
            else:
                # {'limit': {'track': 13, 'timestamp_ms': '1585851016736'}}

                tweet_id = str(datajson['id'])
                created_at = datajson['created_at']
                timestamp = datetime.strftime(datetime.strptime(created_at, '%a %b %d %H:%M:%S +0000 %Y'),
                                              '%Y-%m-%d %H:%M:%S')
                username = datajson['user']['screen_name']
                raw_text = datajson['text'].encode("ascii", "ignore").decode()  # filter out non-ascii characters

                lemmatizer = WordNetLemmatizer()
                pre_processing_pipeline = PipeLine(lemmatizer)
                context = pre_processing_pipeline.text_preprocessing(raw_text)

                # process the geo-tags
                if datajson['coordinates'] == None:
                    try:
                        bbox = datajson['place']['bounding_box']['coordinates'][0]
                        lng = (bbox[0][0] + bbox[2][0]) / 2.0
                        lat = (bbox[0][1] + bbox[1][1]) / 2.0
                    except:
                        lat = 0
                        lng = 0
                else:
                    lng = datajson['coordinates']['coordinates'][0]
                    lat = datajson['coordinates']['coordinates'][1]

                if lat != 0:
                    record = {"tweet_id": tweet_id, "user_name": username, "time": timestamp, "longitude": lng,
                              "latitude": lat, "context": context}
                    print(record)
                    self.f.write(json.dumps(record, ensure_ascii=False) + '\n')
                else:
                    pass
        else:
            self.f.close()
            print("finished")
            return False


if __name__ == "__main__":
    # These are provided to you through the Twitter API after you create a account
    # register a Twitter App to get the keys and access tokens.
    output_file = "D:/Heidelberg/master_thesis/GeoBurst_OSM/data/tweets_NY.json"

    # Apply for your own Twitter API keys at https://developer.twitter.com/en/apply-for-access
    consumer_key = "RLscOzYXozeyX6ZISRDjUVhOw"
    consumer_secret = "1IoVz7huJtbqg0DppA9NuBZlBEwPmjFqK3kpD1LFp9vc3ff2zC"
    access_token = "1131159482292211714-wKdfHqmf8JUuMc1Z9TXjcrGwiXqcIT"
    access_token_secret = "JeAreqHT1a07pxfHwbQKFsgngO7Wk8oobzlzewpgOUKbM"

    myauth = tweepy.OAuthHandler(consumer_key, consumer_secret)
    myauth.set_access_token(access_token, access_token_secret)

    # LOCATIONS are the longitude, latitude coordinate corners for a box that restricts the
    # geographic area from which you will stream tweets. The first two define the southwest
    # corner of the box and the second two define the northeast corner of the box.
    LOCATIONS = [-74.63, 40.50, -73.63, 40.97]  # New York City

    stream_listener = StreamListener(time_limit=86400, file=output_file)
    stream = tweepy.Stream(auth=myauth, listener=stream_listener)
    stream.filter(locations=LOCATIONS)
