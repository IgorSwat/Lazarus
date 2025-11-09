#pragma once

#include "zobrist.h"
#include <chess/pieces.h>
#include <chess/rules.h>
#include <iostream>
#include <moves/move.h>
#include <string>
#include <utilities/stable_stack.h>

namespace engine {

    using namespace chess;

    // --------------------
    // Board representation
    // --------------------

    class Board 
    {
    public:
        Board() { load_position(); }
        Board(const Board& other) { load_position(other); }

        // Position change - static loading
        void clear();                                                      // Clear the entire board
        void load_position() { load_position(rules::STARTING_POSITION); }  // Loads starting position
        void load_position(const std::string& fen);                        // Loads position from given FEN notation
        void load_position(const Board& other);                            // Loads position from different Board object
        
        // Position change - dynamic (move make & unmake)
        void make_move(const Move& move);
        void undo_move();                               // Undo only the last move, if any move was made
        void make_null_move();                          // Specialized null move (passing move) maker
        void undo_null_move();                          // Specialized null move (passing move) unmaker

        // Position analysis - piece-centric operations
        Bitboard pieces(Color side) const { return m_pieces_c[side]; }
        Bitboard pieces(PieceType ptype = ALL_PIECES) const { return m_pieces_t[ptype]; }
        template <typename... PieceTypes>
        Bitboard pieces(PieceType ptype, PieceTypes... types) const { return pieces(ptype) | pieces(types...); }
        template <typename... PieceTypes>
        Bitboard pieces(Color side, PieceTypes... types) const { return pieces(side) & pieces(types...); }
        Square king_position(Color side) const { return m_kings[side]; }

        // Position analysis - square-centric operations
        Piece on(Square sq) const { return m_board[sq]; }
        bool is_occupied(Square sq) const { return m_board[sq] != NO_PIECE; }
        Bitboard attackers_to(Square sq, Bitboard occ) const;
        Bitboard attackers_to(Square sq) const { return attackers_to(sq, pieces()); }                     // Attackers from both sides
        Bitboard attackers_to(Square sq, Color side) const { return attackers_to(sq, side, pieces()); }   // Attackers from given side
        Bitboard attackers_to(Square sq, Color side, Bitboard occ) const { return attackers_to(sq, occ) & pieces(side); }

        // Position analysis - checks & pins
        // - NOTE: all of those methods assumes that position is legal and only side to move can be in check or give a check
        bool in_check() const { return pos().checkers; }
        Bitboard checkers() const { return pos().checkers; }
        Bitboard possible_checks(PieceType ptype) const { return pos().check_areas[ptype]; }
        Bitboard pinned(Color side) const { return pos().pinned[side]; }
        Bitboard pinners(Color side) const { return pos().pinners[side]; }

        // Position analysis - castling properties
        CastlingRights castling_rights() const { return pos().castling_rights; }
        bool can_castle(Color side, Castle castle) const { return castling_rights() & make_castle_right(side, castle); }
        bool is_castle_path_clear(Color side, Castle castle) const { return !(rules::castle_path(side, castle) & pieces()); }

        // Position analysis - enpassant properties
        // - NOTE: Enpassant square is the position of the only pawn (if any) that could be captured en-passant in the next move
        Square ep_square() const { return pos().ep_square; }
        bool ep_available() const { return ep_square() != NULL_SQUARE; }

        // Position analysis - move count
        uint16_t n_halfmoves() const { return pos().halfmoves; }
        uint16_t n_moves() const { return (n_halfmoves() + 2 - m_moving_side) / 2; }
        uint16_t hf_clock() const { return pos().halfmove_clock; }

        // Position analysis - repetitions
        // - This breaks the single responsibility rule, but is a little bit quicker than 2 separate functions for count and for gap
        struct RepetitionData { uint16_t count; uint16_t gap; };    // NOTE: gap - distance (in halfmoves aka plies) to last repetition of the position
        RepetitionData repetitions() const;                         // Repetitions of current position

        // Position analysis - miscellaneous
        Color side_to_move() const { return m_moving_side; }
        Move last_move() const { return pos().last_move; }
        zobrist::Hash hash() const { return m_zobrist.hash(); }

        // Move analysis - legality checks
        // - Pseudo legal move is a move that could be legal if we would ignore pins and potential attacks on the king.
        //   In other words, pseudo legal move is geometrically correct in given position
        // - Legal move is a pseudo legal move with proved correctness in the context of pins and attacks on the king
        bool is_pseudolegal(const Move& move) const;
        bool is_legal(const Move& move) const { return is_pseudolegal(move) && maybe_legal(move); }          // Full leglity test
        bool maybe_legal(const Move& move) const;       // Partial test - does not check the pseudolegality of a move, only the pins & checks part

