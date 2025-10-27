#include "pieces.h"


namespace chess::pieces {

	// ----------------------
	// Pieces - lookup tables
	// ----------------------

	Bitboard PawnAttacks[COLOR_RANGE][SQUARE_RANGE] = { 0 };
	Bitboard PseudoAttacks[PIECE_TYPE_RANGE][SQUARE_RANGE] = { 0 };


    // ----------------------
	// Pieces - initilization
	// ----------------------

    void initialize_attack_tables()
    {
        for (int sq = SQ_A1; sq <= SQ_H8; ++sq) {
			Bitboard squareBB = as_bitboard(Square(sq));
			PawnAttacks[WHITE][sq] = pawn_attacks<WHITE>(squareBB);
			PawnAttacks[BLACK][sq] = pawn_attacks<BLACK>(squareBB);
			PseudoAttacks[KNIGHT][sq] = knight_attacks(squareBB);
			PseudoAttacks[KING][sq] = king_attacks(squareBB);
			PseudoAttacks[BISHOP][sq] = __bishop_attacks(Square(sq), 0);
			PseudoAttacks[ROOK][sq] = __rook_attacks(Square(sq), 0);
			PseudoAttacks[QUEEN][sq] = __queen_attacks(Square(sq), 0);
		}
    }

}