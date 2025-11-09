#pragma once

#include <cinttypes>


// -----------------------------------------------
// Type definitions - general move representations
// -----------------------------------------------

namespace chess {

    // We can distinguish 4 main types of moves: standard moves, and 3 special categories (enpassant, castle and promotion)
    // - The categories are completely separate
    // - Standard moves and promotions can be further divided into captures and non-captures, while enpassant is always a capture
    enum MoveType : uint32_t {
        NORMAL = 1,
        PROMOTION,
        ENPASSANT,
        CASTLE,

        // A special category used only in the context of null move instance
        NULL_MOVE = 0
    };

}


// -------------------------------------------------------
// Type definitions - engine-specific move representations
// -------------------------------------------------------

namespace engine {

    // Move enhancement definition
    // - Enhancement is some additional information about move, embedded together with Move object itself
    // - It could be some evaluation performed on a move (like SEE), or just a custom number which serves as a sorting index
    enum class MoveEnhancement : uint8_t {
        PURE_SEE = 1,
        PURE_SEARCH_SCORE,
        CUSTOM_SORTING,

        NONE = 0
    };

}