        // Move analysis - SEE (Static Exchange Evaluation)
        // - SEE allows to evaluate a serie of captures initiated by given move statically - without search
        // - Always assumes that both sides will make optimal trades that maximizes their material gain
        int32_t see(Square from, Square to, PieceType promote_to = NULL_PIECE_TYPE) const;
        int32_t see(const Move& move) const { return see(move.from(), move.to(), move.is_promotion() ? move.promotion_type() : NULL_PIECE_TYPE); }

        // Move analysis - other move properties
        bool is_check(const Move& move) const;

        // Text representation - FEN
        // - Reverse operation to load_position(fen)
        std::string fen() const;

        // Abstraction layer - logical operators
        // - Those comparisions compare all the logical aspects of two boards down to the top entry in position stack
        // - This means we do not compare the history of two boards, but just their current logical state
        // - For this reasons, we ommit checking halfmove counters and zobrist
        bool operator==(const Board& other) const;
        bool operator!=(const Board& other) const { return !(*this == other); }

        // Abstraction layer - text representation
        // - Use FEN representation to display the board
        friend std::ostream& operator<<(std::ostream& os, const Board& board) { os << board.fen(); return os; }

    private:
        // Helper functions - move makers
        void make_standard_move(const Move& move);
        void make_promotion(const Move& move);
        void make_enpassant(const Move& move);
        void make_castle(const Move& move);

        // Helper functions - piece placement handlers
        void place_piece(Piece piece, Square sq);
        void remove_piece(Square sq);
        void move_piece(Square from, Square to);

        // Helper functions - checks & pins update
        void update_checks();       // Updates all the check related informations for side to move only
        void update_pins();         // Updates all the pin related informations for both sides

        // Board state - side on move
        Color m_moving_side;

        // Board state - piece alignment
        Piece m_board[SQUARE_RANGE];              // A general table indexed by a square
        Bitboard m_pieces_t[PIECE_TYPE_RANGE];    // Aggregative piece map indexed by piece type (_t) (pieces of both colors)
        Bitboard m_pieces_c[COLOR_RANGE];         // Aggregative piece map indexed by piece color (_c) (pieces of all types)
        Square m_kings[COLOR_RANGE];              // Equivalent to m_pieces_c, but eliminates the necessity for converting a bitboard to square

        // Board state - persistant elements
        // - An aggregated set of board state properties, which are persisted within the position history branch (mostly for performance)
        struct PNode
        {
            PNode() = default;
            explicit PNode(const Move& last_move) : last_move(last_move) {}

            PNode& operator=(const PNode& other) = default;

            Bitboard checkers = 0;							    // Map of pieces that currently give a check (against side to move)
	        Bitboard check_areas[PIECE_TYPE_RANGE] = { 0 };	    // Map of possible checks for current side to move pieces
	        Bitboard discoveries[COLOR_RANGE] = { 0 };		    // Map of squares occupied by pieces which could cause a discovered check (for both sides)
	        Bitboard pinned[COLOR_RANGE] = { 0 };			    // Map of pinned pieces (for both sides)
	        Bitboard pinners[COLOR_RANGE] = { 0 };			    // Map of pieces that pin at least one of enemy's pieces ( for both sides)

            CastlingRights castling_rights = NO_RIGHTS;

            Square ep_square = NULL_SQUARE;                     // Enpassant square - a position of the pawn that moves 2 squares in the last move

            uint16_t halfmoves = 0;
            uint16_t halfmove_clock = 0;                        // A halfmove clock that resets after each capture or pawn move

            zobrist::Hash hash = 0;

            // Nodes logical connection - moves
            Move last_move = moves::null;
            Piece captured = NO_PIECE;
        };

        // Position history branch
        // - Allows to look back in the game history & undo moves quickly
        using PBranch = utilities::StableStack<memory::Storage::DYNAMIC, PNode, 16>;
        PBranch m_pbranch;

        PNode& pos() { return m_pbranch.top(); }                // An alias for current position (within the position branch)
        const PNode& pos() const { return m_pbranch.top(); }    // An alias for current position (within the position branch)

        // Hashing module
        zobrist::Manager m_zobrist;
    };
    
} // namespace engine