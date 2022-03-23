import json
import numpy as np
import spacy
import nltk
from nltk.tag import pos_tag
from nltk.tokenize import word_tokenize
from sner import Ner

nltk.download('punkt')
nltk.download('averaged_perceptron_tagger')
nltk.download('maxent_ne_chunker')
nltk.download('words')

stanfordNER_tagger = Ner(host='localhost',port=9199)
spacy_tagger = spacy.load('en_core_web_sm')

def parse_stanford_location(tagged):
    if not tagged:
        return set()

    location_set = set()
    loc_name = ""
    index = 0
    while index < len(tagged):
        while index < len(tagged) and (tagged[index][1] == 'LOCATION' or tagged[index][1] == 'ORGANIZATION'):
            item = tagged[index]
            loc_name = loc_name + item[0] + ' '
            index = index + 1
        if loc_name:
            location_set.add(loc_name[:-1])
        loc_name = ""
        index = index + 1
    return location_set


def parse_spacy_location(tagged):
    if not tagged:
        return set()

    location_set = set()
    for entry in tagged:
        if entry.label_ == 'ORG' or entry.label_ == 'LOC' or entry.label_ == 'GPE':
            location_set.add(entry.text)
    return location_set


def location_match(ground_truth_locs, predict_locs):
    tp = 0
    precision, recall, f_score = 0., 0., 0.
    if predict_locs:
        for pred_loc in predict_locs:
            if pred_loc in ground_truth_locs:
                tp = tp + 1
        precision = tp / len(predict_locs)
        recall = tp / len(ground_truth_locs)
        if precision == 0. and recall == 0.:
            f_score = 0.
        else:
            f_score = 2 * (precision * recall) / (precision + recall)
    return precision, recall, f_score


def parse_nltk_location(context):
    if not context:
        return set()

    location_set = set()
    for sent in nltk.sent_tokenize(context):
        for chunk in nltk.ne_chunk(nltk.pos_tag(nltk.word_tokenize(sent))):
            if hasattr(chunk, 'label'):
                if chunk.label() == 'LOCATION' or chunk.label() == 'GPE' or chunk.label() == 'ORGANIZATION':
                    location_set.add(' '.join(c[0] for c in chunk))
    return location_set


def evaluate_process(file):
    Precision_sner = 0.
    Recall_sner = 0.
    F_score_sner = 0.
    Precision_spacy = 0.
    Recall_spacy = 0.
    F_score_spacy = 0.
    Precision_nltk = 0.
    Recall_nltk = 0.
    F_score_nltk = 0.

    count = 0
    while True:
        line = file.readline()
        if not line:
            break
        count = count + 1
        tweet = json.loads(line)
        ground_truth_locations = set(tweet['ground_truth_loc'])
        precision, recall, f_score = 0., 0., 0.

        # Stanford NER
        stanford_entities = stanfordNER_tagger.get_entities(tweet['context'])
        location_set = parse_stanford_location(stanford_entities)
        precision, recall, f_score = location_match(ground_truth_locations, location_set)
        Precision_sner = Precision_sner + precision
        Recall_sner = Recall_sner + recall
        F_score_sner = F_score_sner + f_score

        # spacy
        spacy_entities = spacy_tagger(tweet['context']).ents
        location_set = parse_spacy_location(spacy_entities)
        precision, recall, f_score = location_match(ground_truth_locations, location_set)
        Precision_spacy = Precision_spacy + precision
        Recall_spacy = Recall_spacy + recall
        F_score_spacy = F_score_spacy + f_score

        # nltk
        location_set = parse_nltk_location(tweet['context'])
        precision, recall, f_score = location_match(ground_truth_locations, location_set)
        Precision_nltk = Precision_nltk + precision
        Recall_nltk = Recall_nltk + recall
        F_score_nltk = F_score_nltk + f_score

    print("Stanford_NER: Precision: {p}, Recall: {r}, F_score: {f}".format(p=Precision_sner / count, r=Recall_sner / count, f=F_score_sner / count))
    print("Spacy: Precision: {p}, Recall: {r}, F_score: {f}".format(p=Precision_spacy / count, r=Recall_spacy / count, f=F_score_spacy / count))
    print("NLTK: Precision: {p}, Recall: {r}, F_score: {f}".format(p=Precision_nltk / count, r=Recall_nltk / count, f=F_score_nltk / count))


if __name__ == "__main__":
    NAME_FILE_PATH = "/home/dietrich/master_thesis/GeoBurst_OSM/evaluation/geo-parser/tweets_name_based_locations.json"
    ADDRESS_FILE_PATH = "/home/dietrich/master_thesis/GeoBurst_OSM/evaluation/geo-parser" \
                        "/tweets_address_based_locations.json"

    f_name_loc = open(NAME_FILE_PATH, 'r', encoding="utf-8")
    f_address_loc = open(ADDRESS_FILE_PATH, 'r', encoding="utf-8")

    evaluate_process(f_address_loc)

