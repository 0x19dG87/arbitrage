#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <cmath>
#include <execution>
#include <ranges>
#include <numeric>
#include <stack>
#include <set>

#include <iostream>

namespace arb
{

    template <typename address_t, typename price_t>
    struct pair_t
    {
        using address_type = address_t;
        using price_type = price_t;

        address_t token1;
        price_t tokenPrice1;
        address_t token2;
        price_t tokenPrice2;
        address_t pool;

        pair_t(address_t _token1, price_t _tokenPrice1, address_t _token2, price_t _tokenPrice2, address_t _pool)
            : token1(_token1), tokenPrice1(_tokenPrice1), token2(_token2), tokenPrice2(_tokenPrice2), pool(_pool)
        {
        }

        std::string toString()
        {
            return "token1:" + token1 + ", token_price1:" + std::to_string(tokenPrice1) + ", token2:" + token2 + ", token_price2:" + std::to_string(tokenPrice2) + ", pool:" + pool;
        }
    };

    template <typename index_t, typename pair_t>
    struct pairs_t
    {
        using pair_type = pair_t;
        using index_type = index_t;
        using address_type = pair_t::address_type;
        using price_type = pair_t::price_type;

        inline size_t key(const index_t &i, const index_t &j) { return (size_t)i << 32 | j; }

        std::unordered_map<address_type, index_t> indexesByTokens;
        std::unordered_map<size_t, pair_type> poolsByTokenPairs;

        int getNumTokens() { return indexesByTokens.size(); }
        int getNumValues() { return poolsByTokenPairs.size() * 2; } // one pair will be represented 2 times
        auto getPairs() { return poolsByTokenPairs; }
        index_t getTokenIndex(const address_type &token) { return indexesByTokens[token]; }

        void addEntry(const pair_type &pair)
        {
            // get index for 1st token
            index_t tokenIndex1 = indexesByTokens.size();
            if (indexesByTokens.contains(pair.token1))
            {
                tokenIndex1 = indexesByTokens[pair.token1];
            }
            else
            {
                indexesByTokens[pair.token1] = tokenIndex1;
            }

            // get index for 2nd token
            index_t tokenIndex2 = indexesByTokens.size();
            if (indexesByTokens.contains(pair.token2))
            {
                tokenIndex2 = indexesByTokens[pair.token2];
            }
            else
            {
                indexesByTokens[pair.token2] = tokenIndex2;
            }

            if (auto pair_ref = findPair(tokenIndex1, tokenIndex2))
            {
                pair_ref->get() = pair;
            }
            else
            {
                poolsByTokenPairs.insert(std::pair<size_t, pair_type>(key(tokenIndex1, tokenIndex2), pair));
            }
        }

        auto findPair(const index_t &tokenIndex1, const index_t &tokenIndex2)
        {   
            bool is_revers;
            return findPair(tokenIndex1, tokenIndex2, is_revers);
        }

        auto findPair(const index_t &tokenIndex1, const index_t &tokenIndex2, bool &is_revers)
        {
            size_t tokensPairKey = key(tokenIndex1, tokenIndex2);
            // get pointer of existing pool for the current pair of tokens
            auto pair_it = poolsByTokenPairs.find(tokensPairKey);
            if (pair_it == poolsByTokenPairs.end())
            {
                size_t tokensPairKey_revers = key(tokenIndex2, tokenIndex1);
                auto pair_r_it = poolsByTokenPairs.find(tokensPairKey_revers);
                is_revers = true;
                return pair_r_it == poolsByTokenPairs.end() ? std::nullopt : std::optional<std::reference_wrapper<pair_type>>{pair_r_it->second};
            }
            else
            {
                is_revers = false;
                return std::optional<std::reference_wrapper<pair_type>>{pair_it->second};
            }
        }

        void print()
        {
            std::cout << "token_pair_key:pools" << std::endl;
            for (auto &p : poolsByTokenPairs)
            {
                std::cout << p.first << ">> " << p.second.toString() << std::endl;
            }
            std::cout << "index:token" << std::endl;
            for (auto &i : indexesByTokens)
            {
                std::cout << i.second << " " << i.first << std::endl;
            }
            std::cout << std::endl;
        }
    };

    template <typename pairs_t>
    struct csr_t
    {
        using pairs_type = pairs_t;
        using index_t = pairs_t::index_type;
        using value_t = pairs_t::price_type;
        using address_t = pairs_t::address_type;

        pairs_type pairs;

        index_t rows;                        // Number of rows.
        index_t cols;                        // Number of columns.
        index_t nnzs;                        // Number of non-zeros.
        std::vector<index_t> row_offsets;    // Row offsets.
        std::vector<index_t> column_indices; // Column indices.
        std::vector<value_t> values;         // Values of non-zeros.

