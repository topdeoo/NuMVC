//
// Created by max on 14.10.22.
//
#ifndef MPGRAPHS_MAPGRAPH_HPP
#define MPGRAPHS_MAPGRAPH_HPP

#include <optional>

#include "common.hpp"

namespace mpgraphs {

namespace detail {
/**
 * vertex list entry.
 */
template <class VertexData, class VertexKey>
struct VertexEntry {
    /// vertex data
    VertexData data;
    /// out neighbors
    set<VertexKey> out_edges;
    /// Entry with empty edges
    template <class... T>
    VertexEntry( T... args ) : data( std::forward<T>( args )... ) {}
};

/**
 * vertex list entry with in edge list.
 */
template <class VertexData, class VertexKey>
struct BidirectionalVertexEntry {
    /// vertex data
    VertexData data;
    /// out neighbors
    set<VertexKey> out_edges;
    /// in neighbors
    set<VertexKey> in_edges;
    /// Entry with empty edges
    template <class... T>
    BidirectionalVertexEntry( T... args ) : data( std::forward<T>( args )... ) {}
};
}  // namespace detail

/**
 * Powerful graph class that supports fast iteration and modification.
 * The vertex and adjacency lists are backed by hash maps which support
 * edge and vertex lookups in constant time and enable fast iteration.
 *
 * Available variants are directed, bidirectional and undirected graphs.
 * The bidirectional variant can enumerate incoming edges and allows for faster
 * vertex deletion at the cost of storing all incoming edges of each vertex.
 * These features are also available in undirected graphs but not in directed
 * graphs.
 * The graph type is determined by the Dir template argument.
 *
 * Vertices and edges can hold additional data.
 * Use VertexData and EdgeData to specify what to store in the respective entries.
 *
 * Both VertexDescriptor and EdgeDescriptor are stable under modification.
 * @tparam VertexData vertex data
 * @tparam EdgeData edge data
 * @tparam Dir graph type
 * @tparam VertexKey vertex key
 */
template <class VertexData = Empty, class EdgeData = Empty, EdgeDirection Dir = EdgeDirection::Directed,
          class VertexKey = size_t>
class MapGraph {
public:
    /**
     * Unique vertex identifier.
     */
    using VertexDescriptor = VertexKey;
    /**
     * Unique edge identifier.
     * Note that two different edge descriptors can refer to the same edge.
     */
    using EdgeDescriptor = std::pair<VertexKey, VertexKey>;

    constexpr static EdgeDirection Direction = Dir;

private:
    using VertexEntry = std::conditional_t<Dir == EdgeDirection::Bidirectional,
                                           detail::BidirectionalVertexEntry<VertexData, VertexKey>,
                                           detail::VertexEntry<VertexData, VertexKey> >;
    size_t m_next_vertex;
    map<VertexKey, VertexEntry> m_vertices;
    map<EdgeDescriptor, EdgeData> m_edges;

    inline EdgeDescriptor normalize_edge( EdgeDescriptor edge ) const {
        if constexpr ( Dir == EdgeDirection::Undirected ) {
            return std::minmax( edge.first, edge.second );
        } else {
            return edge;
        }
    }

public:
    /**
     * Creates an empty graph.
     */
    MapGraph() : m_next_vertex( 0 ), m_vertices{}, m_edges{} {};

    /**
     * Copies `other`
     * @param other graph to copy
     */
    MapGraph( const MapGraph& other ) = default;

    /**
     * Move constructor.
     * @param other
     */
    MapGraph( MapGraph&& other ) noexcept = default;

    MapGraph& operator=( const MapGraph& other ) = default;
    MapGraph& operator=( MapGraph&& other ) noexcept = default;

    // template<class... T>
    // WrappedGraph(T&&... args) : m_graph(std::forward(args)...) { }

    /**
     * @return whether the graph is directed.
     */
    static constexpr bool is_directed() { return Dir != EdgeDirection::Undirected; }

    /**
     * Returns the out-edges of `v` as range.
     * While {u,v} and {v,u} are considered the same in undirected graphs,
     * the source of the enumerated edge descriptors is always `v`
     * and its neighbor is always the target.
     */
    auto outEdges( VertexDescriptor v ) const {
        return m_vertices.at( v ).out_edges |
               ranges::views::transform( [v]( VertexDescriptor w ) { return std::make_pair( v, w ); } );
    }
    // using out_edges_range =
    // decltype(std::declval<MapGraph>().outEdges(std::declval<VertexDescriptor>())); using
    // out_edge_iterator = decltype(begin(std::declval<out_edges_range>()));

