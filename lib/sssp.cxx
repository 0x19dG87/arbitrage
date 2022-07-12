#pragma once

#include <vector>
#include <algorithm>
#include <mutex>
#include <execution>
#include "datastructs.cxx"

namespace arb
{
    namespace atomic
    {
        template <typename type_t>
        type_t min(type_t *a, type_t b, std::mutex &m)
        {
            std::lock_guard<std::mutex> guard(m);
            type_t old = *a;
            type_t ans = min(old, b);
            *a = ans;
            return old;
        }
    } // namespace atomic

    /**
     * @brief Shortest Path Faster Algorithm algorithm.
     * https://konaeakira.github.io/posts/using-the-shortest-path-faster-algorithm-to-find-negative-cycles.html
     */
    template <typename graph_t>
    auto spfa(graph_t &g)
    {
        using vertex_t = graph_t::index_t;
        using weight_t = graph_t::weight_t;

        // Initialize
        std::vector<weight_t> distances(g.getNumVertices(), 0);
        std::vector<vertex_t> pre(g.getNumVertices(), -1);
        std::vector<vertex_t> len(g.getNumVertices());
        std::vector<bool> in_queue(g.getNumVertices(), true);
        std::vector<vertex_t> active_vertexes = g.getVertices();
        std::vector<std::mutex> m_locks(g.getNumVertices());

        std::vector<vertex_t> output;
        output.reserve(g.getNumVertices());
        // lambda
        auto relax = [&](auto const &source_vertex)
        {
            in_queue[source_vertex] = false;

            // For all edges of vertex.
            for (auto edge : g.getEdges(source_vertex))
            {
                auto destination_vertex = g.getDestinationVertex(edge);
                auto edge_weight = g.getEdgeWeight(edge);
                weight_t new_distance = distances[source_vertex] + edge_weight;

                if (new_distance < distances[destination_vertex])
                {
                    distances[destination_vertex] = new_distance;
                    pre[destination_vertex] = source_vertex;
                    len[destination_vertex] = len[source_vertex] + 1;
                    if (len[destination_vertex] < g.getNumVertices() && !in_queue[destination_vertex])
                    {
                        in_queue[destination_vertex] = true;
                        output.push_back(destination_vertex);
                    }
                }
            }
        };

        // Main
        int counter = 0;
        while (!active_vertexes.empty())
        {
            std::cout << ++counter << std::endl;
            std::for_each(active_vertexes.begin(), active_vertexes.end(), relax);
            active_vertexes = output;
            output.clear();
        }

        // Last iteration to build negative cycle
        active_vertexes = g.getVertices();
        auto build_cycle = [&](auto &vertex)
        {
            arb::cycle_t<graph_t> cycle(g, pre, vertex);
        };

        std::for_each(active_vertexes.begin(), active_vertexes.end(), build_cycle);

        // --
        // Return the distances vector.
        return distances;
    }

} // namespace essentials