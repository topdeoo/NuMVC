#include "heap.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <random>
#include <utility>
#include <vector>

#include "basic.hpp"
#include "graph.hpp"

std::vector<std::pair<u32, u32>> add_score;
u32 add_heap_index;
std::vector<std::pair<u32, u32>> removing_score;
u32 remove_heap_index;
map<u32, u32> age;

static std::random_device rd{ "hw" };

static bool random_x() {
    static auto gen =
        std::bind( std::uniform_int_distribution<>( 0, 1 ), std::default_random_engine( rd() ) );
    return gen();
}

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

u32 parient( u32 index ) { return index / 2; }
u32 left_child( u32 index ) { return 2 * index; }
u32 right_child( u32 index ) { return 2 * index + 1; }

static void up( u32 index, HEAPTYPE which ) {
    if ( which == HEAPTYPE::ADD ) {
        while ( parient( index ) > 0 &&
                ( add_score[parient( index )].second <= add_score[index].second ) ) {
            auto u = add_score[parient( index )], v = add_score[index];
            if ( u.second > v.second ) {
                goto add_swap;
            } else if ( u.second == v.second && age[u.first] < age[v.first] ) {
                goto add_swap;
            } else if ( u.second == v.second && age[u.first] == age[v.first] && random_x() ) {
                goto add_swap;
            }
        add_swap:
            std::swap( add_score[parient( index )], add_score[index] );
            index = parient( index );
        }
    } else {
        while ( parient( index ) > 0 &&
                removing_score[parient( index )].second >= removing_score[index].second ) {
            auto u = removing_score[parient( index )], v = removing_score[index];
            if ( u.second > v.second ) {
                goto remove_swap;
            } else if ( u.second == v.second && age[u.first] < age[v.first] ) {
                goto remove_swap;
            } else if ( u.second == v.second && age[u.first] == age[v.first] && random_x() ) {
                goto remove_swap;
            }
        remove_swap:
            std::swap( removing_score[parient( index )], removing_score[index] );
            index = parient( index );
        }
    }
}

static void down( u32 index, HEAPTYPE which ) {
    u32 l = left_child( index ), r = right_child( index );
    if ( which == HEAPTYPE::ADD ) {
        u32 largest = 0;
        if ( l <= add_heap_index && add_score[l].second > add_score[index].second ) {
            largest = l;
        } else if ( l <= add_heap_index && add_score[l].second == add_score[index].second ) {
            if ( age[add_score[l].first] < age[add_score[index].first] ) {
                largest = l;
            } else if ( age[add_score[l].first] == age[add_score[index].first] && random_x() ) {
                largest = l;
            } else {
                largest = index;
            }
        } else {
            largest = index;
        }

        if ( r <= add_heap_index && add_score[r].second > add_score[largest].second ) {
            largest = r;
        } else if ( r <= add_heap_index && add_score[r].second == add_score[largest].second ) {
            if ( age[add_score[r].first] < age[add_score[largest].first] ) {
                largest = r;
            } else if ( age[add_score[r].first] == age[add_score[largest].first] && random_x() ) {
                largest = r;
            } else {
                largest = index;
            }
        } else {
            largest = index;
        }

        if ( largest != index ) {
            std::swap( add_score[index], add_score[largest] );
            down( largest, which );
        }

    } else {
        u32 smallest = 0;
        if ( l <= remove_heap_index && removing_score[l].second < removing_score[index].second ) {
            smallest = l;
        } else if ( l <= remove_heap_index &&
                    removing_score[l].second == removing_score[index].second ) {
            if ( age[removing_score[l].first] < age[removing_score[index].first] ) {
                smallest = l;
            } else if ( age[removing_score[l].first] == age[removing_score[index].first] &&
                        random_x() ) {
                smallest = l;
            } else {
                smallest = index;
            }
        } else {
            smallest = index;
        }

        if ( r <= remove_heap_index && removing_score[r].second < removing_score[smallest].second ) {
            smallest = r;
        } else if ( r <= remove_heap_index &&
                    removing_score[r].second == removing_score[smallest].second ) {
            if ( age[removing_score[r].first] < age[removing_score[smallest].first] ) {
                smallest = r;
            } else if ( age[removing_score[r].first] == age[removing_score[smallest].first] &&
                        random_x() ) {
                smallest = r;
            }
        }

        if ( smallest != index ) {
            std::swap( removing_score[index], removing_score[smallest] );
            down( smallest, which );
        }
    }
}

void insert( const std::pair<u32, u32> &entry, HEAPTYPE which ) {
    if ( which == HEAPTYPE::ADD ) {
        add_score[++add_heap_index] = entry;
        up( add_heap_index, which );

    } else {
        removing_score[++remove_heap_index] = entry;
        up( remove_heap_index, HEAPTYPE::REMOVE );
    }
}

std::pair<u32, u32> &top( HEAPTYPE which ) {
    if ( which == HEAPTYPE::ADD ) {
        return add_score[1];
    } else {
        return removing_score[1];
    }
}

void pop( HEAPTYPE which ) {
    if ( which == HEAPTYPE::ADD ) {
        std::swap( add_score[1], add_score[add_heap_index] );
        add_heap_index--;
    } else {
        std::swap( removing_score[1], removing_score[remove_heap_index] );
        remove_heap_index--;
    }
    down( 1, which );
}

void init_remove_set( u32 n, Graph &graph ) {
    removing_score.resize( n + 1 );
    std::fill( removing_score.begin(), removing_score.end(), std::make_pair( UINT32_MAX, UINT32_MAX ) );
}