    /**
     * Returns the source vertex of `edge`.
     */
    VertexDescriptor source( EdgeDescriptor edge ) const { return edge.first; }

    /**
     * Returns the target vertex of `edge`.
     */
    VertexDescriptor target( EdgeDescriptor edge ) const { return edge.second; }

    /**
     * Returns the source and target of `edge`.
     */
    std::pair<VertexDescriptor, VertexDescriptor> endpoints( EdgeDescriptor edge ) const {
        return { edge.first, edge.second };
    }

    /**
     * @return an invalid vertex descriptor.
     */
    static VertexDescriptor nullVertex() { return { -1 }; }

    /**
     * Returns deg⁺(`v`), i.e. the number of outgoing edges.
     * In undirected graphs deg⁺(`v`) == deg⁻(`v`) == deg(`v`).
     * @return deg⁺(`v`)
     */
    size_t outDegree( VertexDescriptor v ) const { return m_vertices.at( v ).out_edges.size(); }

    /**
     * @return the incoming edges of `v`.
     */
    auto inEdges( VertexDescriptor v ) const requires( Dir == EdgeDirection::Bidirectional ) {
        return m_vertices.at( v ).in_edges |
               ranges::views::transform( [v]( VertexDescriptor w ) { return std::make_pair( w, v ); } );
    }

    /**
     * Returns the incoming edges of `v`.
     * In an undirected graph this is the same as the outgoing edges, except that source and target are
     * swapped. For inEdges, `v` is  the target and the neighbor is the source.
     */
    auto inEdges( VertexDescriptor v ) const requires( Dir == EdgeDirection::Undirected ) {
        return m_vertices.at( v ).out_edges |
               ranges::views::transform( [v]( VertexDescriptor w ) { return std::make_pair( w, v ); } );
    }

    /**
     * Returns deg⁻(`v`), i.e. the number of incoming edges.
     * For undirected graphs deg⁻(`v`) == deg⁺(`v`) == deg(`v`).
     * @return deg⁻(`v`)
     */
    size_t inDegree( VertexDescriptor v ) const requires( Dir == EdgeDirection::Bidirectional ) {
        return m_vertices.at( v ).in_edges.size();
    }

    /**
     * Returns deg⁻(`v`), i.e. the number of incoming edges.
     * For undirected graphs deg⁻(`v`) == deg⁺(`v`) == deg(`v`).
     * @return deg⁻(`v`)
     */
    size_t inDegree( VertexDescriptor v ) const requires( Dir == EdgeDirection::Undirected ) {
        return outDegree( v );
    }

    /**
     * Returns deg(`v`), i.e. the number of outgoing and incoming edges.
     * For directed graphs deg(`v`) == deg⁻(`v`) == deg⁺(`v`).
     * @return deg(`v`)
     */
    size_t degree( VertexDescriptor v ) const requires( Dir == EdgeDirection::Bidirectional ) {
        return m_vertices.at( v ).out_edges.size() + m_vertices.at( v ).in_edges.size();
    }

    /**
     * Returns deg(`v`), i.e. the number of incident edges.
     * For directed graphs deg(`v`) == deg⁻(`v`) == deg⁺(`v`).
     * @return deg(`v`)
     */
    size_t degree( VertexDescriptor v ) const requires( Dir == EdgeDirection::Undirected ) {
        return m_vertices.at( v ).out_edges.size();
    }

    // using in_edges_range =
    // decltype(std::declval<MapGraph>().inEdges(std::declval<VertexDescriptor>())); using
    // in_edge_iterator = decltype(begin(std::declval<in_edges_range>()));

    /**
     * Returns the neighborhood N(`v`) or N⁺(`v`) of `v` as range.
     * For directed graphs these are the targets of the outgoing edges.
     * @return N(v)
     */
    const set<VertexDescriptor>& neighbors( VertexDescriptor vertex ) const {
        return m_vertices.at( vertex ).out_edges;
    }

