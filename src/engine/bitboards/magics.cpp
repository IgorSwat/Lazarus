#include "magics.h"
#include <chess/chessboard.h>
#include <chess/pieces.h>
#include <utilities/random.h>
#include <vector>

using namespace chess;


namespace bitboards::magics {

    // ----------------------
    // Magics - lookup tables
    // ----------------------

    // Main magic tables (public)
	Magic RookMagics[SQUARE_RANGE];
	Magic BishopMagics[SQUARE_RANGE];

    // Magic helpers (not public)
    // - Those tables serve as database for all precalculated sliding piece attacks
    // - All magics have pointers to appropriate part of one of those tables
    Bitboard RookTable[102400] = {};
	Bitboard BishopTable[5248] = {};


    // ---------------------------------------
    // Magics - lookup tables - initialization
    // ---------------------------------------

    // Helper definition - attack calculator
    using AttackMapFunction = Bitboard (*)(Square, Bitboard);

    // Helper function - partial magic initialization
    // - Takes attack calculator function (func) which allows to generalize for different piece types
    // - Uses randomized search approach to find good magic numbers
    // - Usually takes up to 2 seconds to initialize, depending on the seed
    void initialize_magics(Magic* magics, Bitboard* table, AttackMapFunction attack_calc)
    {
        // Hyperparameters
        constexpr int MAX_ATTACK_TABLE_SIZE = 4096;
		constexpr int RANDOM_SEED = 128;

        // Helper tables
        // - We use std::vector as a safe and simple way of allocating data on heap instead of stack
        std::vector<Bitboard> occupancies(MAX_ATTACK_TABLE_SIZE, 0),
                              attacks(MAX_ATTACK_TABLE_SIZE, 0),
                              mhelper(MAX_ATTACK_TABLE_SIZE, 0);

        int size = 0;

        // Magic values must be initilized for all possible placement of piece
        for (int sq = 0; sq < SQUARE_RANGE; sq++) {
            // Since attack maps do not change if we put any blockers on edge files or ranks, we can extract them
            // to make index smaller
            Bitboard edges = ((board::RANK_1 | board::RANK_8) & ~board::rank(rank_of(Square(sq)))) |
							 ((board::FILE_A | board::FILE_H) & ~board::file(file_of(Square(sq))));
			Bitboard mask = attack_calc(Square(sq), 0) & ~edges;

            Magic& m = magics[sq];
			m.mask = mask;
			m.shift = 64 - bitboards::popcount(m.mask);
			m.attacks = sq == SQ_A1 ? table : magics[sq - 1].attacks + size;    // Shift the pointer to appropriate location

            size = 0;

            // Now calculate attacks for every possible occupancy that affects attack map
            Bitboard bb = 0;
			do {
				occupancies[size] = bb;
				attacks[size] = attack_calc(Square(sq), bb);
				bb = (bb - mask) & mask;
				size++;
			} while (bb != 0);

            utilities::random::MagicsGenerator rng(RANDOM_SEED);

            // I will leave this code without explanation because it was so long ago the last time I touched it
            // that I don't even remember how does this shit work :)
			uint64_t magic;
			for (int i = 0; i < size; ) {
				for (magic = 0; bitboards::popcount((magic * mask) >> 56) < 6; magic = rng()) 
					continue;
				m.magic = magic;
				for (i = 0; i < size; i++) {
					int id = m.index(occupancies[i]);
					if (mhelper[id] != magic) {
						m.attacks[id] = attacks[i];
						mhelper[id] = magic;
					}
					else if (m.attacks[id] != attacks[i]) break;
				}
			}
        }
    }

    // Main magics initialization
    void initialize_magics()
    {
        initialize_magics(RookMagics, RookTable, pieces::__rook_attacks);
		initialize_magics(BishopMagics, BishopTable, pieces::__bishop_attacks);
    }
    
}