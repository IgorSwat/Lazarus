#pragma once

#include "chessboard.h"
#include <bitboards/magics.h>


namespace chess::pieces {

    // ----------------------
	// Pieces - initilization
	// ----------------------

    void initialize_attack_tables();


    // --------------------
	// Pieces - ray attacks
	// --------------------

    // Ray attacks are parts of bishop, rook and queen attack maps
    // - We can calculate ray attacks on empty board using fill algorithm from bitboards library
    // - To calculate ray attack with given occupancy, we can follow the path calculation method from boardspace.cpp

    // Ray attacks (in one direction) with respect to given board occupancy
    template <Direction dir>
    Bitboard ray_attacks(Square sq, Bitboard occ = 0)
    {
        Bitboard att = bitboards::fill<dir>(as_bitboard(sq)) ^ sq;   // Do not count attacking the starting square
        Bitboard blockers = att & occ;

        // Depending on direction, the nearest blocker can stand either on LSB or MSB of blockers bitboard
        constexpr auto nearest_blocker = dir > 0 ? bitboards::lsb : bitboards::msb;

        return !blockers ? att : att & bitboards::fill<~dir>(as_bitboard(nearest_blocker(blockers)));
    }

    // Bidirectional attacks - file attacks (NORTH & SOUTH)
    inline Bitboard file_attacks(Square sq, Bitboard occ = 0) { return ray_attacks<NORTH>(sq, occ) | ray_attacks<SOUTH>(sq, occ); }

    // Bidirectional attacks - rank attacks (EAST & WEST)
    inline Bitboard rank_attacks(Square sq, Bitboard occ = 0) { return ray_attacks<EAST>(sq, occ) | ray_attacks<WEST>(sq, occ); }

    // Bidirectional attacks - diagonal attacks (NORTH_EAST & SOUTH_WEST)
    inline Bitboard diagonal_attacks(Square sq, Bitboard occ = 0) { return ray_attacks<NORTH_EAST>(sq, occ) | ray_attacks<SOUTH_WEST>(sq, occ); }

    // Bidirectional attacks - antidiagonal attacks (NORTH_WEST & SOUTH_EAST)
    inline Bitboard antidiagonal_attacks(Square sq, Bitboard occ = 0) { return ray_attacks<NORTH_WEST>(sq, occ) | ray_attacks<SOUTH_EAST>(sq, occ); }


    // ----------------------------------------------
	// Pieces - single piece attacks - by calculation
	// ----------------------------------------------

    // !!!
    // Since using ray attack calculations to calculate sliding piece attacks is inefficient,
    // those functions should not be used unless lookup versions are not initialized or not available

    inline Bitboard __rook_attacks(Square sq, Bitboard occ)
    {
        return rank_attacks(sq, occ) | file_attacks(sq, occ);
    }

    inline Bitboard __bishop_attacks(Square sq, Bitboard occ)
    {
        return diagonal_attacks(sq, occ) | antidiagonal_attacks(sq, occ);
    }

    inline Bitboard __queen_attacks(Square sq, Bitboard occ)
    {
        return __rook_attacks(sq, occ) | __bishop_attacks(sq, occ);
    }


    // -------------------------------------------------
	// Single piece attacks - by lookup - direct attacks
	// -------------------------------------------------

    // Lookup tables
    // - Pseudo attack is attack that ignores occupancy factor (which would be legal on empty board)
    // - NOTE: PieceAttacks contains only pseudo legal moves, which are equivalent of legal moves for knight and king,
    //         but discard occupancy factor for sliding pieces (bishops, rooks and queens)
    extern Bitboard PawnAttacks[COLOR_RANGE][SQUARE_RANGE];
	extern Bitboard PseudoAttacks[PIECE_TYPE_RANGE][SQUARE_RANGE];

    // A simple wrapper to use instead of lookup table
    inline Bitboard pseudo_attacks(PieceType ptype, Square sq)
    {
        return PseudoAttacks[ptype][sq];
    }

    inline Bitboard pawn_attacks(Color side, Square sq)
    {
        return PawnAttacks[side][sq];
    }