        csr_t(pairs_type &_pairs) : pairs(_pairs)
        {
            rows = cols = _pairs.getNumTokens();
            nnzs = _pairs.getNumValues();

            // COO vectors
            std::vector<index_t> row_indices(nnzs);
            std::vector<index_t> col_indices(nnzs);
            std::vector<value_t> vals(nnzs);

            index_t i = 0;
            // for_each(std::execution::par, begin(pairs.getPairs()), end(pairs.getPairs()), [&](auto &p) {
            //     // first pair t1-t2
            //     row_indices[i] = pairs.getTokenIndex(p.second.token1);
            //     col_indices[i] = pairs.getTokenIndex(p.second.token2);
            //     vals[i++] = -log(p.second.tokenPrice2);

            //     // second pair t2-t1
            //     row_indices[i] = pairs.getTokenIndex(p.second.token2);
            //     col_indices[i] = pairs.getTokenIndex(p.second.token1);
            //     vals[i++] = -log(p.second.tokenPrice1);
            // });
            for (const auto &p : _pairs.getPairs())
            {
                // first pair t1-t2
                row_indices[i] = _pairs.getTokenIndex(p.second.token1);
                col_indices[i] = _pairs.getTokenIndex(p.second.token2);
                vals[i++] = -std::log(p.second.tokenPrice2);

                // second pair t2-t1
                row_indices[i] = _pairs.getTokenIndex(p.second.token2);
                col_indices[i] = _pairs.getTokenIndex(p.second.token1);
                vals[i++] = -std::log(p.second.tokenPrice1);
            }
            // Convert from COO to CSR.

            // CSR vectors
            row_offsets.resize(rows + 1);
            column_indices.resize(nnzs);
            values.resize(nnzs);

            // compute number of non-zero entries per row of A.
            for (index_t n = 0; n < nnzs; ++n)
            {
                ++row_offsets[row_indices[n]];
            }

            // cumulative sum the nnz per row to get row_offsets[].
            for (index_t i = 0, sum = 0; i < rows; ++i)
            {
                index_t temp = row_offsets[i];
                row_offsets[i] = sum;
                sum += temp;
            }
            row_offsets[rows] = nnzs;

            // write coordinate column indices and nonzero values into CSR's
            // column indices and nonzero values.
            for (index_t n = 0; n < nnzs; ++n)
            {
                index_t row = row_indices[n];
                index_t dest = row_offsets[row];

                column_indices[dest] = col_indices[n];
                values[dest] = vals[n];

                ++row_offsets[row];
            }

            for (index_t i = 0, last = 0; i <= rows; ++i)
            {
                index_t temp = row_offsets[i];
                row_offsets[i] = last;
                last = temp;
            }
        }
    };

    /**
     * @brief Graph data structure based on CSR format.
     *
     * @tparam vertex_t Type of vertex.
     * @tparam edge_t Type of edge.
     * @tparam weight_t Type of weight.
     */
    template <typename csr_t>
    struct graph_t : public csr_t
    {
        using index_t = csr_t::index_t;
        using weight_t = csr_t::value_t;
        using address_t = csr_t::address_t;
        using pairs_t = csr_t::pairs_type;

        graph_t(pairs_t &pairs) : csr_t(pairs) {}

        auto getPair(const index_t &src, const index_t &dest, bool &is_revers) { return csr_t::pairs.findPair(src, dest, is_revers); }
        auto getEdgeWeight(const index_t &e) { return csr_t::values[e]; }
        auto getNumVertices() { return csr_t::rows; }
        auto getDestinationVertex(const index_t &e) { return csr_t::column_indices[e]; }
        auto getVertices()
        {
            std::vector<index_t> v(getNumVertices());
            std::iota(std::begin(v), std::end(v), 0);
            return v;
        }
        auto getEdges(const index_t &v) { return std::ranges::iota_view{csr_t::row_offsets[v], csr_t::row_offsets[v + 1]}; }

        auto print()
        {
            std::cout << "rows_count:" << csr_t::rows << ":cols_count:" << csr_t::cols << ":nnzs_count:" << csr_t::nnzs << std::endl;
            std::cout << "row offsets" << std::endl;
            for (auto const &ro : csr_t::row_offsets)
            {
                std::cout << ro << " ";
            }
            std::cout << std::endl;
            std::cout << "column indexes" << std::endl;
            for (auto const &ci : csr_t::column_indices)
            {
                std::cout << ci << " ";
            }
            std::cout << std::endl;
            std::cout << "values" << std::endl;
            for (auto const &v : csr_t::values)
            {
                std::cout << v << " ";
            }
            std::cout << std::endl;
        }
    };

    template <typename graph_t>
    struct cycle_t
    {
        using vertex_t = graph_t::index_t;
        using weight_t = graph_t::weight_t;

        std::set<vertex_t> cycle_set;
        std::stack<vertex_t> cycle_stack;
        std::vector<vertex_t> cycle;
        weight_t weight = 1.0f;

        cycle_t(graph_t &g, const std::vector<vertex_t> &pre, vertex_t vertex)
        {
            while (!cycle_set.contains(vertex))
            {
                cycle_stack.push(vertex);
                cycle_set.insert(vertex);
                vertex = pre[vertex];
            }
            cycle.reserve(cycle_set.size());
            cycle.push_back(vertex);
            std::cout << "-----start-----" << std::endl;
            while (cycle_stack.top() != vertex)
            {
                cycle.push_back(cycle_stack.top());
                cycle_stack.pop();

                vertex_t s1 = *(cycle.rbegin() + 1);
                vertex_t s2 = *(cycle.rbegin());
                std::cout << s1 << "-" << s2;
                bool is_revers;;
                if (auto pair = g.getPair(*(cycle.rbegin() + 1), *cycle.rbegin(), is_revers))
                {
                    weight_t price = (!is_revers) ? pair->get().tokenPrice2 : pair->get().tokenPrice1;
                    std::cout << "(" << pair->get().pool << ") ";
                    weight *= price;
                }
            }
            cycle.push_back(vertex);

            vertex_t s1 = *(cycle.rbegin() + 1);
            vertex_t s2 = *(cycle.rbegin());
            std::cout << s1 << "-" << s2;
            bool is_revers;
            if (auto pair = g.getPair(*(cycle.rbegin() + 1), *cycle.rbegin(), is_revers))
            {
                weight_t price = (!is_revers) ? pair->get().tokenPrice2 : pair->get().tokenPrice1;
                std::cout << "(" << pair->get().pool << ") ";
                weight *= price;
            }
            std::cout << " :: " << weight << std::endl;
            std::cout << "------end-----" << std::endl;
        }
    };

}