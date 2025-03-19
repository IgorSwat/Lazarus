#include "src/engine/engine.h"
#include "src/engine/pieces.h"
#include "src/engine/zobrist.h"
#include "src/engine/ttable.h"
#include "test/test.h"
#include <iostream>
#include <memory>


int main()
{
    // --------------------
    // Initialization stage
    // --------------------

    Chessboard::initialize_board_space();
    Pieces::initialize_attack_tables();
    Zobrist::initialize_zobrist_numbers();
    // ....


    // -------------
    // Testing stage
    // -------------

    // bool test_results = Testing::run_tests();

    // if (!test_results)
    //     std::abort();

    // Speed test
    // Testing::search_speed_test(5);

    // Quality test
    Testing::search_accuracy_test(7);


    // ---------------
    // Main processing
    // ---------------

    std::unique_ptr<Engine> engine = std::make_unique<Engine>(Engine::Mode::STANDARD);
    // engine->grid_search();

    // engine->set_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    // engine->evaluate(5);

    // std::cout << std::endl;
    // engine->show_ordering();

    // std::string fen;
    // while (true) {
    //     std::getline(std::cin, fen);
    //     if (fen.empty()) break;

    //     int depth;
    //     std::cin >> depth;
    //     std::cin.get();

    //     engine->set_position(fen);
    //     engine->evaluate(Search::Depth(5));
    // }

    return 0;
}