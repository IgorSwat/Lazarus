#include "test/test.h"
#include "src/engine/board/board.h"
#include <iostream>

int main()
{
    // Step 1 - initialization
    bitboards::magics::initialize_magics();
    chess::board::initialize_board_geometry();
    chess::pieces::initialize_attack_tables();
    engine::zobrist::initialize_zobrist_numbers();

    // Step 2 - run tests
    testing::run_tests();

    return 0;
}