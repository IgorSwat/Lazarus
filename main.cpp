#include "test/test.h"
#include "src/engine/chess/pieces.h"
#include <iostream>

int main()
{
    // Step 1 - initialization
    bitboards::magics::initialize_magics();

    // Step 2 - run tests
    testing::run_tests();

    return 0;
}