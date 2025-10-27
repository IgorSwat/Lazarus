#pragma once

#include "../chess/types.h"
#include <string>
#include <nmmintrin.h>


namespace bitboards {

	using namespace chess;

    // ----------------------------------
    // Bitboards - local - bit extraction
    // ----------------------------------

    // An operation is local if it aims to extract an information about a single bit of a bitboard

    // Extracts (without removing it) the least significant bit of a bitboard
    inline Square lsb(Bitboard mask)
	{
	#ifdef _MSC_VER
		unsigned long bitID;
		_BitScanForward64(&bitID, mask);
	#else
		unsigned long bitID = __builtin_ctzll(mask);
	#endif
		return Square(bitID);
	}

    // Extracts (without removing it) the most significant bit of a bitboard
    inline Square msb(Bitboard mask)
	{
	#ifdef _MSC_VER
		unsigned long bitID;
		_BitScanReverse64(&bitID, mask);
	#else
		unsigned long bitID = 63 - __builtin_clzll(mask);
	#endif
		return Square(bitID);
	}

    // Extracts (with removal!) the leeast significant bit of a bitboard
    // - Allows to iterate over all bits (squares) easily in something like a while loop
    inline Square pop_lsb(Bitboard& mask)
	{
		Square sq = lsb(mask);
		mask &= (mask - 1);

		return sq;
	}


    // -------------------------------------
    // Bitboards - global - population count
    // -------------------------------------

    // An operation is global if it aims to extract an information about bitboard structure or population

    // Checks whether bitboard is empty or not
    constexpr inline bool empty(Bitboard bb)
    {
        return !bb;
    }

    // Checks whether bitboard contains exactly one non-zero bit
    // - It is recommended to use this function instead of more general popcount() if we just want to check for a single non-zero bit form
    constexpr inline bool singly_populated(Bitboard bb)
	{
		return bb && !(bb & (bb - 1));
	}

    // Counts all non-zero bits in a bitboard
    inline unsigned popcount(Bitboard bb)
	{
	#ifdef _MSC_VER
		return uint32_t(__popcnt64(bb));
	#else
		return __builtin_popcountll(bb);
	#endif
	}


    // ---------------------------
    // Bitboards - global - shifts
    // ---------------------------

    // Shifts bitboard in a given direction
    // - Works similarly to adding a direction to a square, but here we apply this direction to all of bitboard non-zero bits
    constexpr inline Bitboard shift(Bitboard bb, Direction dir)
	{
		// We need to use "magic numbers" here, because constants from boardspace.h cannot be included to avoid header include loop
		return dir == NORTH ? bb << 8 : dir == SOUTH ? bb >> 8 :
			   dir == EAST ? (bb & 0x7f7f7f7f7f7f7f7f) << 1 : dir == WEST ? (bb & 0xfefefefefefefefe) >> 1 :
			   dir == NORTH_EAST ? (bb & 0x7f7f7f7f7f7f7f7f) << 9 : dir == NORTH_WEST ? (bb & 0xfefefefefefefefe) << 7 :
			   dir == SOUTH_EAST ? (bb & 0x7f7f7f7f7f7f7f7f) >> 7 : dir == SOUTH_WEST ? (bb & 0xfefefefefefefefe) >> 9 : 0;
	}

	// A more aesthetic alias with direction given as template parameter
	template <Direction dir> constexpr inline Bitboard shift(Bitboard bb) { return shift(bb, dir); }


    // --------------------------
    // Bitboards - global - fills
    // --------------------------

    // General fill algorithm
    template <Direction dir>
	constexpr inline Bitboard fill(Bitboard bb)
	{
		Bitboard shifted = bb;

		// In general, we can describe fill operation as many shifts performed in the same direction
		do {
			bb = shifted;
			shifted |= shift<dir>(shifted);
		} while (shifted != bb);

		return shifted;
	}

    // Specyfic, efficient implementation for NORTH direction
	// - Uses divide and conquer method
	template <>
	constexpr inline Bitboard fill<NORTH>(Bitboard bb)
	{
		bb |= (bb << 32);
		bb |= (bb << 16);
		bb |= (bb << 8);

		return bb;
	}

	// Specyfic, efficient implementation for SOUTH direction
	// - Uses divide and conquer method
	template <>
	constexpr inline Bitboard fill<SOUTH>(Bitboard bb)
	{
		bb |= (bb >> 32);
		bb |= (bb >> 16);
		bb |= (bb >> 8);
		
		return bb;
	}


    // ----------------------------------
    // Bitboards - global - miscellaneous
    // ----------------------------------

    // Converts bitboard to a text representation
    // - Only for debugging purposes
    std::string to_string(Bitboard bb);
    
}