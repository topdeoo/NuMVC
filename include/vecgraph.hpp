#ifndef MPGRAPHS_VECGRAPH_HPP
#define MPGRAPHS_VECGRAPH_HPP

#include "common.hpp"
#include <vector>
#include <optional>
#include <range/v3/all.hpp>
#include "vecmap.hpp"

namespace mpgraphs {

/**
 * Graph using `std::vector` for vertex and adjacency list.
 * This graph cannot store information in edges.
 *
 * `Directed` graphs only store outgoing edges but no reference to incoming edges.
 * `Undirected` graphs work like directed graphs that store two arc for each edge.
 * `Bidirectional` graphs are directed graphs that have additional information on incoming edges in each vertex.
 *
 * `Simple` graphs cannot have multiple edges.
 * Otherwise multiple edges are allowed.
 *
 * @tparam VertexData data stored in vertices
 * @tparam Dir edge direction
 * @tparam Simple whether the graph is simple
 */
template<class VertexData = Empty, EdgeDirection Dir = EdgeDirection::Directed, bool Simple = true, class Timestamp=bool, std::unsigned_integral Unsigned=size_t>
class VecGraph {
public:
    using VertexDescriptor = Unsigned;
    using EdgeDescriptor = std::pair<VertexDescriptor, Unsigned>;
    static constexpr EdgeDirection Direction = Dir;
private:
    struct DirectedVertexEntry {
        template<class... Args>
        DirectedVertexEntry(Args... args) : inDegree{0}, data{args...}, outNeighbors{} {}
        Unsigned inDegree;
        VertexData data;
        std::vector<VertexDescriptor> outNeighbors;
    };
    struct BidirectionalVertexEntry {
        template<class... Args>
        BidirectionalVertexEntry(Args... args) : inDegree(0), outDegree(0), data{args...}, outNeighbors{}, inNeighbors{} {}
        Unsigned inDegree, outDegree;
        VertexData data;
        std::vector<VertexDescriptor> outNeighbors;
        std::vector<VertexDescriptor> inNeighbors;
    };
    using VertexEntry = std::conditional_t<Dir == EdgeDirection::Bidirectional, BidirectionalVertexEntry, DirectedVertexEntry>;

    VecMap<Unsigned, VertexEntry, Timestamp> m_vertices;
    Unsigned m_numEdges;
public:
    /**
     * Create a new graph with `numVertices` isolated vertices.
     */
    VecGraph(Unsigned numVertices) : m_vertices(numVertices), m_numEdges{0} { }
    /**
     * create a new empty graph.
     */
    VecGraph() : VecGraph(0) { }
    /**
     * Copy constructor.
     */
    VecGraph(const VecGraph&) = default;
    /**
     * Move constructor.
     */
    VecGraph(VecGraph&&) = default;

    VecGraph& operator=(const VecGraph&) = default;
    VecGraph& operator=(VecGraph&&) = default;

    /**
     * Returns whether the graph is directed.
     */
    static constexpr bool isDirected() {
        return Dir != EdgeDirection::Undirected;
    }

    static constexpr bool hasEmptyEdgeEntries() {
        return Dir == EdgeDirection::Directed;
    }

    void cleanVertices() {
        // TODO
        struct NotImplemented : public std::exception {};
        throw NotImplemented{};
    }

    /**
     * Returns a range of all outgoing edges of `v`.
     */
    auto outEdges(VertexDescriptor v) const requires (VecGraph::hasEmptyEdgeEntries()) {
        return ranges::views::enumerate(m_vertices.at(v).outNeighbors)
            | ranges::views::filter([this](auto iw) { return hasVertex(iw.second);})
            | ranges::views::transform([v](auto iw) { return std::make_pair(v, iw.first);});
    }

