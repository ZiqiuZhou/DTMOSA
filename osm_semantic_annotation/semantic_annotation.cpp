//
// Created by dietrich on 08.02.22.
//

#include "semantic_annotation.h"

namespace OpenStreetMapAnnotation::SemanticAnnotation {

    std::mutex mut;

    void MATF(int block, int thread_num,
              double& avg_doc_length,
              double& sum_score,
              std::string& word,
              std::vector<DocumentType>& corpus) {
        double local_score = 0.;
        for (int i = block * corpus.size() / thread_num; i < (block + 1) * corpus.size() / thread_num; ++i) {
            DocumentType& doc = corpus[i];
            if (doc.find(word) == doc.end()) {
                continue;
            }
            std::unordered_map<std::string, int> word_count = {};
            for (auto& w : doc) {
                word_count[w]++;
            }
            int tf = word_count[word];
            double average_tf = 0.;
            for (auto& [w, count] : word_count) {
                average_tf += count;
            }
            average_tf /= doc.size();

            double RITF = std::log(1 + tf) / std::log(1 + average_tf);
            double LRTF = tf * std::log(1 + avg_doc_length / doc.size());
            local_score += (RITF + LRTF);
            local_score /= 2.0;

            std::unique_lock<std::mutex> lock(mut);
            sum_score += local_score;
            lock.unlock();
        }
    }

    AnnotationHandler::AnnotationHandler(OpenStreetMap& _osm_object, std::vector<Tweet>& candidate_tweets)
    : osm_object(_osm_object) {
        corpus = {};
        annotation_candidates = {};

        for (Tweet& tweet : candidate_tweets) {
            auto& word_bag = tweet.GetWordBag();
            corpus.emplace_back(word_bag);
        }
    }

    AnnotationHandler::~AnnotationHandler() {
        corpus.clear();
    }

    void AnnotationHandler::Rank() {
        std::unordered_map<std::string, double> word_rank = {};
        std::size_t n = corpus.size();
        double avg_doc_length = 0.;

        // term frequency (TF)
        for (DocumentType& doc : corpus) {
            avg_doc_length += doc.size();
            std::unordered_set<std::string> helper_word_set = {};
            for (const std::string& word : doc) {
                if (helper_word_set.find(word) == helper_word_set.end()) {
                    helper_word_set.insert(word);
                } else {
                    continue;
                }

                word_rank[word]++;
            }
        }
        avg_doc_length /= corpus.size();

        // IDF
        for (auto& [word, score] : word_rank) {
            double doc_has_word = 0.;
            for (DocumentType& doc : corpus) {
                if (doc.find(word) != doc.end()) {
                    doc_has_word++;
                }
            }
            word_rank[word] = score * std::log((n + 1.) / (doc_has_word + 1)); // TF-IDF
        }

        // compute Multi Aspect Term Frequency (MATF)
        std::size_t thread_num = std::thread::hardware_concurrency();
        if (n < thread_num) {
            thread_num = n;
        }
        for (auto& word_score : word_rank) {
            std::string word = word_score.first;
            double sum_score = 0.;

            std::vector<std::thread> threads;
            for (int block = 0; block < thread_num; ++block) {
                threads.push_back(
                        std::thread{MATF, block, thread_num,
                                    std::ref(avg_doc_length),
                                    std::ref(sum_score),
                                    std::ref(word),
                                    std::ref(corpus)});
            }
            for (int block = 0; block < thread_num; ++block) {
                threads[block].join();
            }

            word_rank[word] *= sum_score;
            word_rank[word] += 1e-5;
        }

        struct Cmp {
            bool operator()(const WordScoreType& lhs, const WordScoreType& rhs) {
                return lhs.second < rhs.second;
            }
        };
        std::priority_queue<WordScoreType, std::vector<WordScoreType>, Cmp> rank_queue;
        for (auto& [word, score] : word_rank) {
            rank_queue.push({word, score});
            if (rank_queue.size() > rank_num) {
                rank_queue.pop();
            }
        }

        while (!rank_queue.empty()) {
            annotation_candidates.emplace_back(rank_queue.top());
            rank_queue.pop();
        }

        return ;
    }

    std::vector<WordScoreType>& AnnotationHandler::GetAnnotations() {
        return this->annotation_candidates;
    }
}

