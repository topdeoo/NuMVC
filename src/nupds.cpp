#include "nupds.hpp"

#include <cassert>
#include <exception>
#include <iostream>
#include <iterator>
#include <optional>
#include <range/v3/range/conversion.hpp>
#include <utility>
#include <vector>

#include "basic.hpp"
#include "pdsgraph.hpp"
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

std::pair<PDSGraph::Vertex, double> NuPDS::selectVertexToAdd( bool first ) {
    if ( first ) {
        mpgraphs::set<PDSGraph::Vertex>::const_iterator it = add_available_vertices_.begin();
        std::advance( it, random_int( 0, add_available_vertices_.size() ) );
        return { *it, 0 };
    }
    return getMaxObserved();
}

std::pair<PDSGraph::Vertex, double> NuPDS::getMaxObserved() {
    double maxn = 0;
    PDSGraph::Vertex best = *add_available_vertices_.begin();
    for ( auto& v : add_available_vertices_ ) {
        if ( pds_graph_.graph_[v].update ) {
            auto newly_observed = testAddVertex( pds_graph_, v );
            auto score = ( 1 + random_alpha() ) * newly_observed.size();
            if ( score > maxn ) {
                maxn = score;
                best = v;
            }
        }
    }
    return { best, maxn };
}

std::vector<PDSGraph::Vertex> NuPDS::testAddVertex( PDSGraph graph, PDSGraph::Vertex v ) {
    return graph.setDominating( v ) | ranges::to<std::vector<PDSGraph::Vertex>>();
}

// std::pair<NuPDS::Vertex, double> NuPDS::selectVertexToRemove() {
//     auto [v, minn] = *remove_available_vertices_.begin();
//     for ( auto& [w, score] : remove_available_vertices_ ) {
//         if ( score < minn ) {
//             minn = score;
//             v = w;
//         }
//     }
//     return { v, minn };
// }

// // TODO This function has some optimization(check it out later)
// std::pair<NuPDS::Vertex, double> NuPDS::getBestObserver() {
//     double maxn = 0;
//     Vertex best = *add_available_vertices_.begin();

//     for ( auto v : add_available_vertices_ ) {
//         auto prev_graph = graph_;
//         auto prev_deps = dependencies_;
//         auto prev_dominating_count = dominating_count_;
//         auto prev_unobserved_degree = unobserved_degree_;

//         auto newlyObserved = setDominating( v );
//         auto score = ( 1 + random_alpha() ) * newlyObserved.size();
//         if ( score > maxn ) {
//             maxn = score;
//             best = v;
//         }

//         graph_ = std::move( prev_graph );
//         dependencies_ = std::move( prev_deps );
//         dominating_count_ = prev_dominating_count;
//         unobserved_degree_ = std::move( prev_unobserved_degree );
//     }

//     return { best, maxn };
// }

void NuPDS::getClouser( PDSGraph::Vertex vertex, mpgraphs::set<PDSGraph::Vertex>& clouser,
                        mpgraphs::set<PDSGraph::Vertex>& newly_observed ) {
    for ( auto& v : newly_observed ) {
        clouser.insert( v );
        for ( auto& w : pds_graph_.graph_.neighbors( v ) ) {
            if ( !clouser.contains( w ) ) {
                clouser.insert( w );
            }
        }
    }
}

void NuPDS::updateAfterDominating( PDSGraph::Vertex vertex, double score,
                                   mpgraphs::set<PDSGraph::Vertex>& newly_observed ) {
    mpgraphs::set<PDSGraph::Vertex> clouser;
    getClouser( vertex, clouser, newly_observed );
    for ( auto v : add_available_vertices_ ) {
        if ( !pds_graph_.isUpdate( v ) && !pds_graph_.isDominating( v ) ) {
            // Judge whether `v` is effect by `clouser`(i.e. is there any neighbor of `v` in `clouser`)
            for ( auto& w : pds_graph_.graph_.neighbors( v ) ) {
                if ( clouser.contains( w ) ) {
                    pds_graph_.setUpdate( v );
                    break;
                }
            }
        }
    }
    add_available_vertices_.erase( vertex );
    pds_graph_.graph_[vertex].state = VertexState::Domating;
    remove_available_vertices_.insert( { vertex, score } );
}

