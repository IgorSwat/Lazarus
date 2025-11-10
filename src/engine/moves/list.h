#pragma once

#include <cassert>
#include <moves/move.h>
#include <utilities/stable_vector.h>

namespace chess::moves {

    // ------------------------------
    // Move List - helper definitions
    // ------------------------------

    // The maximum number of chess moves for one side in a legal chess position
    constexpr int MAX_LEGAL_MOVES = 218;

    // Move list size limit - the maximum number of legal moves rounded to the nearest power of 2
    constexpr int MOVELIST_SIZE_LIMIT = 256;


    // ---------------------------
    // Move List - main definition
    // ---------------------------

    // We utilize the implementation of StableVector class
    // - By templating the definition we allow to create lists of different types of moves
    // - We allow to create lists of lower size, which uses less amount of memory
    template <typename MoveT = Move, unsigned max_size = MOVELIST_SIZE_LIMIT>
    using List = utilities::StableVector<
        memory::Storage::STATIC, 
        MoveT, 
        max_size
    >;

}