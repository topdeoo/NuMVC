#include "pds.hpp"

#include <cstdint>
#include <iostream>
#include <optional>
#include <stack>
#include <utility>

#include "basic.hpp"
#include "graph.hpp"
#include "heap.h"

void NuPDS::init( std::ifstream &fin ) {
    u32 n, m;
    fin >> n >> m;

    for ( u32 i = 0; i < m; i++ ) {
        u32 from, to;
        fin >> from >> to;
        graph_.add_edge( from, to );
        graph_.add_edge( to, from );
        vertices_state_.insert( { from, { false, false, false, false } } );
        vertices_state_.insert( { to, { false, false, false, false } } );
        available_candidates_.insert( from );
        available_candidates_.insert( to );
        non_observed_.insert( from );
        non_observed_.insert( to );
    }

    u32 k;
    fin >> k;
    for ( u32 i = 0; i < k; i++ ) {
        u32 v;
        fin >> v;
        pre_selected_.insert( v );
        vertices_state_[v].used = true;
        vertices_state_[v].pre_selected = true;
        available_candidates_.erase( v );
    }

    u32 l;
    fin >> l;
    for ( u32 i = 0; i < l; i++ ) {
        u32 v;
        fin >> v;
        excluded_.insert( v );
        vertices_state_[v].excluded = true;
        available_candidates_.erase( v );
    }

    u32 p;
    fin >> p;
    for ( u32 i = 0; i < p; i++ ) {
        u32 v;
        fin >> v;
        non_propagating_.insert( v );
    }

    timestamp_ = 0;
    tabu_size_ = 10;
    alpha_ = 0.5;

    for ( auto &v : graph_.vertices() ) {
        unobserved_degree_[v] = graph_.degree( v );
        age[v] = 0;
        cc_[v] = true;
        tabu_[v] = 0;
    }

    // TODO
    cutoff_ = 100;

    // init_remove_set( n, graph_ );
}

void NuPDS::pre_process() {
    timestamp_ = 0;

    for ( auto &v : pre_selected_ ) {
        add_into_solution( { v, -1 }, false );
        tabu_[v] = 0;
    }
}

void NuPDS::observe_one( u32 vertex, u32 origin, std::stack<u32> &stack_ ) {
    if ( !is_observed( vertex ) ) {
        dependencies_.add_vertex( vertex );
        non_observed_.erase( vertex );
        if ( origin != vertex ) dependencies_.add_edge( origin, vertex );
        if ( unobserved_degree_[vertex] == 1 ) stack_.push( vertex );
        for ( auto w : graph_.get_out_neighbors( vertex ) ) {
            cc_[w] = true;
            unobserved_degree_[w] -= 1;
            if ( unobserved_degree_[w] == 1 && is_observed( w ) && can_propagate( w ) )
                stack_.push( w );
        }
    }
}

void NuPDS::propagate( std::stack<u32> &stack_ ) {
    while ( !stack_.empty() ) {
        auto v = stack_.top();
        stack_.pop();
        if ( is_observed( v ) && unobserved_degree_[v] == 1 && can_propagate( v ) ) {
            for ( auto w : graph_.get_neighbors( v ) ) {
                if ( !is_observed( w ) ) {
                    observe_one( w, v, stack_ );
                    break;
                }
            }
        }
    }
}

bool NuPDS::is_observed( u32 v ) { return dependencies_.has_vertex( v ); }

bool NuPDS::all_observed() { return dependencies_.vertex_nums() >= graph_.vertex_nums(); }

bool NuPDS::is_in_solution( u32 v ) { return solution_.find( v ) != solution_.end(); }

bool NuPDS::not_exculded( u32 v ) { return excluded_.find( v ) == excluded_.end(); }

bool NuPDS::is_tabu( u32 v ) { return tabu_[v] != 0; }