// void NuPDS::updateAfterRemoving( Vertex vertex ) { remove_available_vertices_.erase( vertex ); }

void NuPDS::GRASP() {
    // 1. Select a vertex randomly and add it into solution
    //      1. 1 update the vertices state within the vertex which is added before
    //      1. 2 update only the clouser of this vertex
    // 2.  Select a vertex with the highest `Ob` value and add into solution until the solution is
    // fealible
    // 3. Use Local Search to remove redundant vertices(Not removing vertices but recursively adding
    // vertices til it is fealible)
    bool first = true;
    while ( !pds_graph_.allObserved() ) {
        auto [v, score] = selectVertexToAdd( first );
        first = false;
        auto newly_observed = pds_graph_.setDominating( v );
        updateAfterDominating( v, score, newly_observed );
        std::cout << "Select Dominating Vertex: " << v << std::endl;
        std::cout << "\tNewly Observed: " << newly_observed.size() << std::endl;
        std::cout << "\tTotal: " << pds_graph_.graph_.numVertices() << std::endl;
        std::cout << "\tObserved: " << pds_graph_.numObserved() << std::endl;
        std::cout << "\tDominating: " << pds_graph_.getDominatingCount() << std::endl;
        if ( first ) {
            first = false;
        }
    }
}

void NuPDS::localSearch() {}

void NuPDS::search() {
    GRASP();
    // NOTE For Debugging
    return;

    // u32 timestramp = 0;
    // while ( timestramp < cutoff_ ) {
    //     if ( allObserved() ) {
    //         best_solution = graph_.vertices() | ranges::views::filter( [this]( auto v ) {
    //                             return graph_[v].state == VertexState::Domating ||
    //                                    graph_[v].state == VertexState::InSured;
    //                         } ) |
    //                         ranges::to<std::vector>();
    //         // TODO random select a vertex to remove
    //         auto index = random_int( 0, remove_available_vertices_.size() );
    //         auto v = remove_available_vertices_.at( index );
    //         removeDominating( v );
    //         continue;
    //     }
    //     // auto [v, _] = selectVertexToRemove();
    //     // removeDominating( v );
    //     // auto [u, score] = selectVertexToAdd();
    //     // setDominating( u );
    //     // updateAfterDominating( u, score );
    //     timestramp += 1;
    // }
}

void NuPDS::init( std::ifstream& fin ) {
    int n, m;
    fin >> n >> m;
    mpgraphs::map<u32, Graph::VertexDescriptor> vertices;
    for ( int v = 0; v < n; v++ ) {
        // u32 v;
        // fin >> v;
        vertices[v] = pds_graph_.graph_.addVertex( Node{ .name = std::to_string( v ),
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
        pds_graph_.graph_.addEdge( getVertex( u ), getVertex( v ) );
    }

    // int k;
    // fin >> k;

    // for ( int i = 0; i < k; i++ ) {
    //     u32 v;
    //     fin >> v;
    //     pds_graph_.graph_.getVertex( getVertex( v ) ).state = VertexState::InSured;
    //     pds_graph_.setDominating( getVertex( v ) );
    // }

    // fin >> k;

    // for ( int i = 0; i < k; i++ ) {
    //     u32 v;
    //     fin >> v;
    //     pds_graph_.graph_.getVertex( getVertex( v ) ).state = VertexState::Exclude;
    //     add_available_vertices_.erase( getVertex( v ) );
    // }

    // fin >> k;

    // for ( int i = 0; i < k; i++ ) {
    //     u32 v;
    //     fin >> v;
    //     pds_graph_.graph_.getVertex( getVertex( v ) ).non_propgating = true;
    // }
}

std::vector<unsigned long> NuPDS::getSolution() {
    return pds_graph_.graph_.vertices() | ranges::views::filter( [this]( auto v ) {
               return pds_graph_.graph_[v].state == VertexState::Domating ||
                      pds_graph_.graph_[v].state == VertexState::InSured;
           } ) |
           ranges::to<std::vector>();
}
