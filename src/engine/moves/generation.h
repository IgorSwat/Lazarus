#pragma once

#include <board/board.h>
#include <moves/list.h>
#include <moves/move.h>

namespace engine::movegen {

    // --------------------------------------------
    // Move generation - generation mode definition
    // --------------------------------------------

    // Decides on which groups of moves should be generated
    enum Mode : uint32_t {
        NONE = 0,

        QUIET = 1,          // Not a capture and not a direct check
        QUIET_CHECK,        // A direct check
        CAPTURE,            // A capture
        CHECK_EVASION,      // A move that evades a check (either by moving the king, blocking the check or eliminating the checking piece)

        PSEUDO_LEGAL,       // All the moves that would pass is_pseudolegal() check
        LEGAL,              // All the moves that are legal in given position

        MODE_RANGE = 7
    };


    // ---------------------------------------
    // Move generation - collective generation
    // ---------------------------------------

    // Generate moves according to given position and add them to movelist
    template <Mode mode, typename MoveT = chess::Move>
	void generate_moves(const Board& board, chess::moves::List<MoveT>& movelist);

}