void NuPDS::add_into_solution( std::pair<u32, double> vertex_pair, bool fake ) {
    auto v = vertex_pair.first;
    u32 score = dependencies_.vertex_nums();

    if ( dependencies_.has_vertex( v ) ) {
        auto neighbors = dependencies_.get_in_neighbors( v );
        for ( auto &u : neighbors ) {
            dependencies_.remove_edge( u, v );
        }
    }
    std::stack<u32> stack_;
    observe_one( v, v, stack_ );
    for ( auto w : graph_.get_neighbors( v ) ) {
        cc_[w] = true;
        observe_one( w, v, stack_ );
    }
    propagate( stack_ );

    if ( !fake ) {
        solution_.insert( v );
        tabu_[v] = tabu_size_;
        if ( !vertices_state_[v].pre_selected ) {
            if ( vertex_pair.second != -1 ) {
                remove_score_[v] = vertex_pair.second;
            } else {
                remove_score_[v] = ( dependencies_.vertex_nums() - score ) * ( 1 + random_alpha() );
            }
        }
    }
}

void NuPDS::remove_from_solution( u32 v ) {
    std::stack<u32> propagating;
    std::stack<u32> stack_;
    set<u32> enqueued;

    tabu_[v] = tabu_size_;

    solution_.erase( v );
    cc_[v] = false;
    stack_.push( v );
    enqueued.insert( v );

    u32 score = 0;

    while ( !stack_.empty() ) {
        auto v = stack_.top();
        stack_.pop();
        std::optional<u32> observer;
        for ( auto w : graph_.get_neighbors( v ) ) {
            cc_[w] = true;
            unobserved_degree_[w] += 1;

            if ( !is_in_solution( w ) ) {
                // w is observed from v
                if ( dependencies_.has_edge( v, w ) ) {
                    if ( enqueued.find( w ) == enqueued.end() ) {
                        stack_.push( w );
                        enqueued.insert( w );
                    }
                } else if ( is_observed( w ) ) {
                    if ( dependencies_.has_neighbors( w ) ) {
                        for ( auto x : dependencies_.get_out_neighbors( w ) ) {
                            if ( enqueued.find( x ) == enqueued.end() ) {
                                stack_.push( x );
                                enqueued.insert( x );
                            }
                        }
                    }
                    propagating.push( w );
                }
            } else {
                observer = { w };
            }
        }
        // mark unobserved
        dependencies_.remove_vertex( v );
        non_observed_.insert( v );
        if ( observer.has_value() ) {
            observe_one( v, observer.value(), propagating );
        }
    }

    score = dependencies_.vertex_nums();

    propagate( propagating );

    score = score - dependencies_.vertex_nums();

    if ( !is_observed( v ) ) {
        score += unobserved_degree_[v] + graph_.degree( v );
    }
}

bool NuPDS::is_effected( u32 vertex, const set<u32> &changed ) {
    auto neighbors = graph_.get_neighbors( vertex );
    for ( auto &v : changed ) {
        if ( neighbors.find( v ) != neighbors.end() ) {
            return true;
        }
    }
    return false;
}

set<u32> NuPDS::get_neighbor( const set<u32> &vertices, bool closed ) {
    set<u32> neighbors;
    for ( auto &v : vertices ) {
        if ( closed ) {
            neighbors.insert( v );
        }
        for ( auto &w : graph_.get_neighbors( v ) ) {
            neighbors.insert( w );
        }
    }
    return neighbors;
}

void NuPDS::update_candidate_after_add( u32 vertex ) {
    auto changed = dependencies_.get_out_neighbors( vertex );
    auto nei_changed = get_neighbor( changed );
    set<u32> effected;
    for ( auto &v : available_candidates_ ) {
        auto state = vertices_state_[v];
        if ( ( !state.used ) && ( !state.update ) ) {
            if ( is_effected( v, nei_changed ) ) {
                // effected.insert( v );
                vertices_state_[v].update = true;
            }
        }
    }
    vertices_state_[vertex].used = true;
    available_candidates_.erase( vertex );
}

