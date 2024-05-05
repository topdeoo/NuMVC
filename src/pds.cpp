#include "pds.hpp"

#include <cassert>
#include <exception>
#include <iostream>
#include <iterator>
#include <optional>
#include <range/v3/range/conversion.hpp>
#include <utility>
#include <vector>

#include "basic.hpp"
#include "utility.hpp"

static std::random_device rd{ "hw" };

double random_alpha() {
    static auto gen =
        std::bind( std::uniform_real_distribution<>( 0, 1 ), std::default_random_engine( rd() ) );
    return gen();
}

u32 random_int( u32 l, u32 r ) {
    static auto gen =
        std::bind( std::uniform_int_distribution<u32>( l, r ), std::default_random_engine( rd() ) );
    return gen();
}

NuPDS::NuPDS() : NuPDS( Graph() ) {}

NuPDS::NuPDS( const Graph& graph ) : NuPDS( Graph{ graph } ) {}

NuPDS::NuPDS( Graph&& graph ) {
    // TODO copy all data
}

void NuPDS::addEdge( Vertex source, Vertex target ) {
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

void NuPDS::removeVertex( Vertex v ) {
    if ( !isObserved( v ) ) {
        for ( auto& w : graph_.neighbors( v ) ) {
            unobserved_degree_[w] -= 1;
        }
    }
    dependencies_.removeVertex( v );
    graph_.removeVertex( v );
    unobserved_degree_.erase( v );
}

void NuPDS::propagate( std::vector<Vertex>& queue, mpgraphs::set<Vertex>& newlyObserved ) {
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

// bool NuPDS::observe( Vertex vertex, Vertex origin ) {
//     std::vector<Vertex> queue;
//     if ( observeOne( vertex, origin, queue ) ) {
//         propagate( queue );
//         return true;
//     } else {
//         return false;
//     }
// }

bool NuPDS::observeOne( Vertex vertex, Vertex origin, std::vector<Vertex>& queue,
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

mpgraphs::set<NuPDS::Vertex> NuPDS::setDominating( Vertex vertex ) {
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

bool NuPDS::removeDominating( Vertex vertex ) {
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

std::pair<NuPDS::Vertex, double> NuPDS::selectVertexToAdd( bool first ) {
    if ( first ) {
        mpgraphs::set<Vertex>::const_iterator it = add_available_vertices_.begin();
        std::advance( it, random_int( 0, add_available_vertices_.size() ) );
        return { *it, 0 };
    }
    return getBestObserver();
}

std::pair<NuPDS::Vertex, double> NuPDS::selectVertexToRemove() {
    auto [v, minn] = *remove_available_vertices_.begin();
    for ( auto& [w, score] : remove_available_vertices_ ) {
        if ( score < minn ) {
            minn = score;
            v = w;
        }
    }
    return { v, minn };
}

// TODO This function has some optimization(check it out later)
std::pair<NuPDS::Vertex, double> NuPDS::getBestObserver() {
    double maxn = 0;
    Vertex best = *add_available_vertices_.begin();

    for ( auto v : add_available_vertices_ ) {
        auto prev_graph = graph_;
        auto prev_deps = dependencies_;
        auto prev_dominating_count = dominating_count_;
        auto prev_unobserved_degree = unobserved_degree_;

        auto newlyObserved = setDominating( v );
        auto score = ( 1 + random_alpha() ) * newlyObserved.size();
        if ( score > maxn ) {
            maxn = score;
            best = v;
        }

        graph_ = std::move( prev_graph );
        dependencies_ = std::move( prev_deps );
        dominating_count_ = prev_dominating_count;
        unobserved_degree_ = std::move( prev_unobserved_degree );
    }

    return { best, maxn };
}

void NuPDS::getClouser( Vertex vertex, mpgraphs::set<Vertex>& clouser,
                        mpgraphs::set<Vertex>& newlyObserved ) {
    for ( auto& v : newlyObserved ) {
        clouser.insert( v );
        for ( auto& w : graph_.neighbors( v ) ) {
            if ( !clouser.contains( w ) ) {
                clouser.insert( w );
            }
        }
    }
}

void NuPDS::updateAfterDominating( Vertex vertex, double score, mpgraphs::set<Vertex>& newlyObserved ) {
    mpgraphs::set<Vertex> clouser;
    getClouser( vertex, clouser, newlyObserved );
    for ( auto v : add_available_vertices_ ) {
        if ( !isUpdate( v ) && !isDominating( v ) ) {
            // Judge whether `v` is effect by `clouser`(i.e. is there any neighbor of `v` in `clouser`)
            for ( auto& w : graph_.neighbors( v ) ) {
                if ( clouser.contains( w ) ) {
                    setUpdate( v );
                    break;
                }
            }
        }
    }
    add_available_vertices_.erase( vertex );
    graph_[vertex].state = VertexState::Domating;
    remove_available_vertices_.insert( { vertex, score } );
}

void NuPDS::updateAfterRemoving( Vertex vertex ) { remove_available_vertices_.erase( vertex ); }

void NuPDS::GRASP() {
    // 1. Select a vertex randomly and add it into solution
    //      1. 1 update the vertices state within the vertex which is added before
    //      1. 2 update only the clouser of this vertex
    // 2.  Select a vertex with the highest `Ob` value and add into solution until the solution is
    // fealible
    // 3. Use Local Search to remove redundant vertices(Not removing vertices but recursively adding
    // vertices til it is fealible)
    bool first = true;
    while ( !allObserved() ) {
        if ( first ) {
            // Insure we do not choose vertex.state = InSured or Excluded
            auto [v, score] = selectVertexToAdd( first );
            auto newlyObserved = setDominating( v );
            first = false;
            score = ( 1 + random_alpha() ) * ( newlyObserved.size() );
            updateAfterDominating( v, score, newlyObserved );
            continue;
        }
        auto [v, score] = selectVertexToAdd();
        auto newlyObserved = setDominating( v );
        updateAfterDominating( v, score, newlyObserved );
        std::cout << "Select Dominating Vertex: " << v << std::endl;
        std::cout << "\tNewly Observed: " << newlyObserved.size() << std::endl;
        std::cout << "\tObserved: " << dependencies_.numVertices() << std::endl;
        std::cout << "\tTotal: " << graph_.numVertices() << std::endl;
        std::cout << "\tDominating: " << dominating_count_ << std::endl;
    }
}

void NuPDS::localSearch() {}

void NuPDS::search() {
    GRASP();
    // NOTE For Debugging
    return;

    auto best_solution =
        graph_.vertices() | ranges::views::filter( [this]( auto v ) {
            return graph_[v].state == VertexState::Domating || graph_[v].state == VertexState::InSured;
        } ) |
        ranges::to<std::vector>();
    u32 timestramp = 0;
    while ( timestramp < cutoff_ ) {
        if ( allObserved() ) {
            best_solution = graph_.vertices() | ranges::views::filter( [this]( auto v ) {
                                return graph_[v].state == VertexState::Domating ||
                                       graph_[v].state == VertexState::InSured;
                            } ) |
                            ranges::to<std::vector>();
            // TODO random select a vertex to remove
            auto index = random_int( 0, remove_available_vertices_.size() );
            auto v = remove_available_vertices_.at( index );
            removeDominating( v );
            continue;
        }
        // auto [v, _] = selectVertexToRemove();
        // removeDominating( v );
        // auto [u, score] = selectVertexToAdd();
        // setDominating( u );
        // updateAfterDominating( u, score );
        timestramp += 1;
    }
}

void NuPDS::init( std::ifstream& fin ) {
    int n, m;
    fin >> n >> m;
    mpgraphs::map<u32, Graph::VertexDescriptor> vertices;
    for ( int v = 0; v < n; v++ ) {
        // u32 v;
        // fin >> v;
        vertices[v] = graph_.addVertex( Node{ .name = std::to_string( v ),
                                              .id = static_cast<u32>( v ),
                                              .non_propgating = false,
                                              .update = false,
                                              .state = VertexState::Blank } );
        add_available_vertices_.insert( vertices[v] );
    }
    auto getVertex = [&vertices]( auto i ) {
        if ( !vertices.contains( i ) ) throw std::exception();
        return vertices[i];
    };

    for ( int i = 0; i < m; i++ ) {
        u32 u, v;
        fin >> u >> v;
        graph_.addEdge( getVertex( u ), getVertex( v ) );
    }

    // int k;
    // fin >> k;

    // for ( int i = 0; i < k; i++ ) {
    //     u32 v;
    //     fin >> v;
    //     graph_.getVertex( getVertex( v ) ).state = VertexState::InSured;
    //     setDominating( getVertex( v ) );
    // }

    // fin >> k;

    // for ( int i = 0; i < k; i++ ) {
    //     u32 v;
    //     fin >> v;
    //     graph_.getVertex( getVertex( v ) ).state = VertexState::Exclude;
    //     add_available_vertices_.erase( getVertex( v ) );
    // }

    // fin >> k;

    // for ( int i = 0; i < k; i++ ) {
    //     u32 v;
    //     fin >> v;
    //     graph_.getVertex( getVertex( v ) ).non_propgating = true;
    // }
}

std::vector<unsigned long> NuPDS::getSolution() {
    return graph_.vertices() | ranges::views::filter( [this]( auto v ) {
               return graph_[v].state == VertexState::Domating;
           } ) |
           ranges::to<std::vector>();
}