    /**
     * Returns a range of all outgoing edges of `v`.
     */
    auto outEdges(VertexDescriptor v) const requires (!VecGraph::hasEmptyEdgeEntries()) {
        return ranges::views::enumerate(m_vertices.at(v).outNeighbors)
               | ranges::views::transform([v](auto iw) { return std::make_pair(v, iw.first);});
    }

    /**
     * Returns the incoming edges of `v`.
     * *Note:* this is not very efficient, if possible use `inNeighbors` instead.
     */
    auto inEdges(VertexDescriptor v) const requires (Dir == EdgeDirection::Undirected) {
        assert(hasVertex(v));
        return m_vertices.at(v).outNeighbors
            | ranges::views::transform([v, this](auto w) { return *edge(w, v);});
    }

    /**
     * Returns the incoming edges of `v`.
     * *Note:* this is not very efficient, if possible use `inNeighbors` instead.
     */
    auto inEdges(VertexDescriptor v) const requires (Dir == EdgeDirection::Bidirectional) {
        assert(hasVertex(v));
        return m_vertices.at(v).inNeighbors
            | ranges::views::transform([v, this](auto w) { return *edge(w, v);});
    }

    /**
     * Returns the source of the specified edge.
     */
    inline VertexDescriptor source(EdgeDescriptor edge) const {
        return edge.first;
    }

    /**
     * Returns the target of the specified edge.
     */
    inline VertexDescriptor target(EdgeDescriptor edge) const {
        assert(hasVertex(edge.first));
        assert(m_vertices.at(edge.first).outNeighbors.size() > edge.second);
        return m_vertices.at(edge.first).outNeighbors.at(edge.second);
    }

    /**
     * Returns source and target of the specified edge as a pair.
     */
    inline std::pair<VertexDescriptor, VertexDescriptor> endpoints(EdgeDescriptor edge) const {
        return {source(edge), target(edge)};
    }

    /**
     * Returns an invalid vertex descriptor.
     */
    inline static VertexDescriptor nullVertex() {
        return std::numeric_limits<VertexDescriptor>::max();
    }

    /**
     * Returns deg⁺(`v`), i.e. the number of outgoing edges of the specified vertex.
     * *Time Complexity:* O(1) for undirected and bidirectional graphs, O(deg-(v)) for directed graphs.
     */
    inline Unsigned outDegree(VertexDescriptor vertex) const {
        assert(hasVertex(vertex));
        if constexpr (Dir == EdgeDirection::Directed) {
            return ranges::distance(neighbors(vertex));
        } else if constexpr (Dir == EdgeDirection::Undirected) {
            return m_vertices.at(vertex).inDegree;
        } else if constexpr (Dir == EdgeDirection::Bidirectional) {
            return m_vertices.at(vertex).outDegree;
        }
    }

    const auto& neighbors(VertexDescriptor vertex) const requires (!VecGraph::hasEmptyEdgeEntries()) {
        assert(hasVertex(vertex));
        return m_vertices.at(vertex).outNeighbors;
    }

    auto neighbors(VertexDescriptor vertex) const requires (VecGraph::hasEmptyEdgeEntries()) {
        assert(hasVertex(vertex));
        return m_vertices.at(vertex).outNeighbors
               | ranges::views::filter([this](auto w) { return hasVertex(w);});
    }

    inline auto inNeighbors(VertexDescriptor vertex) const requires (Dir == EdgeDirection::Undirected) {
        return neighbors(vertex);
    }

    inline auto inNeighbors(VertexDescriptor vertex) const requires (Dir == EdgeDirection::Bidirectional) {
        assert(hasVertex(vertex));
        return m_vertices.at(vertex).inNeighbors
        | ranges::views::filter([this](auto v) { return hasVertex(v);});
    }

    /**
     * Returns deg⁻(`v`), i.e. the number of incoming edges.
     * For undirected graphs deg⁻(`v`) == deg⁺(`v`) == deg(`v`).
     * *Time Complexity:* O(1)
     * @return deg⁻(`v`)
     */
    inline size_t inDegree(VertexDescriptor vertex) const {
        assert(hasVertex(vertex));
        if constexpr (Dir != EdgeDirection::Undirected) {
            return m_vertices.at(vertex).inDegree;
        } else {
            return degree(vertex);
        }
    }

