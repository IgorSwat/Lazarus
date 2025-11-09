#pragma once

#include <chess/types.h>

// Forward declaration
namespace engine { class Board; }

namespace engine::zobrist {

    using chess::Square;
    using chess::Color;
    using chess::Piece;
    using chess::CastlingRights;

    // -----------------------------
    // Zobrist hash - initialization
    // -----------------------------

    // Initializes a global array of pseudo-random numbers used in zobrist hashing mechanism
    void initialize_zobrist_numbers();


    // --------------------------
    // Zobrist hash - definitions
    // --------------------------

    // Each Zobrist number is a single 64-bit hash value
    using Hash = uint64_t;

    // Each number is stored in precalculated table. Each element corresponds to given aspect of the position:
    // - 768 elements correspond to every possible placement of every piece (12 * 64)
    // - 16 elements correspond to any combination of castling rights
    // - 65 elements correspond to any possible enpassant square (including no enpassant available)
    // - 1 element corresponds to distinguish white to move vs black to move positions
    extern Hash ZobristNumbers[850];


    // ---------------------------
    // Zobrist hash - hash manager
    // ---------------------------

    // Wrapps a bare zobrist hash value with a generation & update logic
    class Manager
    {
    public:
        constexpr Manager() = default;

        // Getter
        Hash hash() const { return m_hash; }

        // Static update - (re)set & generate
        void set(Hash hash) { m_hash = hash; }
        void generate(const Board& board);      // Generates the hash for given position

        // Dynamic update
        void update(Piece piece, Square sq) { m_hash ^= ZobristNumbers[chess::color_of(piece) * 384 + chess::type_of(piece) * 64 + sq]; }  
        void update(CastlingRights rights)  { m_hash ^= ZobristNumbers[768 + rights]; }
        void update(Square epsquare)        { m_hash ^= ZobristNumbers[784 + epsquare]; }       
        void update(Color side2move)        { m_hash ^= ZobristNumbers[849]; }

    private:
        // Bare zobrist hash value
        Hash m_hash = 0;
    };

} // namespace engine::zobrist