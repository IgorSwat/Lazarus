#pragma once

#include "chessboard.h"


namespace chess::rules {

    // ---------------------
    // Chess rules - general
    // ---------------------

    // A string representation (FEN) of standard chess starting position 
    const std::string STARTING_POSITION = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // Extracts king starting position
    constexpr inline Square king_starting_position(Color side)
    {
        return side == WHITE ? SQ_E1 : SQ_E8;
    }


    // ----------------------
	// Chess rules - castling
	// ----------------------

    // Calculate starting castle square for king
    // - An equivalent for king starting square
    constexpr inline Square castle_from(Color side)
    {
        return king_starting_position(side);
    }

    // Calculate target castle square for king
    constexpr inline Square castle_to(Color side, Castle castle)
    {
       return side == WHITE ? (castle == KINGSIDE_CASTLE ? SQ_G1 : SQ_C1) :
                              (castle == QUEENSIDE_CASTLE ? SQ_G8 : SQ_C8);
    }

    // Calculate kings path during castling
    // - NOTE: does note include kings starting square
    constexpr inline Bitboard castle_path(Color side, Castle castle)
    {
        return side == WHITE ? (castle == KINGSIDE_CASTLE ? 0x0000000000000060 : 0x000000000000000e) :
                               (castle == KINGSIDE_CASTLE ? 0x6000000000000000 : 0x0e00000000000000);
    }

    // Lookup table - castling right loss after moving from square
    // - Defines which castle rights are being lost after entering or moving away from given square
    // - Since castling right loss is irreversible, only the first time we move away king or rook really matters
    // - From previously mentioned reason, we can always connect moving from square like A1 with moving white's rook
    // - Moving the king (moves related to E1 or E8 squares) results in loss of all castling rights for given side
    inline constexpr CastlingRights CastleLoss[SQUARE_RANGE] = {
        WHITE_OOO, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, WHITE_BOTH, NO_RIGHTS, NO_RIGHTS, WHITE_OO,
        NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS,
        NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS,
        NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS,
        NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS,
        NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS,
        NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS,
        BLACK_OOO, NO_RIGHTS, NO_RIGHTS, NO_RIGHTS, BLACK_BOTH, NO_RIGHTS, NO_RIGHTS, BLACK_OO
    };
}