void NuPDS::update_candidate_after_remove( u32 vertex ) {
    auto nei_changed = get_neighbor( non_observed_ );
    // set<u32> effected;
    for ( auto &v : available_candidates_ ) {
        auto state = vertices_state_[v];
        if ( ( !state.used ) && ( !state.update ) ) {
            if ( is_effected( v, nei_changed ) ) {
                // effected.insert( v );
                vertices_state_[v].update = true;
            }
        }
    }
    vertices_state_[vertex].used = false;
    available_candidates_.insert( vertex );
}

std::pair<u32, double> NuPDS::select_add_vertex() {
    double maxn = -1;
    u32 ret = *available_candidates_.begin();

    for ( auto &v : available_candidates_ ) {
        if ( vertices_state_[v].update && !is_tabu( v ) && cc_[v] ) {
            auto prev = dependencies_;
            auto prev_unobserved = unobserved_degree_;
            add_into_solution( { v, -1 }, true );
            auto score = dependencies_.vertex_nums() - prev.vertex_nums();
            dependencies_ = std::move( prev );
            unobserved_degree_ = std::move( prev_unobserved );
            vertices_state_[v].update = false;
            double vscore = score * ( 1 + random_alpha() );
            if ( vscore > maxn ) {
                maxn = vscore;
                ret = v;
            }
        }
    }
    if ( maxn == -1 ) {
        return { *available_candidates_.begin(), -1 };
    }
    return { ret, maxn };
}

u32 NuPDS::select_remove_vertex() {
    double minn = INT32_MAX;
    u32 ret = remove_score_.begin()->first;
    for ( auto &[v, score] : remove_score_ ) {
        if ( is_tabu( v ) && cc_[v] && minn < score ) {
            minn = score;
            ret = v;
        }
    }
    return ret;
}

void NuPDS::greedy() {
    while ( !all_observed() ) {
        auto v = select_add_vertex();
        add_into_solution( v, false );
        update_candidate_after_add( v.first );
    }
}

void NuPDS::redundant_removal() {
    // step1: remove all unnecessary vertices
    auto candidates = solution_;
    for ( auto &v : candidates ) {
        auto copy_graph = dependencies_;
        auto unobserved = unobserved_degree_;
        auto non_observed = non_observed_;
        remove_from_solution( v );
        if ( !all_observed() ) {
            solution_.insert( v );
            dependencies_ = std::move( copy_graph );
            unobserved_degree_ = std::move( unobserved );
            non_observed_ = std::move( non_observed );
        } else {
            best_solution_.erase( v );
        }
    }
}

void NuPDS::solve() {
    if ( !all_observed() ) {
        greedy();
    }

    if ( solution_.size() == 1 ) {
        best_solution_ = solution_;
        return;
    }

    for ( auto &[k, v] : unobserved_degree_ ) {
        v = 0;
    }

    tabu_.clear();

    best_solution_ = solution_;

    // step2: local search
    while ( timestamp_ < cutoff_ ) {
        timestamp_++;
        if ( all_observed() && solution_.size() > 1 ) {
            best_solution_ = solution_;
            // should random select a vertex to remove
            redundant_removal();
            continue;
        }
        auto v = select_remove_vertex();
        remove_from_solution( v );
        update_candidate_after_remove( v );
        auto u = select_add_vertex();
        add_into_solution( u, false );
        update_candidate_after_add( u.first );
    }
}

const set<u32> &NuPDS::get_solution() const { return solution_; }

const set<u32> &NuPDS::get_best_solution() const { return best_solution_; }

const set<u32> &NuPDS::get_observed_vertex() const { return dependencies_.vertices(); }

void NuPDS::update_pre_selected() {
    dependencies_ = Graph();
    best_solution_.clear();
    solution_.clear();
    for ( auto &v : graph_.vertices() ) {
        unobserved_degree_[v] = graph_.degree( v );
        age[v] = 0;
        cc_[v] = true;
        tabu_[v] = 0;
    }
}