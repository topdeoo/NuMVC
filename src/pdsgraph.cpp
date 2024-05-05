#include "pdsgraph.hpp"

#include <cassert>
#include <exception>
#include <iostream>
#include <optional>
#include <range/v3/range/conversion.hpp>
#include <utility>
#include <vector>

PDSGraph::PDSGraph( PDSGraph& graph )
    : unobserved_degree_( graph.unobserved_degree_ ),
      dominating_count_( graph.dominating_count_ ),
      graph_( graph.graph_ ),
      dependencies_( graph.dependencies_ ) {}

PDSGraph::PDSGraph( const PDSGraph& graph )
    : unobserved_degree_( graph.unobserved_degree_ ),
      dominating_count_( graph.dominating_count_ ),
      graph_( graph.graph_ ),
      dependencies_( graph.dependencies_ ) {}

void PDSGraph::addEdge( Vertex source, Vertex target ) {
    assert( source != target );
    if ( !graph_.edge( source, target ) ) {
        graph_.addEdge( source, target );
        if ( !isObserved( source ) ) {
            unobserved_degree_[target] += 1;
        }
        if ( !isObserved( target ) ) {
            unobserved_degree_[source] += 1;
        }
    }
}

void PDSGraph::removeVertex( Vertex v ) {
    if ( !isObserved( v ) ) {
        for ( auto& w : graph_.neighbors( v ) ) {
            unobserved_degree_[w] -= 1;
        }
    }
    dependencies_.removeVertex( v );
    graph_.removeVertex( v );
    unobserved_degree_.erase( v );
}

void PDSGraph::propagate( std::vector<Vertex>& queue, mpgraphs::set<Vertex>& newlyObserved ) {
    while ( !queue.empty() ) {
        auto v = queue.back();
        queue.pop_back();
        if ( isObserved( v ) && !isNonPropagating( v ) && unobserved_degree_[v] == 1 ) {
            for ( auto& w : graph_.neighbors( v ) ) {
                if ( !isObserved( w ) ) {
                    observeOne( w, v, queue, newlyObserved );
                }
            }
        }
    }
}

bool PDSGraph::observeOne( Vertex vertex, Vertex origin, std::vector<Vertex>& queue,
                           mpgraphs::set<Vertex>& newlyObserved ) {
    if ( !isObserved( vertex ) ) {
        dependencies_.getOrAddVertex( vertex );
        newlyObserved.insert( vertex );
        graph_[vertex].state = VertexState::Observed;
        if ( origin != vertex ) {
            dependencies_.addEdge( origin, vertex );
        }
        if ( unobserved_degree_[vertex] == 1 ) {
            queue.push_back( vertex );
        }
        for ( auto& w : graph_.neighbors( vertex ) ) {
            unobserved_degree_[w] -= 1;
            if ( unobserved_degree_[w] == 1 && isObserved( w ) && !isNonPropagating( w ) ) {
                queue.push_back( w );
            }
        }
        return true;
    } else {
        return false;
    }
}

mpgraphs::set<PDSGraph::Vertex> PDSGraph::setDominating( Vertex vertex ) {
    mpgraphs::set<Vertex> newlyObserved;
    if ( !isDominating( vertex ) ) {
        graph_[vertex].state = VertexState::Domating;
        dominating_count_++;
        if ( dependencies_.hasVertex( vertex ) ) {
            while ( dependencies_.inDegree( vertex ) > 0 ) {
                auto e = *dependencies_.inEdges( vertex ).begin();
                dependencies_.removeEdge( e );
            }
        }
        std::vector<Vertex> queue;
        observeOne( vertex, vertex, queue, newlyObserved );
        for ( auto w : graph_.neighbors( vertex ) ) {
            observeOne( w, vertex, queue, newlyObserved );
        }
        propagate( queue, newlyObserved );
    }
    return newlyObserved;
}

bool PDSGraph::removeDominating( Vertex vertex ) {
    mpgraphs::set<Vertex> newlyObserved;
    if ( isDominating( vertex ) ) {
        std::vector<Vertex> queue;
        std::vector<Vertex> propagating;
        mpgraphs::set<Vertex> enqueued;

        graph_[vertex].state = VertexState::Blank;
        dominating_count_--;
        queue.push_back( vertex );
        enqueued.insert( vertex );
        while ( !queue.empty() ) {
            auto v = queue.back();
            queue.pop_back();
            std::optional<Vertex> observer;
            for ( auto& w : graph_.neighbors( v ) ) {
                unobserved_degree_[w] += 1;
                if ( !isObserved( w ) ) {
                    if ( dependencies_.hasEdge( v, w ) ) {
                        if ( !enqueued.contains( w ) ) {
                            queue.push_back( w );
                            enqueued.insert( w );
                        }
                    } else if ( isObserved( w ) ) {
                        for ( auto& u : graph_.neighbors( w ) ) {
                            if ( !enqueued.contains( u ) ) {
                                queue.push_back( u );
                                enqueued.insert( u );
                            }
                        }
                        propagating.push_back( w );
                    }
                } else {
                    observer = { w };
                }
            }
            dependencies_.removeVertex( v );
            if ( observer.has_value() ) {
                observeOne( v, observer.value(), propagating, newlyObserved );
            }
        }
        propagate( propagating, newlyObserved );
    }
    return isObserved( vertex );
}