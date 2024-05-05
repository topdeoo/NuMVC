#pragma once

#include "utility.hpp"
#ifndef PDS_HPP
#define PDS_HPP
#include <fstream>
#include <vector>

#include "basic.hpp"
#include "common.hpp"
#include "graph.hpp"
#include "vecgraph.hpp"
#include "vecset.hpp"

enum class VertexState { Blank = 0, Domating = 1, Observed = 2, Exclude = 3, InSured = 4 };

struct Node {
    std::string name;
    u32 id;
    bool non_propgating;
    bool update;
    VertexState state;
};

using Graph = mpgraphs::VecGraph<Node, mpgraphs::EdgeDirection::Undirected, true, u8, u32>;
using DenpenceGraph =
    mpgraphs::VecGraph<mpgraphs::Empty, mpgraphs::EdgeDirection::Bidirectional, true, u8>;

template <typename T>
using VertexMap = mpgraphs::VecMap<Graph::VertexDescriptor, T, u8>;

using VertexSet = mpgraphs::VecSet<Graph::VertexDescriptor, u8>;

using VertexList = std::vector<Graph::VertexDescriptor>;

class NuPDS {
public:
    using Vertex = Graph::VertexDescriptor;

private:
    VertexMap<u32> unobserved_degree_;
    mpgraphs::set<Vertex> add_available_vertices_;
    mpgraphs::map<Vertex, double> remove_available_vertices_;
    u32 dominating_count_ = 0;
    u32 cutoff_ = 1000;
    Graph graph_;
    DenpenceGraph dependencies_;

private:
    void propagate( std::vector<Vertex>& queue, mpgraphs::set<Vertex>& newlyObserved );
    // bool observe( Vertex vertex, Vertex origin );
    bool observeOne( Vertex vertex, Vertex origin, std::vector<Vertex>& queue,
                     mpgraphs::set<Vertex>& newlyObserved );
    std::pair<NuPDS::Vertex, double> selectVertexToAdd( bool first = false );
    std::pair<NuPDS::Vertex, double> selectVertexToRemove();
    std::pair<NuPDS::Vertex, double> getBestObserver();

public:
    NuPDS();
    explicit NuPDS( const Graph& graph );
    explicit NuPDS( Graph&& graph );

    NuPDS( const NuPDS& ) = default;
    NuPDS( NuPDS&& ) = default;
    NuPDS& operator=( NuPDS&& ) = default;
    NuPDS& operator=( const NuPDS& ) = default;

    void addEdge( Vertex source, Vertex target );
    void removeVertex( Vertex v );

    mpgraphs::set<Vertex> setDominating( Vertex vertex );
    void updateAfterDominating( Vertex vertex, double score, mpgraphs::set<Vertex>& newlyObserved );
    void getClouser( Vertex vertex, mpgraphs::set<Vertex>& clouser,
                     mpgraphs::set<Vertex>& newlyObserved );
    bool removeDominating( Vertex vertex );
    void updateAfterRemoving( Vertex vertex );

    inline bool isObserved( Vertex v ) const { return dependencies_.hasVertex( v ); }
    inline bool isDominating( Vertex v ) const { return graph_[v].state == VertexState::Domating; }
    inline bool isInSured( Vertex v ) const { return graph_[v].state == VertexState::InSured; }
    inline bool isBlack( Vertex v ) const { return graph_[v].state == VertexState::Blank; }
    inline bool isNonPropagating( Vertex v ) const { return graph_[v].non_propgating; }
    inline bool isUpdate( Vertex v ) const { return graph_[v].update; }
    inline void setUpdate( Vertex v ) { graph_[v].update = true; }
    inline bool isObervingEdge( Vertex source, Vertex target ) const {
        return dependencies_.hasEdge( source, target );
    }
    inline u32 unobservedDegree( Vertex v ) const { return unobserved_degree_.at( v ); }
    inline u32 numObserved() const { return dependencies_.numVertices(); }
    inline bool allObserved() const { return numObserved() == graph_.numVertices(); }

public:
    void init( std::ifstream& fin );
    void GRASP();
    void localSearch();
    void search();
    inline u32 getDominatingCount() const { return dominating_count_; }
    std::vector<unsigned long> getSolution();
};
#endif  // PDS_HPP