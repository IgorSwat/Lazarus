#pragma once

#include "bitboards.h"


namespace bitboards {

    // -------------------
    // Magics - definition
    // -------------------

    struct Magic
	{
		Bitboard* attacks;      // Pointer to appropriate part of a lookup table

		Bitboard mask;          // Mask = {attacks from sq} - {board edges}
		uint64_t magic;         // A magic number found by randomized search that performs best in given case
		uint32_t shift;         // Shift = 64 - popcount(mask)

        // Lookup table index calculation
		uint32_t index(Bitboard bb) const
		{
			return uint32_t(((bb & mask) * magic) >> shift);
		}
	};

    namespace magics {

        // ----------------------
        // Magics - lookup tables
        // ----------------------

        // NOTE: We can just share magics for each square as a proxy for heavy lookup tables
        extern Magic RookMagics[SQUARE_RANGE];
        extern Magic BishopMagics[SQUARE_RANGE];


        // ---------------------------------------
        // Magics - lookup tables - initialization
        // ---------------------------------------

        void initialize_magics();

    }

}