     // General static version - for knight & king attacks
    template <PieceType ptype>
    inline Bitboard piece_attacks(Square sq, Bitboard occ = 0)
    {
        return PseudoAttacks[ptype][sq];
    }

    // Specialized static version - sliding piece attacks - bishop magics lookup
    template <>
	inline Bitboard piece_attacks<BISHOP>(Square sq, Bitboard occ)
	{
		bitboards::Magic& m = bitboards::magics::BishopMagics[sq];
		return m.attacks[m.index(occ)];
	}

    // Specialized static version - sliding piece attacks - rook magics lookup
    template <>
	inline Bitboard piece_attacks<ROOK>(Square sq, Bitboard occ)
	{
		bitboards::Magic& m = bitboards::magics::RookMagics[sq];
		return m.attacks[m.index(occ)];
	}

    // Specialized static version - sliding piece attacks - queen attacks
    template <>
	inline Bitboard piece_attacks<QUEEN>(Square sq, Bitboard occ)
	{
		return piece_attacks<BISHOP>(sq, occ) | piece_attacks<ROOK>(sq, occ);
	}


    // ------------------------------------------------
	// Single piece attacks - by lookup - x-ray attacks
	// ------------------------------------------------

    // X-ray attacks is a pseudo attack blocked by a friendly piece with the same properties as attacking piece
    // For example, rook attacks are extended to x-ray attacks if it stands behind another rook or queen
    // - X-ray attacks play a key role in SEE (Static Exchange Evaluation) algorithm

    // NOTE: results in undefined behavior for non-sliding pieces (pawn, knight, king)
    template <PieceType ptype>
    inline Bitboard xray_attacks(Square sq, Bitboard occ, Bitboard blockers)
    {
        Bitboard attacks = piece_attacks<ptype>(sq, occ);
		blockers &= attacks;

		return attacks ^ piece_attacks<ptype>(sq, occ ^ blockers);
    }

    template <>
	inline Bitboard xray_attacks<QUEEN>(Square sq, Bitboard occ, Bitboard blockers)
	{
		return xray_attacks<BISHOP>(sq, occ, blockers) | xray_attacks<ROOK>(sq, occ, blockers);
	}


    // ----------------------
	// Multiple piece attacks
	// ----------------------

    // The following algorithms perform aggregative attack calculation for non sliding pieces (pawns, knights and king)
    // - Aggregative calculation means it can calculate a whole map of attacks of all pieces of given type in one go
    // - Utilizes divide & conquer approach on bitboards

    // Each square can only be attacked by one pawn...
    template <Color side>
    constexpr inline Bitboard pawn_attacks(Bitboard pawns)
    {
        Bitboard ar_squares = (pawns & board::NOT_FILE_A) >> 1 | (pawns & board::NOT_FILE_H) << 1;

        return bitboards::shift<side == WHITE ? NORTH : SOUTH>(ar_squares);
    }

    // or two pawns
    template <Color side>
    constexpr inline Bitboard double_pawn_attacks(Bitboard pawns)
    {
        return side == WHITE ? bitboards::shift<NORTH_WEST>(pawns) & bitboards::shift<NORTH_EAST>(pawns) :
                               bitboards::shift<SOUTH_WEST>(pawns) & bitboards::shift<SOUTH_EAST>(pawns);
    }

    constexpr inline Bitboard knight_attacks(Bitboard knights)
    {
        Bitboard l1 = (knights & board::NOT_FILE_A) >> 1;
		Bitboard l2 = (knights & board::NOT_FILE_AB) >> 2;
		Bitboard r1 = (knights & board::NOT_FILE_H) << 1;
		Bitboard r2 = (knights & board::NOT_FILE_GH) << 2;

		Bitboard h1 = l1 | r1;
		Bitboard h2 = l2 | r2;

		return (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);
    }

    constexpr inline Bitboard king_attacks(Bitboard kings)
    {
        Bitboard att = (kings & board::NOT_FILE_H) << 1 | (kings & board::NOT_FILE_A) >> 1;
        
        kings |= att;
        att |= (kings << 8) | (kings >> 8);

        return att;
    }

}