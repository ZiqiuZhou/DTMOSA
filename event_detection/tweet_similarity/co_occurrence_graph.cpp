//
// Created by dietrich on 28.10.21.
//

#include "co_occurrence_graph.h"

namespace EventTweet::Co_Occurrence_Graph {

    KeyWordGraph::KeyWordGraph(SnapShot& _snapshot) {
        snapshot = _snapshot;
        SIZE = snapshot.GetBurstyWords().size();
        vertex_list.clear();
        vertex_index_map.clear();
    }

    KeyWordGraph& KeyWordGraph::Init() {
        adjacent_matrix.resize(static_cast<long>(SIZE), static_cast<long>(SIZE));
        return *this;
    }

    KeyWordGraph::~KeyWordGraph() {
        snapshot.Reset();
        vertex_list.clear();
        vertex_index_map.clear();
        adjacent_matrix.setZero();
    }

    void KeyWordGraph::Reset() {
        snapshot.Reset();
        vertex_list.clear();
        vertex_index_map.clear();
        adjacent_matrix.setZero();
    }

    KeyWordGraph& KeyWordGraph::GenerateVertexList() {
        auto& bursty_words_set = snapshot.GetBurstyWords();
        for (const std::string& word : bursty_words_set) {
            auto vertex = Vertex(word);
            vertex_list.emplace_back(std::move(vertex));
        }
        return *this;
    }

    KeyWordGraph& KeyWordGraph::GenerateVertexIndexMap() {
        if (vertex_list.empty()) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " empty vertex_list." << std::endl;
            return *this;
        }
        for (std::size_t i = 0; i < vertex_list.size(); ++i) {
            std::string word = vertex_list[i].GetVertex();
            vertex_index_map[word] = i;
        }
        return *this;
    }

    int KeyWordGraph::FindCommonTweets(std::unordered_set<std::string>& left_set,
                                       std::unordered_set<std::string>& right_set) {
        if (left_set.empty() || right_set.empty()) {
            std::cout << " file path = " << __FILE__ << " function name = " << __FUNCTION__ << " line = " << __LINE__
                      << " empty set." << std::endl;
            return 0;
        }

        int count = 0;
        for (auto& word: left_set) {
            if (right_set.find(word) != right_set.end()) {
                ++count;
            }
        }
        return count;
    }

    KeyWordGraph& KeyWordGraph::GenerateAdjacentMatrix() {
        auto& word_tweet_pair = snapshot.GetWordTweetPair();
        std::vector<Triplet<double> > coefficients; // list of non-zeros coefficients (row_idx, col_idx, value)
        struct Pos {
            std::size_t row;
            std::size_t col;
            Pos(std::size_t _row, std::size_t _col) : row(_row), col(_col) {}
        };
        using ElementInfo = std::pair<Pos, double>;

        std::vector<ElementInfo> non_zero_row;
        // O(n**2) complexity to compute normalized weighted sparse matrix
        for (std::size_t i = 0; i < vertex_list.size(); ++i) {
            double sum = 0.;
            for (std::size_t j = 0; j < vertex_list.size(); ++j) {
                if (i == j) {
                    continue;
                }
                auto word_i = vertex_list[i].GetVertex();
                auto word_j = vertex_list[j].GetVertex();
                if (word_tweet_pair.find(word_i) == word_tweet_pair.end() ||
                    word_tweet_pair.find(word_j) == word_tweet_pair.end()) {
                    continue;
                }
                std::unordered_set<std::string>& tweet_set_i = word_tweet_pair[word_i];
                std::unordered_set<std::string>& tweet_set_j = word_tweet_pair[word_j];
                int count = FindCommonTweets(tweet_set_i, tweet_set_j);
                if (count > 0) {
                    Pos position(i, j);
                    ElementInfo element_info = std::make_pair(position, static_cast<double>(count));
                    non_zero_row.push_back(std::move(element_info));
                    sum += count;
                }
            }
            if (!non_zero_row.empty()) {
                std::for_each(non_zero_row.begin(), non_zero_row.end(), [&sum](ElementInfo &element) {
                    element.second /= sum;
                });
                for (ElementInfo& info: non_zero_row) {
                    int row = info.first.row;
                    int col = info.first.col;
                    double score = info.second;
                    if (score >= 0.02) {
                        coefficients.emplace_back(Triplet<double>(row, col, score));
                    }
                }
            }
            non_zero_row.clear();
        }
        adjacent_matrix.setFromTriplets(coefficients.begin(), coefficients.end());
        return *this;
    }

    std::unordered_map<std::string, int>& KeyWordGraph::GetVertexIndexMap() {
        return this->vertex_index_map;
    }

    SparseMatrix<double, ColMajor>& KeyWordGraph::GetAdjacentMatrix() {
        return this->adjacent_matrix;
    }

    std::vector<Vertex>& KeyWordGraph::GetVertexList() {
        return this->vertex_list;
    }
}

