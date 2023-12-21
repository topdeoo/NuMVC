#include "pds.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <optional>
#include <stack>
#include <utility>
#include <vector>

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
    }

    u32 k;
    fin >> k;
    for ( u32 i = 0; i < k; i++ ) {
        u32 v;
        fin >> v;
        pre_selected_.insert( v );
    }

    u32 l;
    fin >> l;
    for ( u32 i = 0; i < l; i++ ) {
        u32 v;
        fin >> v;
        excluded_.insert( v );
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
        base_score_[v] = graph_.degree( v );
        age[v] = 0;
        cc_[v] = true;
        tabu_[v] = 0;
    }

    // TODO
    cutoff_ = 5000;

    init_heap( n, graph_ );
}

void NuPDS::pre_process() {
    add_heap_index = 0;
    timestamp_ = 0;

    for ( auto &v : pre_selected_ ) {
        add_into_solution( v );
        tabu_[v] = 0;
    }
    remove_heap_index = 0;
}

void NuPDS::observe( u32 vertex, u32 origin ) {
    std::stack<u32> stack_;
    observe_one( vertex, origin, stack_ );
    propagate( stack_ );
}

void NuPDS::observe_one( u32 vertex, u32 origin, std::stack<u32> &stack_ ) {
    if ( !is_observed( vertex ) ) {
        dependencies_.add_vertex( vertex );
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

u32 NuPDS::Ob( u32 v ) {
    u32 score = 0;
    if ( dependencies_.has_vertex( v ) ) {
        auto neighbors = dependencies_.get_in_neighbors( v );
        for ( auto &u : neighbors ) {
            dependencies_.remove_edge( u, v );
        }
    }

    std::stack<u32> stack_;
    observe_one( v, v, stack_ );
    for ( auto w : graph_.get_neighbors( v ) ) {
        observe_one( w, v, stack_ );
    }
    propagate( stack_ );
    score = dependencies_.vertex_nums();

    std::stack<u32> propagating;
    while ( !stack_.empty() ) stack_.pop();
    set<u32> enqueued;
    stack_.push( v );
    enqueued.insert( v );

    while ( !stack_.empty() ) {
        auto u = stack_.top();
        stack_.pop();
        std::optional<u32> observer;
        for ( auto w : graph_.get_neighbors( u ) ) {
            unobserved_degree_[w] += 1;

            if ( !is_in_solution( w ) ) {
                // w is observed from u
                if ( dependencies_.has_edge( u, w ) ) {
                    if ( enqueued.find( w ) == enqueued.end() ) {
                        stack_.push( w );
                        enqueued.insert( w );
                    }
                } else if ( is_observed( w ) ) {
                    if ( dependencies_.has_neighbors( w ) ) {
                        for ( auto x : dependencies_.get_neighbors( w ) ) {
                            if ( enqueued.find( w ) == enqueued.end() ) {
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
        dependencies_.remove_vertex( u );
        if ( observer.has_value() ) {
            observe_one( v, observer.value(), propagating );
        }
    }
    propagate( propagating );
    return score - dependencies_.vertex_nums();
}

void NuPDS::add_into_solution( u32 v ) {
    solution_.insert( v );

    tabu_[v] = tabu_size_;

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
    score = dependencies_.vertex_nums() - score;
    insert(
        {
            v,
            score,
        },
        HEAPTYPE::REMOVE );
    remove_score_[v] = score;
}

void NuPDS::remove_from_solution( u32 v ) {
    std::stack<u32> propagating;
    std::stack<u32> stack_;
    set<u32> enqueued;

    tabu_[v] = tabu_size_;

    solution_.erase( v );
    candidate_solution_set_.erase( v );
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

    auto alpha = random_alpha();

    insert( { v, alpha * score + ( 1 - alpha ) * base_score_[v] }, HEAPTYPE::ADD );
}

u32 NuPDS::select_add_vertex() {
    std::vector<std::pair<u32, u32>> candidates;
    u32 ret = UINT32_MAX;
    while ( add_heap_index > 0 ) {
        if ( add_heap_index == 0 ) break;
        auto v = top( HEAPTYPE::ADD );
        pop( HEAPTYPE::ADD );
        if ( cc_[v.first] && !is_tabu( v.first ) && not_exculded( v.first ) &&
             !is_in_solution( v.first ) ) {
            ret = v.first;
            break;
        } else if ( !is_in_solution( v.first ) && not_exculded( v.first ) ) {
            tabu_[v.first]--;
            candidates.push_back( v );
        }
    }
    for ( auto &v : candidates ) {
        insert( v, HEAPTYPE::ADD );
    }
    return ret;
}

u32 NuPDS::select_remove_vertex() {
    std::vector<std::pair<u32, u32>> candidates;
    u32 ret = INT32_MAX;
    while ( remove_heap_index > 0 ) {
        if ( remove_heap_index == 0 ) break;
        auto v = top( HEAPTYPE::REMOVE );
        pop( HEAPTYPE::REMOVE );
        if ( cc_[v.first] && !is_tabu( v.first ) && is_in_solution( v.first ) &&
             pre_selected_.find( v.first ) == pre_selected_.end() ) {
            ret = v.first;
            break;
        } else if ( is_in_solution( v.first ) &&
                    pre_selected_.find( v.first ) == pre_selected_.end() ) {
            tabu_[v.first]--;
            candidates.push_back( v );
        }
    }
    for ( auto &v : candidates ) {
        insert( v, HEAPTYPE::REMOVE );
    }
    return ret;
}

void NuPDS::tabu_forget() {}

void NuPDS::update_score() {}

void NuPDS::greedy() {
    for ( auto &v : graph_.vertices() ) {
        if ( !is_in_solution( v ) && not_exculded( v ) ) {
            insert( { v, base_score_[v] }, HEAPTYPE::ADD );
        }
    }

    while ( !all_observed() ) {
        auto v = select_add_vertex();
        add_into_solution( v );
        tabu_[v] = 0;
        age[v] = timestamp_;
    }
}

void NuPDS::redundant_removal() {
    // step1: remove all unnecessary vertices
    auto candidates = candidate_solution_set_;
    for ( auto &v : candidates ) {
        auto copy_graph = dependencies_;
        auto unobserved = unobserved_degree_;
        remove_from_solution( v );
        if ( !all_observed() ) {
            solution_.insert( v );
            dependencies_ = std::move( copy_graph );
            unobserved_degree_ = std::move( unobserved );
        } else {
            best_solution_.erase( v );
            candidate_solution_set_.erase( v );
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

    update_pre_selected();
    pre_process();

    // TODO: can optimize here by merge two procedure

    for ( auto &v : candidate_solution_set_ ) {
        add_into_solution( v );
        tabu_[v] = 0;
        age[v] = timestamp_;
    }

    best_solution_ = solution_;

    add_heap_index = 0;
    for ( auto &v : graph_.vertices() ) {
        if ( !is_in_solution( v ) && not_exculded( v ) ) {
            insert( { v, base_score_[v] }, HEAPTYPE::ADD );
        }
    }

    remove_heap_index = 0;
    for ( auto &v : candidate_solution_set_ ) {
        insert( { v, remove_score_[v] }, HEAPTYPE::REMOVE );
    }

    // step2: local search
    while ( timestamp_ < cutoff_ && remove_heap_index > 0 && add_heap_index > 0 &&
            candidate_solution_set_.size() > 1 ) {
        timestamp_++;
        if ( all_observed() && solution_.size() > 1 ) {
            std::cout << "1" << std::endl;
            best_solution_ = solution_;
            // should random select a vertex to remove
            redundant_removal();

            continue;
        }

        auto v_remove = select_remove_vertex();
        auto v_add = select_add_vertex();
        if ( v_remove == INT32_MAX || v_add == INT32_MAX ) {
            tabu_forget();
            continue;
        }

        remove_from_solution( v_remove );
        age[v_remove] = timestamp_;
        add_into_solution( v_add );
        age[v_add] = timestamp_;
        candidate_solution_set_.insert( v_add );

        update_score();
    }
}

const set<u32> &NuPDS::get_solution() const { return solution_; }

const set<u32> &NuPDS::get_best_solution() const { return best_solution_; }

const set<u32> &NuPDS::get_observed_vertex() const { return dependencies_.vertices(); }

void NuPDS::update_pre_selected() {
    candidate_solution_set_.clear();
    for ( auto &v : solution_ ) {
        if ( pre_selected_.find( v ) == pre_selected_.end() ) {
            candidate_solution_set_.insert( v );
        }
    }
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