    /**
     * Returns deg(`v`), i.e. the number of outgoing and incoming edges.
     * @return deg(`v`)
     */
    size_t degree(VertexDescriptor vertex) const requires (Dir == EdgeDirection::Undirected) {
        assert(hasVertex(vertex));
        return m_vertices.at(vertex).inDegree;
    }

    /**
     * Returns all vertices of the graph as a range.
     */
    auto vertices() const {
        return m_vertices | ranges::views::transform([](auto kv) { return kv.first; });
    }

    /**
     * Returns the number of vertices in the graph.
     */
    inline size_t numVertices() const {
        return m_vertices.size();
    }

    /**
     * Returns all edges of the graph as a range.
     */
    auto edges() const requires (Dir == EdgeDirection::Undirected) {
        return vertices()
               | ranges::views::transform([this](Unsigned v) {
            return ranges::views::enumerate(m_vertices.at(v).outNeighbors)
                   | ranges::views::filter([v](auto iw) { return v <= iw.second;})
                   | ranges::views::transform([v](auto iw) { return std::make_pair(v, iw.first);});
        })
               | ranges::views::join;
    }

    /**
     * Returns all edges of the graph as a range.
     */
    auto edges() const requires (Dir != EdgeDirection::Undirected) {
        return vertices()
        | ranges::views::transform([this](Unsigned v) {
            if constexpr (hasEmptyEdgeEntries()) {
                return ranges::views::enumerate(m_vertices.at(v).outNeighbors)
                       | ranges::views::filter([this](auto iw) { return hasVertex(iw.second);})
                       | ranges::views::transform([v](auto iw) { return std::make_pair(v, iw.first);});
            } else {
                return ranges::views::enumerate(m_vertices.at(v).outNeighbors)
                       | ranges::views::transform([v](auto iw) { return std::make_pair(v, iw.first);});
            }
        })
        | ranges::views::join;
    }

    /**
     * Returns the number of edges in the graph.
     */
    size_t numEdges() const {
        return m_numEdges;
    }

    /**
     * Returns an edge descriptor of an edge between `u` and `v`, if present, otherwise returns an empty optional.
     */
    std::optional<EdgeDescriptor> edge(VertexDescriptor u, VertexDescriptor v) const {
        if (!hasVertex(u)) return {};
        auto r = m_vertices.at(u).outNeighbors
            | ranges::views::enumerate
            | ranges::views::filter([this, v](auto w) { return (!hasEmptyEdgeEntries() || hasVertex(w.second)) && w.second == v; });
        if (ranges::empty(r)) {
            return {};
        } else {
            auto edgeIndex = *begin(r);
            return {std::pair{u, edgeIndex.first}};
        }
    }

    /**
     * Returns whether there is an edge from `u`to `v`.
     */
    bool hasEdge(VertexDescriptor u, VertexDescriptor v) const {
        return hasVertex(u) && ranges::contains(neighbors(u), v);
    }

    /**
     * Returns whether `v` is a valid vertex descriptor.
     */
    bool hasVertex(VertexDescriptor v) const {
        return m_vertices.contains(v);
    }

    /**
     * Create a new vertex.
     * Constructs vertex data in-place from the specified arguments.
     *
     * *Time Complexity:* O(1)
     */
    template<typename... T>
    VertexDescriptor addVertex(T&&... args) requires std::is_constructible_v<VertexData, T...> {
        auto idx = numVertices();
        m_vertices.emplace(idx, std::forward<T>(args)...);
        return idx;
    }

