#include "zobrist.h"
#include <algorithm>
#include <board/board.h>
#include <unordered_set>
#include <utilities/random.h>

namespace engine::zobrist {

    // ------------------------------------
    // Zobrist hash - zobrist numbers table
    // ------------------------------------

    // See https://en.wikipedia.org/wiki/Zobrist_hashing for more explanation
    Hash ZobristNumbers[850] = { 0 };


    // -----------------------------
    // Zobrist hash - initialization
    // -----------------------------

    void initialize_zobrist_numbers()
    {
        // The quality of generated numbers should not depend on the seed too much when using good RNG (such as MT algorithm)
        constexpr unsigned int SEED = 410375;

        utilities::random::StandardGenerator<Hash> rng(SEED);

        // We use a hash set to avoid duplications of hash key (each zobrist number must be distinctive from others)
        std::unordered_set<Hash> generated_codes = {};

        // Distinctive random number generation
        std::generate(ZobristNumbers, ZobristNumbers + 850, [&generated_codes, &rng]() {
            Hash code = 0;
            do
                code = rng();
            while (generated_codes.contains(code));

            // Save hash to not repeat it in the future
            generated_codes.insert(code);

            return code;
        });
    }


    // -------------------------------------------
    // Zobrist hash - hash manager - static update
    // -------------------------------------------

    void Manager::generate(const Board& board)
    {
        // To generate hash from scratch, we simply test position for every indyvidual hashing aspect
        m_hash = 0;

        // Piece placement hash
        for (int sq = 0; sq < SQUARE_RANGE; sq++) {
            Piece piece = board.on(Square(sq));
            if (piece != NO_PIECE)
                update(piece, Square(sq));
        }

        // Other aspects of the position
        if (board.side_to_move() == BLACK)
            update(BLACK);
        update(board.castling_rights());
        update(board.ep_square());
    }

}