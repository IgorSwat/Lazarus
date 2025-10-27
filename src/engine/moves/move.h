#pragma once

#include "types.h"
#include "../chess/types.h"


namespace Chess {

    // -------------------------------
    // Helper definitions - move flags
    // -------------------------------

    namespace Moves {

        // A unique identifier of a move. Consists of (from most important to least important bits):
        // - 4 bits for move flags (promotion bit, capture bit, and two special bits indicating special types of moves)
        // - 6 bits for target (to) square
        // - 6 bits for start (from) square
        using Mask = uint16_t;

        // Flags are part of full move mask - more specyfically, 4 most important bits
        // - To map flags into correct place in move mask, one should shift flags left by 12 bits
        // - NOTE: all flags must be possible to save using 4 bits (not exceed 0xf)
        using Flags = Mask;

        // Predefined flags - elementary
        constexpr Flags SPECIAL1_FLAG = 0x1;
        constexpr Flags SPECIAL2_FLAG = 0x2;
        constexpr Flags CAPTURE_FLAG = 0x4;
        constexpr Flags PROMOTION_FLAG = 0x8;

        // Predefined flags - complex
        // - Built by combining elementary flags and giving them context
        constexpr Flags QUIET_MOVE_FLAG         = 0x0;
        constexpr Flags NON_QUIET_MOVE_FLAG     = CAPTURE_FLAG | PROMOTION_FLAG;
        constexpr Flags DOUBLE_PAWN_PUSH_FLAG   = SPECIAL1_FLAG;
        constexpr Flags ENPASSANT_FLAG          = CAPTURE_FLAG | SPECIAL1_FLAG;
        constexpr Flags KINGSIDE_CASTLE_FLAG    = SPECIAL2_FLAG;
        constexpr Flags QUEENSIDE_CASTLE_FLAG   = SPECIAL2_FLAG | SPECIAL1_FLAG;
        constexpr Flags KNIGHT_PROMOTION_FLAG   = PROMOTION_FLAG;
        constexpr Flags BISHOP_PROMOTION_FLAG   = PROMOTION_FLAG | SPECIAL1_FLAG;
        constexpr Flags ROOK_PROMOTION_FLAG     = PROMOTION_FLAG | SPECIAL2_FLAG;
        constexpr Flags QUEEN_PROMOTION_FLAG    = PROMOTION_FLAG | SPECIAL2_FLAG | SPECIAL1_FLAG;

        // Specify move category based on given move flags
        constexpr inline MoveType type_of(Flags flags)
        {
            return flags >= PROMOTION_FLAG ? PROMOTION :
                   flags == ENPASSANT_FLAG ? ENPASSANT :
                   flags == KINGSIDE_CASTLE_FLAG || flags == QUEENSIDE_CASTLE_FLAG ? CASTLE : NORMAL;
        }

    }
    

    // ------------------------------
    // Moves definitions - plain move
    // ------------------------------

    // The primary idea behind the following class is to provide an useful abstraction for chess move
    // - It only covers the logic behind chess move, does not contain any additional info about move
    class Move
    {
    public:
        // Constructors
        constexpr Move() = default;                                                         // Static initialization
        Move(Square from, Square to, Moves::Flags flags) :
            m_move((flags & 0xf) << 12 | Moves::Mask(to) << 6 | Moves::Mask(from)) {}       // Dynamic (runtime) initialization

        // Getters - move squares
        Square from() const { return Square(m_move & 0x3f); }
	    Square to() const { return Square((m_move >> 6) & 0x3f); }

        // Getters - mask & flags
        Moves::Mask raw() const { return m_move; }                 // Returns the whole mask (a complete move identifier)
        Moves::Mask butterfly() const { return m_move & 0x0fff; }  // Returns a butterfly index (indentyfing move by from and to squares)
        Moves::Flags flags() const { return m_move >> 12; }        // Returns just a move flags (specyfing move type)

        // Getters - move type & properties
        MoveType type() const { return Moves::type_of(flags()); }
        PieceType promotion_type() const { return is_promotion() ? PieceType((flags() & 0x3) + 2) : NULL_PIECE_TYPE; }
        Castle castle_type() const { return is_castle() ? Castle((flags() & 0x3) - 1) : NO_CASTLE; }
        bool is_capture() const { return flags() & Moves::CAPTURE_FLAG; }
        bool is_promotion() const { return flags() & Moves::PROMOTION_FLAG; }
        bool is_quiet() const { return !(flags() & Moves::NON_QUIET_MOVE_FLAG); }
        bool is_double_pawn_push() const { return flags() == Moves::DOUBLE_PAWN_PUSH_FLAG; }
        bool is_enpassant() const { return flags() == Moves::ENPASSANT_FLAG; }
        bool is_castle() const { return flags() == Moves::KINGSIDE_CASTLE_FLAG || flags() == Moves::QUEENSIDE_CASTLE_FLAG; }

        // Logical operators - comparisions
        friend bool operator==(const Move& m1, const Move& m2) { return m1.m_move == m2.m_move; }
	    friend bool operator!=(const Move& m1, const Move& m2) { return m1.m_move != m2.m_move; }

        // Printing
        friend std::ostream& operator<<(std::ostream& os, const Move& move);

    protected:
        Moves::Mask m_move = 0;
    };


    // ----------------------------
    // Move definitions - null move
    // ----------------------------

    namespace Moves {

        // A singleton null move representation
        // - In exact terms, we represent null move as a quiet move from A1 to A1
        inline constexpr Move null;

    }

}