    /**
     * Returns the vertex with the specified descriptor.
     * If no such vertex exists, it is constructed in-place with the specified arguments.
     */
    template<typename... T>
    VertexData& getOrAddVertex(VertexDescriptor v, T&&... args) requires (!VecGraph::hasEmptyEdgeEntries() && std::is_constructible_v<VertexData, T...>) {
        auto [it, emplaced] = m_vertices.try_emplace(v, std::forward<T>(args)...);
        unused(emplaced);
        return it->second.data;
    }

    /**
     * Removes the specified vertex.
     * Invalidates edge descriptors of all adjacent vertices.
     *
     * *Time Complexity:* O(\sum_{v\in N(v)} (1+deg⁻(v)))
     */
    void removeVertex(VertexDescriptor vertex) {
        if (hasVertex(vertex)) {
            for (auto w: neighbors(vertex)) {
                --m_vertices.at(w).inDegree;
                --m_numEdges;
                if constexpr (Dir == EdgeDirection::Undirected) {
                    if (vertex != w) {
                        auto it = ranges::find(m_vertices.at(w).outNeighbors, vertex);
                        assert(it != m_vertices.at(w).outNeighbors.end());
                        m_vertices.at(w).outNeighbors.erase(it);
                    }
                } else if constexpr (Dir == EdgeDirection::Bidirectional) {
                    if (vertex != w) {
                        auto it = ranges::find(m_vertices.at(w).inNeighbors, vertex);
                        assert(it != m_vertices.at(w).inNeighbors.end());
                        m_vertices.at(w).inNeighbors.erase(it);
                    }
                }
            }
            if constexpr (Dir == EdgeDirection::Bidirectional) {
                for (auto w: inNeighbors(vertex)) {
                    --m_vertices.at(w).outDegree;
                    m_numEdges -= (w != vertex);
                    auto it = ranges::find(m_vertices.at(w).outNeighbors, vertex);
                    assert(it != m_vertices.at(w).outNeighbors.end());
                    m_vertices.at(w).outNeighbors.erase(it);
                    assert(vertex == w || !ranges::contains(neighbors(w), vertex));
                    assert(vertex == w || !ranges::contains(inNeighbors(w), vertex));
                }
            } else if constexpr (Dir == EdgeDirection::Directed) {
                m_numEdges -= m_vertices.at(vertex).inDegree;
            }
            m_vertices.erase(vertex);
        }
    }

    /**
     * Add an edge from `source` to `target`.
     */
    std::optional<EdgeDescriptor> addEdge(VertexDescriptor source, VertexDescriptor target) {
        if (!hasVertex(source) || !hasVertex(target) || (Simple && hasEdge(source, target))) {
            return {};
        } else {
            auto edgeId = std::make_pair(source, m_vertices.at(source).outNeighbors.size());
            m_vertices.at(source).outNeighbors.template emplace_back(target);
            ++m_numEdges;
            ++m_vertices.at(target).inDegree;
            if constexpr (Dir == EdgeDirection::Bidirectional) {
                m_vertices.at(target).inNeighbors.template emplace_back(source);
                ++m_vertices.at(source).outDegree;
            } else if constexpr (Dir == EdgeDirection::Undirected) {
                if (source != target) {
                    m_vertices.at(target).outNeighbors.template emplace_back(source);
                    ++m_vertices.at(source).inDegree;
                }
            }
            return {edgeId};
        }
    }

    /**
     * Removes an edge fom `u` to `v`, if present.
     * @see removeEdge(EdgeDescriptor)
     */
    void removeEdge(VertexDescriptor u, VertexDescriptor v) {
        auto ed = edge(u, v);
        if (ed) removeEdge(*ed);
    }