    /**
     * Returns the in-neighborhood N⁻(`v`) of `v` as range.
     * For directed graphs these are the sources of the outgoing edges.
     * Not available for `Directed` graphs.
     * @return N(v)
     */
    const set<VertexDescriptor>& inNeighbors( VertexDescriptor vertex ) const
        requires( Dir == EdgeDirection::Undirected ) {
        return m_vertices.at( vertex ).out_edges;
    }

    /**
     * Returns the in-neighborhood N⁻(`v`) of `v` as range.
     * For directed graphs these are the sources of the outgoing edges.
     * Not available for `Directed` graphs.
     * @return N(v)
     */
    const set<VertexDescriptor>& inNeighbors( VertexDescriptor vertex ) const
        requires( Dir == EdgeDirection::Bidirectional ) {
        return m_vertices.at( vertex ).in_edges;
    }

    // using neighbor_range = set<VertexDescriptor>;
    // using neighbor_iterator = decltype(begin(std::declval<neighbor_range>()));
    // using adjacency_iterator = neighbor_iterator; // alias used by boost graph

    /**
     * Returns all vertices of the graph as a range.
     */
    auto vertices() const {
        return ranges::transform_view( m_vertices, []( const auto& pair ) { return pair.first; } );
    }
    // using vertex_range = decltype(std::declval<MapGraph>().vertices());
    // using vertex_iterator = decltype(std::begin(std::declval<MapGraph::vertex_range>()));

    /**
     * @return the number of vertices.
     */
    size_t numVertices() const { return m_vertices.size(); }

    /**
     * @return all edges of the graph as a range.
     */
    auto edges() const {
        return m_edges | ranges::views::transform( []( const auto& item ) { return item.first; } );
    }

    /**
     * @return the number of edges.
     */
    size_t numEdges() const { return m_edges.size(); }
    // using edge_range = decltype(std::declval<MapGraph>().edges());
    // using edge_iterator = decltype(std::begin(std::declval<MapGraph::edge_range>()));

    /**
     * Returns an empty optional if no edge from u to v exists, otherwise returns
     * the edge descriptor of that edge.
     */
    std::optional<EdgeDescriptor> edge( VertexDescriptor u, VertexDescriptor v ) const {
        if ( m_edges.contains( normalize_edge( { u, v } ) ) ) {
            return { { u, v } };
        } else {
            return {};
        }
    }

    /**
     * Returns whether there is an edge from u to v.
     */
    bool hasEdge( VertexDescriptor u, VertexDescriptor v ) const {
        return m_edges.contains( normalize_edge( { u, v } ) );
    }

    /**
     * Returns whether the specified vertex exists.
     */
    bool hasVertex( VertexDescriptor v ) const { return m_vertices.contains( v ); }

    /**
     * Creates a new vertex constructed with the data passed as argument.
     * The arguments should be valid constructor arguments for the edge data.
     * @return descriptor of the new vertex
     */
    template <typename... T>
    VertexDescriptor addVertex( T&&... args ) {
        VertexKey id = m_next_vertex++;  // todo
        m_vertices.template emplace( id, VertexEntry( std::forward<T>( args )... ) );
        return id;
    }

    template <typename... T>
    VertexData& getOrAddVertex( VertexDescriptor v, T&&... args ) {
        m_next_vertex = std::max( m_next_vertex, v + 1 );
        m_vertices.template try_emplace( v, VertexEntry( std::forward<T>( args )... ) );
        return m_vertices.at( v ).data;
    }

    const VertexData& getVertex( VertexDescriptor v ) const { return m_vertices.at( v ).data; }

    VertexData& getVertex( VertexDescriptor v ) { return m_vertices.at( v ).data; }

