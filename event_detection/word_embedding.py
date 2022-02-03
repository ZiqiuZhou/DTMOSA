import sys
import json

WORD_DICT_PATH = ""
TWEET_LIST_PATH = ""
OUTPUT_PATH = ""


def word_bag_filter(word_bag_origin, word_embedding_dict):
    word_bag = []
    for word in word_bag_origin:
        if word in word_embedding_dict:
            word_bag.append(word)
    return word_bag


def make_sentence(word_bag):
    sentence = ""
    for idx in range(len(word_bag)):
        if idx == 0:
            sentence += word_bag[idx]
        else:
            sentence += (" " + word_bag[idx])
    return sentence


def count_word(word_bag):
    word_count = {}
    for word in word_bag:
        word_count[word] = 0
    for word in word_bag:
        word_count[word] += 1
    return word_count


def term_frequency(word_count, word_bag):
    tf_dict = {}
    length = len(word_bag)
    for word, count in word_count.items():
        tf_dict[word] = count/float(length)
    word_count = tf_dict
    return word_count


def content_embedding(tweet_list, word_embedding_dict, f):
    for tweet_str in tweet_list:
        tweet = json.loads(tweet_str)
        word_bag_origin = tweet['word_bag']
        word_bag = word_bag_filter(word_bag_origin, word_embedding_dict)
        if not word_bag:
            continue

        word_count = count_word(word_bag)
        weight_dict = term_frequency(word_count, word_bag)
        tweet_output = {'tweet_id': tweet['tweet_id'], 'word_embedding': {}}
        if len(word_bag) == 1:
            word_weight = weight_dict[word_bag[0]]
            vectorization = json.loads(word_embedding_dict[word_bag[0]])
            max_ele = max(vectorization)
            min_ele = min(vectorization)
            for i in range(len(vectorization)):
                vectorization[i] = (vectorization[i] - min_ele) / (max_ele - min_ele + 1e-5)
            tweet_output['word_embedding'][word_bag[0]] = {'weight': word_weight, 'vectorization': vectorization}
        else:
            for word, weight in weight_dict.items():
                vectorization = json.loads(word_embedding_dict[word])
                tweet_output['word_embedding'][word] = {'weight': weight, 'vectorization': vectorization}
        f.write(json.dumps(tweet_output, ensure_ascii=False) + '\n')
    f.close()
    return


def main():
    WORD_DICT_PATH = sys.argv[1]
    TWEET_LIST_PATH = sys.argv[2]
    OUTPUT_PATH = sys.argv[3]

    word_embedding_dict = open(WORD_DICT_PATH, 'r', encoding='utf-8').readlines()[0]
    word_embedding_dict = json.loads(word_embedding_dict)
    tweet_list = open(TWEET_LIST_PATH, 'r', encoding='utf-8').readlines()

    # file to write output
    f = open(OUTPUT_PATH, 'w', encoding="utf-8")
    content_embedding(tweet_list, word_embedding_dict, f)

    return


if __name__ == '__main__':
    main()
