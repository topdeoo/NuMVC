#include <cassert>
#include <fstream>
#include <iostream>

#include "basic.hpp"
#include "nupds.hpp"

int main( int argc, const char* argv[] ) {
    if ( argc <= 1 ) {
        exit( 1 );
    }
    std::ifstream fin( argv[1] );

    std::string t;
    fin >> t;

    NuPDS solver;

    solver.init( fin );

    auto newly_observed = solver.setDominating( 1285 );

    if ( newly_observed.size() != 37 ) {
        std::cout << "Failed test" << std::endl;
        std::cout << "Newly Observed: " << newly_observed.size() << std::endl;
    } else {
        std::cout << "Passed test" << std::endl;
    }
    return 0;
}