    /**
     * Removes all incident edges of `v`.
     * For directed graphs, both ingoing and outgoing edges are removed.
     * The time complexity depends on the type of graph:
     * - `Directed`: O(m)
     * - `Bidirectional`: O(deg⁺(`v`) + deg⁻(`v`))
     * - `Undirected`: O(deg(`v`))
     */
    void clearVertex( VertexDescriptor v ) {
        auto& vertex = m_vertices.at( v );
        if constexpr ( Dir == EdgeDirection::Directed ) {
            erase_if( m_edges, [v, this]( const auto& edge ) {
                if ( source( edge.first ) == v ) {
                    return true;
                } else if ( target( edge.first ) == v ) {
                    m_vertices[source( edge.first )].out_edges.erase( v );
                    return true;
                }
                return false;
            } );
        } else {
            for ( auto w : vertex.out_edges ) {
                m_edges.erase( normalize_edge( { v, w } ) );
                if constexpr ( Dir == EdgeDirection::Bidirectional ) {
                    m_vertices[w].in_edges.erase( v );
                } else if constexpr ( Dir == EdgeDirection::Undirected ) {
                    m_vertices[w].out_edges.erase( v );
                }
            }
            if constexpr ( Dir == EdgeDirection::Bidirectional ) {
                for ( auto w : vertex.in_edges ) {
                    m_edges.erase( normalize_edge( { w, v } ) );
                    m_vertices[w].out_edges.erase( v );
                }
            }
        }
        vertex.out_edges.clear();
        if constexpr ( Dir == EdgeDirection::Bidirectional ) {
            vertex.in_edges.clear();
        }
    }

    /**
     * Removes `vertex` from the graph.
     * Also removes all incident edges.
     * See clearVertex for details on the running time.
     */
    void removeVertex( VertexDescriptor vertex ) {
        auto it = m_vertices.find( vertex );
        if ( it != end( m_vertices ) ) {
            clearVertex( vertex );
            m_vertices.erase( it );
        }
    }

    /**
     * Adds an edges.
     * The arguments should be valid constructor arguments for the edge data.
     * If the edge already exists, the method returns an empty optional.
     * Otherwise the descriptor of newly inserted edge is returned.
     * The data is written in both cases.
     * @param u edege source
     * @param v edege target
     * @param args edge data
     * @return descriptor if the edge is new
     */
    template <typename... T>
    std::optional<EdgeDescriptor> addEdge( VertexDescriptor u, VertexDescriptor v, T&&... args ) {
        auto edge = normalize_edge( { u, v } );
        if ( m_edges.contains( edge ) ) {
            m_edges.template emplace( edge, EdgeData{ std::forward( args )... } );
            return {};
        } else {
            m_edges.template emplace( edge, EdgeData{ std::forward( args )... } );
            m_vertices.at( u ).out_edges.template emplace( v );
            if constexpr ( Dir == EdgeDirection::Undirected ) {
                m_vertices.at( v ).out_edges.template emplace( u );
            } else if constexpr ( Dir == EdgeDirection::Bidirectional ) {
                m_vertices.at( v ).in_edges.template emplace( u );
            }
            return { { u, v } };
        }
    }

    /**
     * Removes the edge from u to v.
     */
    void removeEdge( VertexDescriptor u, VertexDescriptor v ) { removeEdge( EdgeDescriptor{ u, v } ); }

    /**
     * Removes the edges described by e
     */
    void removeEdge( EdgeDescriptor e ) {
        auto it = m_edges.find( normalize_edge( e ) );
        if ( it != end( m_edges ) ) {
            m_vertices.at( e.first ).out_edges.erase( e.second );
            if constexpr ( Dir == EdgeDirection::Undirected ) {
                m_vertices.at( e.second ).out_edges.erase( e.first );
            } else if constexpr ( Dir == EdgeDirection::Bidirectional ) {
                m_vertices.at( e.second ).in_edges.erase( e.first );
            }
            m_edges.erase( it );
        }
    }

    ///**
    // * Removes the edge pointed to by edge_it.
    // */
    // void removeEdge(out_edge_iterator edge_it) {
    //    removeEdge(*edge_it);
    //}

    /**
     * Acess vertex data.
     */
    VertexData& operator[]( VertexDescriptor vertex ) { return m_vertices.at( vertex ).data; }

    /**
     * Acess vertex data immutably.
     */
    const VertexData& operator[]( VertexDescriptor vertex ) const {
        return m_vertices.at( vertex ).data;
    }

    /**
     * Access edge data.
     */
    EdgeData& operator[]( EdgeDescriptor vertex ) { return m_edges.at( vertex ).data; }

    /**
     * Access edge data immutably.
     */
    const EdgeData& operator[]( EdgeDescriptor edge ) const { return m_edges.at( edge ); }
};
}  // namespace mpgraphs

#endif  // MPGRAPHS_MAPGRAPH_HPP