    /**
     * Removes the specified edge.
     *
     * *Time Complexity:* O(deg⁺(source(e)) for undirected and bidirectional graphs, O(1) for directed graphs
     */
    void removeEdge(EdgeDescriptor e) {
        auto s = source(e);
        if (!hasVertex(s) || m_vertices.at(s).outNeighbors.size() <= e.second) return;
        auto t = target(e);
        if (hasVertex(s) && hasVertex(t)) {
            m_vertices.at(s).outNeighbors.erase(m_vertices.at(s).outNeighbors.begin() + e.second);
            --m_numEdges;
            --m_vertices.at(t).inDegree;
            if constexpr (Dir == EdgeDirection::Undirected) {
                if (s != t) {
                    auto it = ranges::find(m_vertices.at(t).outNeighbors, s);
                    m_vertices.at(t).outNeighbors.erase(it);
                    --m_vertices.at(s).inDegree;
                }
            } else if constexpr (Dir == EdgeDirection::Bidirectional) {
                auto it = ranges::find(m_vertices.at(t).inNeighbors, s);
                m_vertices.at(t).inNeighbors.erase(it);
                m_vertices.at(s).outDegree -= (s != t);
            }
        }
    }

    /**
     * Returns the data stored in `v`.
     */
    VertexData& getVertex(VertexDescriptor v) {
        assert(hasVertex(v));
        return m_vertices.at(v).data;
    }

    /**
     * Returns the data stored in `v`.
     */
    const VertexData& getVertex(VertexDescriptor v) const {
        assert(hasVertex(v));
        return m_vertices.at(v).data;
    }

    /**
     * Returns the data stored in `v`.
     */
    VertexData& operator[](VertexDescriptor v) {
        assert(hasVertex(v));
        return m_vertices.at(v).data;
    }

    /**
     * Returns the data stored in `v`.
     */
    const VertexData& operator[](VertexDescriptor v) const {
        assert(hasVertex(v));
        return m_vertices.at(v).data;
    }

    /**
     * Reduces the graph size by eliminating allocated empty space between vertices.
     * This operation invalidates all vertex and edge handles.
     */
    void compress() {
        std::vector<Unsigned> prefix;
        size_t countedVertices = 0, maxI = 0, minI = -1;
        for (size_t i = 0; i < m_vertices.capacity() && countedVertices < numVertices(); ++i) {
            if (m_vertices.contains(i)) {
                ++countedVertices;
                prefix.push_back(1);
                maxI = std::max(maxI, i);
                minI = std::min(minI, i);
            } else {
                prefix.push_back(0);
            }
        }
        assert(countedVertices == numVertices());
        if (countedVertices == 0) {
            m_vertices.shrink_to(0);
            return;
        }
        for (size_t i = 1; i < prefix.size(); ++i) {
            prefix[i] += prefix[i - 1];
        }
        assert(prefix.back() == countedVertices);
        for (size_t i = 0; i <= maxI; ++i) {
            if (m_vertices.contains(i)) {
                size_t newKey = prefix[i] - 1;
                auto vertexData = std::move(m_vertices[i]);
                m_vertices.erase(i);
                assert(!m_vertices.contains(newKey));
                assert(!m_vertices.contains(i));
                erase_if(vertexData.outNeighbors, [&prefix, minI, maxI] (auto w) {
                    return w < minI || (w != minI && (w > maxI || prefix[w] == prefix[w - 1]));
                });
                for (auto& w: vertexData.outNeighbors) {
                    w = prefix[w] - 1;
                }
                if constexpr (Dir == EdgeDirection::Bidirectional) {
                    erase_if(vertexData.inNeighbors, [&prefix, minI] (auto w) { return w != minI && prefix[w] == prefix[w - 1]; });
                    for (auto &w: vertexData.inNeighbors) {
                        w = prefix[w] - 1;
                    }
                }
                m_vertices.try_emplace(newKey, std::move(vertexData));
                assert(m_vertices.contains(newKey));
                assert(i == newKey || !m_vertices.contains(i));
                assert(m_vertices.size() == countedVertices);
            }
        }
        if (prefix.empty()) {
            m_vertices.shrink_to(0);
        } else {
            m_vertices.shrink_to(prefix.back());
        }
    }
};
} // namespace mpgraphs
#endif //MPGRAPHS_VECGRAPH_HPP
