### Documentation
In this directory we mainly focus on crawling Twitter and OSM stream data. The crawling processes are implemented in the file tweet_crawling.py and OSM_crawling.py.

#### 1. tweet_crawling.py
In this python routine one can collected tweet set and do some NLP pre-processing work. The parameters must be set in the main function, to run the routine, just type "python3 tweet_crawling.py", it will generate a ".json" file contain all tweets that meet the requirements. Each line contains only one tweet.

The parameters need to be set in the main function:
-  output_file directory (in absolute path): conatins all tweets.
-  file_location_predict: subset of tweets that is not geo-tagged but probably contain geo-information, thus need to be predicted.
-  bounding_box: a "[lon1, lat1, lon2, lat2]" list represents the geo-boundary. 
-  start_time & end_time: the time period

In the crawling processing, the function "crawl_data" generates the request url strings to visist the Twitter streaming API. You can refer https://dev.twitter.com/streaming/overview to design your own request string.

"tweet_crawling.py" also deals with some pre-processing steps in NLP, such as: lower_case, url / numbers /stop words / multiple whitespace removal, tokenization and lemmatization.

The output file includes all tweets with .json format. For each tweet, we contain the following meta-data:
- tweet_id
- user_id
- time
- longitude & latitude
- context
- word bag (set of words from tweet context)

#### 2. OSM_crawling.py
This routine aims at collecting OpenStreetMap data meeting your demands. Here we deals with three types of OSM objects - buildings, roads and points:
- to pick buildings: "building=* and geometry:polygon"
- to pick roads: "highway=* and type:way"
- to pick points: "geometry:point"
Also just type "python3 OSM_crawling.py" to run the code, and the output is a file containing the collected OSM objects with .json format.

Some parameters to be set:
- path to store the output file
- the request URL: 'https://api.ohsome.org/v1/contributions/latest/geometry'
- data: {"bboxes": "lon1, lat1, lon1, lat1", "time": "yyyy-mm-dd (start time),yyyy-mm-dd (end time)", "showMetadata": "yes", "properties": "metadata,tags"}

The output file includes all OSM objects with .json format. For each object, we contain the following meta-data:
- osm_id
- timestamp
- type
- coordinates
- tags
