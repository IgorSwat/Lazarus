#include "board.h"
#include <chess/rules.h>
#include <exception>
#include <moves/move.h>
#include <sstream>
#include <string>

namespace engine {

    // ----------------------------------------
    // Board - position change - static loading
    // ----------------------------------------

    // Static loading - load empty board
    void Board::clear()
    {
        // Clear all the piece tables
        std::fill(m_board, m_board + SQUARE_RANGE, NO_PIECE);
        std::fill(m_pieces_c, m_pieces_c + COLOR_RANGE, 0);
        std::fill(m_pieces_t, m_pieces_t + PIECE_TYPE_RANGE, 0);
        std::fill(m_kings, m_kings + COLOR_RANGE, NULL_SQUARE);

        // Clear the position history
        // Shrinks the position branch to ply 0 only and reset the data
        m_pbranch.shrink();
        m_pbranch.top() = PNode();      // This replaces the initial node with a node compatible with an empty board
    }

    // Static loading - load from FEN notation
    void Board::load_position(const std::string& fen)
    {
        // Let's start with reseting current board state
        // - This simultaneously resets the history branch
        clear();

        // FEN parsing with std::istringstream
	    std::istringstream stream(fen);
        std::string part;

        // Part 1 - parse piece placement
        stream >> part;
        int rank = 7, file = 0;		// In FEN we start from upper side of a board

        for (char symbol : part) {
            // New rank mark
            if (symbol == '/') {
                rank--;		// Go to another rank
                file = 0;   // Reset file id
            }
            // Shift inside the current rank
            else if (std::isdigit(symbol))
			    file += static_cast<int>(symbol - '0');
            // Piece mark
            else {
                Square square = make_square(Rank(rank), File(file));
                Color color = std::isupper(symbol) ? WHITE : BLACK;
                PieceType type = std::tolower(symbol) == 'p' ? PAWN :
                                 std::tolower(symbol) == 'n' ? KNIGHT :
                                 std::tolower(symbol) == 'b' ? BISHOP :
                                 std::tolower(symbol) == 'r' ? ROOK :
                                 std::tolower(symbol) == 'q' ? QUEEN : KING;

                place_piece(make_piece(color, type), square);

                file++;
            }
        }

        // Part 2 - side to move parsing
	    stream >> part;
	    m_moving_side = part == "w" ? WHITE : BLACK;

        // Part 3 - castling rights parsing
	    stream >> part;
	    if (part.find('K') != std::string::npos)
		    pos().castling_rights |= make_castle_right(WHITE, KINGSIDE_CASTLE);
	    if (part.find('Q') != std::string::npos)
            pos().castling_rights |= make_castle_right(WHITE, QUEENSIDE_CASTLE);
	    if (part.find('k') != std::string::npos)
            pos().castling_rights |= make_castle_right(BLACK, KINGSIDE_CASTLE);
	    if (part.find('q') != std::string::npos)
            pos().castling_rights |= make_castle_right(BLACK, QUEENSIDE_CASTLE);
        
        // Part 4 - enpassant parsing
	    stream >> part;
	    if (part != "-") {
		    Rank epRank = m_moving_side == WHITE ? RANK_5 : RANK_4;
		    File epFile = File(part.front() - 'a');

		    pos().ep_square = make_square(epRank, epFile);
	    }

        // Part 5 - move counters
	    stream >> pos().halfmove_clock;
	    stream >> pos().halfmoves;
	    pos().halfmoves = pos().halfmoves * 2 + m_moving_side - 2;      // A reversed formula to moves_p()

        // Checks & pins update
        update_checks();
        update_pins();

        // Zobrist hash update (static)
        m_zobrist.generate(*this);
        pos().hash = m_zobrist.hash();
    }

    // Static loading - load from other board state
    void Board::load_position(const Board& other)
    {
        // Copy common data
        m_moving_side = other.m_moving_side;
        std::copy(other.m_board, other.m_board + SQUARE_RANGE, m_board);
        std::copy(other.m_pieces_c, other.m_pieces_c + COLOR_RANGE, m_pieces_c);
	    std::copy(other.m_pieces_t, other.m_pieces_t + PIECE_TYPE_RANGE, m_pieces_t);
	    std::copy(other.m_kings, other.m_kings + COLOR_RANGE, m_kings);

        // Reset the stack and simply copy position data from other
        m_pbranch.shrink();
        m_pbranch.top() = other.m_pbranch.top();

        // Clear fields related to last move (since we do not keep track of previous positions from other board)
        m_pbranch.top().last_move = moves::null;
        m_pbranch.top().captured = NO_PIECE;

        // Load zobrist hash
        m_zobrist.set(m_pbranch.top().hash);
    }


    // --------------------------------------------------------------
    // Board - position change - dynamic (move making) - normal moves
    // --------------------------------------------------------------

    // Main move maker function
    void Board::make_move(const Move& move)
    {
        // Push position branch to make room for new ply data
        m_pbranch.push();
        pos().last_move = move;

        // Delegate further updates to specialized function for each move type
        switch (move.type()) {
            case MoveType::NORMAL:
                make_standard_move(move);
                break;
            case MoveType::PROMOTION:
                make_promotion(move);
                break;
            case MoveType::ENPASSANT:
                make_enpassant(move);
                break;
            case MoveType::CASTLE:
                make_castle(move);
                break;
            default:
                throw std::runtime_error("error: invalid move type in Board::make_move()");
        }

        // Side to move change
        m_moving_side = ~m_moving_side;
        m_zobrist.update(m_moving_side);

        // Check & pin update - common for all types of moves
        // - NOTE: It's important to update side to move before update_checks() call
        update_checks();
	    update_pins();

        // Halfmove counter update - common for all types of moves
        pos().halfmoves++;

        // Hash update - common for all types of moves
        pos().hash = m_zobrist.hash();
    }

    // Specialized move makers - standard (normal) moves
    void Board::make_standard_move(const Move& move)
    {
        Square from = move.from();
        Square to = move.to();
        Piece piece = m_board[from];

        // Step 1 - update piece placement & related properties
        // - Normal move can be either quiet or capture
        if (move.is_capture()) {
            pos().captured = m_board[to];

            m_zobrist.update(m_board[to], to);
            remove_piece(to);
        }

        m_zobrist.update(piece, from);
        m_zobrist.update(piece, to);
        move_piece(from, to);

        // Step 2 - update castling rights
        // - For more exaplanation, see CastleLoss table comments
        m_zobrist.update(m_pbranch.top_n(1).castling_rights);
        pos().castling_rights = m_pbranch.top_n(1).castling_rights & 
                                          ~rules::CastleLoss[from] & 
                                          ~rules::CastleLoss[to];
        m_zobrist.update(pos().castling_rights);

        // Step 3 - update enpassant square
        // - Every normal move resets enpassant square except double pawn pushes
        m_zobrist.update(m_pbranch.top_n(1).ep_square);
        pos().ep_square = move.is_double_pawn_push() && board::adjacent_rank_squares(to) & pieces(~m_moving_side, PAWN) ? to : NULL_SQUARE;
        m_zobrist.update(pos().ep_square);

        // Step 4 - other updates
        pos().halfmove_clock = move.is_capture() || type_of(piece) == PAWN ? 0 : m_pbranch.top_n(1).halfmove_clock + 1;
    }

    // Specialized move makers - promotions
    void Board::make_promotion(const Move& move)
    {
        // We know that moving piece is a pawn
        Square from = move.from();
        Square to = move.to();
        Piece promoted = make_piece(m_moving_side, move.promotion_type());

        // Step 1 - update piece placement & related properties
        // - Promotions, similarly to normal moves, can rither capture something or not
        // - We handle promotions with remove_piece + place_piece instead of move_piece to change piece type
        if (move.is_capture()) {
            pos().captured = m_board[to];

            m_zobrist.update(m_board[to], to);
            remove_piece(to);
        }
        
        m_zobrist.update(m_board[from], from);
        remove_piece(from);
        m_zobrist.update(promoted, to);
        place_piece(promoted, to);

        // Step 2 - update castling rights
        // - Promotion cannot affect castling rights unless it comes with a capture of enemy rook
        pos().castling_rights = m_pbranch.top_n(1).castling_rights;
        if (move.is_capture()) {
            m_zobrist.update(m_pbranch.top_n(1).castling_rights);
            pos().castling_rights &= ~rules::CastleLoss[to];
            m_zobrist.update(pos().castling_rights);
        }

        // Step 3 - update enpassant square
        // - Promotion always reset enpassant square
        m_zobrist.update(m_pbranch.top_n(1).ep_square);
        pos().ep_square = NULL_SQUARE;
        m_zobrist.update(NULL_SQUARE);

        // Step 4 - other updates
        pos().halfmove_clock = 0;
    }

    // Specialized move makers - enpassant
    void Board::make_enpassant(const Move& move)
    {
        // We know that moving piece is a pawn
        Square from = move.from();
        Square to = move.to();
        Square capture_square = m_pbranch.top_n(1).ep_square;

        // Step 1 - update piece placement & related properties
        // - Enpassant is by definition always a capture
        // - Unlike the normal captures, enpassant captures the pawn outside the target (to) square - on enpassant square
        pos().captured = m_board[capture_square];

        m_zobrist.update(m_board[capture_square], capture_square);
        remove_piece(capture_square);
        m_zobrist.update(m_board[from], from);
        m_zobrist.update(m_board[from], to);
        move_piece(from, to);

        // Step 2 - update castling rights
        // - Enpassant cannot affect castling rights in any way
        pos().castling_rights = m_pbranch.top_n(1).castling_rights;

        // Step 3 - update enpassant square
        // - Enpassant always resets enpassant square
        m_zobrist.update(capture_square);   // capture_square is simultanously an enpassant square from previous ply
        pos().ep_square = NULL_SQUARE;
        m_zobrist.update(NULL_SQUARE);

        // Step 4 - other updates
        pos().halfmove_clock = 0;
    }

    // Specialized move makers - castle
    void Board::make_castle(const Move& move)
    {
        // Move contains starting and target square for king shift
        Square king_from = move.from();
        Square king_to = move.to();
        Square rook_from = make_square(rank_of(king_to), file_of(king_to) == FILE_G ? FILE_H : FILE_A);
        Square rook_to = make_square(rank_of(king_to), file_of(king_to) == FILE_G ? FILE_F : FILE_D);

        // Step 1 - update piece placement & related properties
        // - Castle consists of two independent moves: king move by 2 squares, and appropriate rook move
        m_zobrist.update(m_board[king_from], king_from);
        m_zobrist.update(m_board[king_from], king_to);
        move_piece(king_from, king_to);
        m_zobrist.update(m_board[rook_from], rook_from);
        m_zobrist.update(m_board[rook_from], rook_to);
        move_piece(rook_from, rook_to);

        // Step 2 - update castling rights
        // - Castling discards all the castling rights for castling side
        // - Equivalent to masking by castle loss for king move
        m_zobrist.update(m_pbranch.top_n(1).castling_rights);
        pos().castling_rights = m_pbranch.top_n(1).castling_rights & ~rules::CastleLoss[king_from];
        m_zobrist.update(pos().castling_rights);

        // Step 3 - update enpassant square
        // - Castling always resets enpassant square
        m_zobrist.update(m_pbranch.top_n(1).ep_square);
        pos().ep_square = NULL_SQUARE;
        m_zobrist.update(NULL_SQUARE);

        // Step 4 - other updates
        // - Castling is considered irreversible move, but it does not reset halfmove clock
        pos().halfmove_clock = m_pbranch.top_n(1).halfmove_clock + 1;
    }

    // Unmaking moves
    // - NOTE: since zobrist hash is already stored on position branch, we can ommit any dynamic upates of zobrist hashing object
    void Board::undo_move()
    {
        // If positon branch is empty, then there are no more moves to be unmade
        if (m_pbranch.empty())
            return;

        const Move& last_move = pos().last_move;
        Square from = last_move.from();
        Square to = last_move.to();

        // Step 1 - revert piece placement changes
        // - Reverting piece move can be done by moving with reverse from-to squares
        // - Additionally, all captured pieces must be placed back onto the board
        switch (last_move.type()) {
            case MoveType::NORMAL:
                move_piece(to, from);
                if (last_move.is_capture()) 
                    place_piece(pos().captured, to);
                break;
            case MoveType::PROMOTION:
                remove_piece(to);
                if (last_move.is_capture())
                    place_piece(pos().captured, to);
                place_piece(make_piece(~m_moving_side, PAWN), from);
                break;
            case MoveType::ENPASSANT:
                move_piece(to, from);
                place_piece(pos().captured, m_pbranch.top_n(1).ep_square);
                break;
            case MoveType::CASTLE:
                move_piece(to, from);	// King
                move_piece(make_square(rank_of(to), file_of(to) == FILE_G ? FILE_F : FILE_D),
                           make_square(rank_of(to), file_of(to) == FILE_G ? FILE_H : FILE_A));	// Rook
                break;
            default:
                return;
        }

        // Step 2 - undo side to move and halfmove counter changes
        m_moving_side = ~m_moving_side;

        // Step 3 - revert all the other changes by decrementing position branch
        m_pbranch.pop();

        // Step 4 - restore zobrist hash
        m_zobrist.set(pos().hash);
    }


    // -------------------------------------------------------------------
    // Board - position change - dynamic (move make & unmake) - null moves
    // -------------------------------------------------------------------

    // Specialized null move (passing move) maker
    void Board::make_null_move()
    {
        // Push position branch to make room for new ply data
        m_pbranch.push();

        // Null move is a passing move with the following properties:
        // - Null move changes side to move
        // - Null move does not affect piece placement and castling rights
        // - Null move might reset enpassant square if it's not a NULL_SQUARE yet
        // - Null move requires update_checks() to be called, since this method is relative to current side to move

        // Change side to move
        m_moving_side = ~m_moving_side;
        m_zobrist.update(m_moving_side);

        // Copy position data and change only relevant properties
        m_pbranch.top() = m_pbranch.top_n(1);

        pos().last_move = moves::null;
        pos().captured = NO_PIECE;

        m_zobrist.update(m_pbranch.top_n(1).ep_square);
        pos().ep_square = NULL_SQUARE;
        m_zobrist.update(NULL_SQUARE);

        pos().hash = m_zobrist.hash();

        // Calculate checks for new side to move
        update_checks();

        // NOTE: since null move is not really a legal move, we do not increase halfmoves counter
    }

    // Specialized null move (passing move) unmaker
    void Board::undo_null_move()
    {
        // Since null move does not affect piece placement, reverting it is very simple
        m_moving_side = ~m_moving_side;
        m_pbranch.pop();
        m_zobrist.set(pos().hash);
    }


    // -----------------------------------------------------
    // Board - position analysis - square centric operations
    // -----------------------------------------------------

    Bitboard Board::attackers_to(Square sq, Bitboard occ) const
    {
        return pieces::pawn_attacks(WHITE, sq) & pieces(BLACK, PAWN) | 
		       pieces::pawn_attacks(BLACK, sq) & pieces(WHITE, PAWN) |
		       pieces::piece_attacks<KNIGHT>(sq) & pieces(KNIGHT) |
		       pieces::piece_attacks<BISHOP>(sq, occ) & pieces(BISHOP, QUEEN) |
		       pieces::piece_attacks<ROOK>(sq, occ) & pieces(ROOK, QUEEN) |
		       pieces::piece_attacks<KING>(sq) & pieces(KING);
    }


    // ---------------------------------------
    // Board - position analysis - repetitions
    // ---------------------------------------

    Board::RepetitionData Board::repetitions() const
    {
        // Repetition of a position requires at least 2 moves from both sides - one to go to a different position, and one to go back
        // For this reason we know that repetition cannot happen if last irreversible move came less then 2 moves (4 plies) ago
        // - NOTE: irreversible moves = moves that reset the halfmove clock + castles
        if (pos().halfmove_clock < 4)
            return {1 ,0};

        // We use board hash as a comparison key
        zobrist::Hash curr_hash = pos().hash;

        uint16_t count = 1;                     // We are already counting the current position as first occurance
        uint16_t distance = 0;

        // Since repetition of current position can happen only at current side to move, we can limit search to only
        // plies with current side to move as a moving side
        // - Determines the maximal range of search inside position stack
        uint16_t loops = pos().halfmove_clock / 2;     
        
        for (int i = 1; i <= loops; i++) {
            if (m_pbranch.size() < 2 * i + 1)
                break;
            
            // We assume that positions are the same if their hashes are equal, altrough it's not always true
            // (due to limited range of hash values)
            if (curr_hash == m_pbranch.top_n(2 * i).hash) {
                count++;
                distance = distance == 0 ? pos().halfmove_clock - m_pbranch.top_n(2 * i).halfmove_clock : distance;
            }
        }

        return {count, distance};
    }


    // ---------------------------------------
    // Board - move analysis - legality checks
    // ---------------------------------------

    bool Board::is_pseudolegal(const Move& move) const
    {
        Square from = move.from(), to = move.to();
        Piece piece = m_board[from];

        // There has to be a piece on starting square of the move
        if (piece == NO_PIECE)
            return false;

        Color side = color_of(piece);
        Color enemy = ~side;
        Direction forward = side == WHITE ? NORTH : SOUTH;

        if (side !=m_moving_side)
            return false;

        // If target square is not empty, it cannot be occupied by friendly piece and move has to be capture
        if (m_board[to] != NO_PIECE && (!move.is_capture() || color_of(m_board[to]) == side))
            return false;

        // If target square is empty, then move cannot be a capture
        if (m_board[to] == NO_PIECE && move.is_capture() && !move.is_enpassant())
            return false;

        // Ensure that move is a check evasion if the side is in check
        if (in_check()) {
            // Piece moves
            if (type_of(piece) != KING) {
                // A piece move can never be a check evasion in case of double check
                if (!bitboards::singly_populated(pos().checkers))
                    return false;

                Square checkSquare = bitboards::lsb(pos().checkers);
                if (to != checkSquare &&
                    (!move.is_enpassant() || to != ep_square() + forward) &&
                    !board::aligned_in_order(m_kings[side], to, checkSquare))
                    return false;
            }
            // King moves
            else if (attackers_to(to, enemy, pieces() ^ from))
                return false;
        }

        // Special case - castle
        if (move.is_castle()) {
            if (!can_castle(side, move.castle_type()) || !is_castle_path_clear(side, move.castle_type()) || in_check())
                return false;
        }
        // Special case - pawn moves
        else if (type_of(piece) == PAWN) {
            Bitboard secondRank = side == WHITE ? RANK_2 : RANK_7;

            if (!(move.is_enpassant() && ep_square() != NULL_SQUARE &&
                  board::adjacent_rank_squares(ep_square()) & from && to == ep_square() + forward) &&                      // Enpassant														// Enpassant
                !(move.is_capture() && !move.is_enpassant() && pieces::pawn_attacks(side, from) & to) &&                   // Capture
                !(to == from + forward && m_board[to] == NO_PIECE) &&                                                      // 1-push
                !(to == from + forward + forward && !(board::Paths[from][to] & (pieces() ^ from)) && secondRank & from))   // 2-push
                return false;
        }
        // Common moves - special flags must be 0
        else if (move.flags() & 0xb)
            return false;
        // Common moves - moving correctness
        else if (!(pieces::pseudo_attacks(type_of(piece), from) & to) ||
                 board::Paths[from][to] & pieces() & ~(as_bitboard(from) | to))
            return false;

        return true;
    }

    bool Board::maybe_legal(const Move& move) const
    {
        Square from = move.from(), to = move.to();
        Piece piece = m_board[from];

        Color side = color_of(piece);
        Color enemy = ~side;

        // Special case - enpassant
        // It requires a check whether a move would create a discovered attack against our king
        if (move.is_enpassant()) {
            Bitboard occ = (pieces() ^ ep_square() ^ from) | to;
            return !(pieces::piece_attacks<BISHOP>(m_kings[side], occ) & pieces(enemy, BISHOP, QUEEN)) &&
                   !(pieces::piece_attacks<ROOK>(m_kings[side], occ) & pieces(enemy, ROOK, QUEEN));
        }

        // Special case - castle
        if (move.is_castle()) {
            Direction dir = to > from ? EAST : WEST;

            // King can't castle via attacked squares
            return !attackers_to(from + dir, enemy, pieces()) &&
                   !attackers_to(to, enemy, pieces());
        }

        // King can't step into a square attacked by enemy piece
        if (type_of(piece) == KING)
            return !attackers_to(to, enemy, pieces() ^ from);

        // Finally, check for pins
        return !(pinned(side) & from) || 
               board::aligned_in_order(m_kings[side], from, to) ||
               board::aligned_in_order(m_kings[side], to, from);
    }


    // --------------------------------------------------------
    // Board - move analysis - SEE (static exchange evaluation)
    // --------------------------------------------------------

    // Helper function - finding the least valuable attacker
    // - Saves the least valuable attacker piece type to type parameter
    // - We assume that piece types are already ordered by increasing value (which in fact, is true for PieceType enum)
    Square lvp(const Board& board, Color side, Bitboard area, PieceType &type)
    {
        for (type = PAWN; type <= KING; type = PieceType(type + 1))
        {
            // area is just a set of squares from which we try to find least valuable attackers
            Bitboard attackers = area & board.pieces(side, type);

            // First found attacker must be of the lowest value
            if (attackers)
                return bitboards::lsb(attackers);
        }

        return NULL_SQUARE;
    }

    // Main SEE procedure
    int32_t Board::see(Square from, Square to, PieceType promote_to) const
    {
        // First, let's cover a null move case
        if (from == to)
            return 0;
        
        // Gain is a helper table used to calculate material balance change after best exchanges in each ply
        // - Since there are no more than 32 pieces on the board, there can be at most 32 exchanges (we include kings for simplicity)
        int32_t gain[33];

        // Depth is an index to gain table
        int depth = int(m_moving_side);

        [[maybe_unused]] PieceType attacked = type_of(on(to));
        [[maybe_unused]] PieceType attacker = type_of(on(from));

        // - attackdef is a map covering all pieces attacking given square (from both sides)
        // - possible_xray is a set of all sliding pieces and pawns
        Bitboard occ = pieces();
        Bitboard attackdef = attackers_to(to, occ);
        Bitboard possible_xray = pieces() ^ pieces(KNIGHT) ^ pieces(KING);

        // Special case - quiet pawn move
        // - Pawns are special in the way that pushing them can never be capture
        // - Updating attackdef in this case is crucial for method to not forget to include pawn value in exchanges
        if (attacker == PAWN && file_of(from) == file_of(to))
            attackdef |= from;

        // TODO: uncomment after adding the evaluation module
        // gain[depth] = Evaluation::PieceValues[attacked] + Evaluation::PieceValues[promote_to]
        //                                                 - Evaluation::PieceValues[PAWN];

        do {
            depth++;

            // TODO: uncomment after adding the evaluation module
            // gain[depth] = Evaluation::PieceValues[attacker] - gain[depth - 1];

            // No point to go any further if last best capture resulted in loss of material
            if (std::max(-gain[depth - 1], gain[depth]) < 0)
                break;
            
            attackdef = attackdef ^ from;
            if (possible_xray & from) {
                Bitboard xray_attackers = (pieces::xray_attacks<BISHOP>(to, occ, as_bitboard(from)) & pieces(BISHOP, QUEEN)) |
										   (pieces::xray_attacks<ROOK>(to, occ, as_bitboard(from)) & pieces(ROOK, QUEEN));
				attackdef |= xray_attackers;
            }

            // Remove piece that made a capture from occupancy set
            occ = occ ^ from;

            // Find next best (least valuable) attacker
            from = lvp(*this, Color(depth & 0x1), attackdef, attacker);
        } while (from != NULL_SQUARE);

        while (--depth)
			gain[depth - 1] = -std::max(-gain[depth - 1], gain[depth]);

		return gain[m_moving_side];
    }


    // ----------------------------------------
    // Board - move analysis - other properties
    // ----------------------------------------

    bool Board::is_check(const Move& move) const
    {
        Square from = move.from();
        Square to = move.to();

        bool is_direct_check = pos().check_areas[type_of(m_board[from])] & to;
        bool is_discovered_check = pos().discoveries[m_moving_side] & from &&
                                   !board::aligned_in_order(m_kings[~m_moving_side], to, from) &&
                                   !board::aligned_in_order(m_kings[~m_moving_side], from, to);

        return is_direct_check || is_discovered_check;
    }


    // --------------------------
    // Board - FEN representation
    // --------------------------

    std::string Board::fen() const
    {
        // Let's start with an empty string object and fill it in consecutive steps
        std::ostringstream fen;

        // Iterate over all squares in top -> down and left -> right order (as denoted in FEN)
        for (int r = RANK_8; r >= 0; r--) {
            int gap = 0;

            for (int f = FILE_A; f <= FILE_H; f++) {
                Piece piece = m_board[make_square(Rank(r), File(f))];

                if (piece != NO_PIECE) {
                    if (gap > 0)
                        fen << gap;
                    gap = 0;

                    fen << piece;
                }
                else
                    gap++;
            }

            if (gap > 0)
                fen << gap;
            if (r > 0)
                fen << "/";
        }

        // Side to move
        fen << " " << (m_moving_side == WHITE ? "w" : "b") << " ";

        // Castling rights
        if (can_castle(WHITE, KINGSIDE_CASTLE))
            fen << "K";
        if (can_castle(WHITE, QUEENSIDE_CASTLE))
            fen << "Q";
        if (can_castle(BLACK, KINGSIDE_CASTLE))
            fen << "k";
        if (can_castle(BLACK, QUEENSIDE_CASTLE))
            fen << "q";
        if (castling_rights() == NO_RIGHTS)
            fen << "-";
        
        // Enpassant
        fen << " ";
        if (ep_square() != NULL_SQUARE && rank_of(ep_square()) == RANK_4 &&
            board::adjacent_rank_squares(ep_square()) & pieces(~m_moving_side, PAWN))
            fen << make_square(RANK_3, file_of(ep_square()));
        else if (ep_square() != NULL_SQUARE && 
                 board::adjacent_rank_squares(ep_square()) & pieces(~m_moving_side, PAWN))
            fen << make_square(RANK_6, file_of(ep_square()));
        else
            fen << "-";

        // Halfmove clock
        fen << " " << hf_clock();

        // Move count
        fen << " " << n_moves();

        return fen.str();
    }


    // ---------------------------------------
    // Board - abstraction layer - comparisons
    // ---------------------------------------

    bool Board::operator==(const Board& other) const
    {
        // NOTE: we do not compare move counters here
        return m_moving_side == other.m_moving_side &&
               std::equal(m_board, m_board + SQUARE_RANGE, other.m_board) &&
               std::equal(m_pieces_c, m_pieces_c + COLOR_RANGE, other.m_pieces_c) &&
               std::equal(m_pieces_t, m_pieces_t + PIECE_TYPE_RANGE, other.m_pieces_t) &&
               std::equal(m_kings, m_kings + COLOR_RANGE, other.m_kings) &&
               castling_rights() == other.castling_rights() &&
               ep_square() == other.ep_square() &&
               checkers() == other.checkers() &&
               std::equal(pos().check_areas, pos().check_areas + PIECE_TYPE_RANGE, other.pos().check_areas) &&
               std::equal(pos().discoveries, pos().discoveries + COLOR_RANGE, other.pos().discoveries) &&
               std::equal(pos().pinned, pos().pinned + COLOR_RANGE, other.pos().pinned) &&
               std::equal(pos().pinners, pos().pinners + COLOR_RANGE, other.pos().pinners);
    }


    // ---------------------------------------------------
    // Board - helper functions - piece placement handlers
    // ---------------------------------------------------

    void Board::place_piece(Piece piece, Square sq)
    {
        // Update piece tables by adding a piece
        m_board[sq] = piece;
	    m_pieces_c[color_of(piece)] |= sq;
	    m_pieces_t[type_of(piece)] |= sq;
	    m_pieces_t[ALL_PIECES] |= sq;

        // Update king position in case of placing a king
	    if (type_of(piece) == KING) 
            m_kings[color_of(piece)] = sq;
    }

    void Board::remove_piece(Square sq)
    {
        // Get the piece standing on sq
        Piece piece = on(sq);

        // Update piece tables by removing a piece from square sq
        if (piece != NO_PIECE) {
            m_board[sq] = NO_PIECE;
	        m_pieces_c[color_of(piece)] ^= sq;
	        m_pieces_t[type_of(piece)] ^= sq;
	        m_pieces_t[ALL_PIECES] ^= sq;
        }

        // NOTE: We assume that king can never be captured or removed from the board,
        //       so we ignore updating the king position.
    }

    void Board::move_piece(Square from, Square to)
    {
        // Get the piece standing on starting square (from)
        Piece piece = on(from);

        // Update piece tables by moving a piece
        if (piece != NO_PIECE) {
            m_board[from] = NO_PIECE;
            m_board[to] = piece;

            // Calculate move shift map
            Bitboard movemap = as_bitboard(from) | as_bitboard(to);

            // By applying map with both from and to bits, we can perform the update in just 1 operation instead of 2
            m_pieces_c[color_of(piece)] ^= movemap;
	        m_pieces_t[type_of(piece)] ^= movemap;
	        m_pieces_t[ALL_PIECES] ^= movemap;

            // Update king position in case of moving the king
            if (type_of(piece) == KING) 
                m_kings[color_of(piece)] = to;
        }
    }


    // -----------------------------------------------
    // Board - helper functions - checks & pins update
    // -----------------------------------------------

    void Board::update_checks()
    {
        // To detect pieces that check the king, we can simply detect all attackers to king's position
        pos().checkers = attackers_to(m_kings[m_moving_side], ~m_moving_side);

        // Detecting check areas works similarly, but instead of aggregative attackers_to we use 
        // specialized piece-attack functions.
        pos().check_areas[PAWN] = pieces::pawn_attacks(~m_moving_side, m_kings[~m_moving_side]);
	    pos().check_areas[KNIGHT] = pieces::piece_attacks<KNIGHT>(m_kings[~m_moving_side], pieces());
	    pos().check_areas[BISHOP] = pieces::piece_attacks<BISHOP>(m_kings[~m_moving_side], pieces());
	    pos().check_areas[ROOK] = pieces::piece_attacks<ROOK>(m_kings[~m_moving_side], pieces());
	    pos().check_areas[QUEEN] = pos().check_areas[BISHOP] | pos().check_areas[ROOK];
    }

    void Board::update_pins()
    {
        for (int side = WHITE; side <= BLACK; side++) {
            // For readability
            Color enemy = ~Color(side);

            // Reset relevant tables first
            pos().pinned[side] = 0;
            pos().pinners[enemy] = 0;
            pos().discoveries[enemy] = 0;

            // We can express pin as a some sort of an x-ray attack, but with piece of opposite color as a blocker
            // - NOTE: Here we also take discovered attack into consideretion (x-ray with same side piece as a blocker)
            Bitboard xray_attackers = (pieces::xray_attacks<ROOK>(m_kings[side], pieces(), pieces()) & pieces(enemy, ROOK, QUEEN)) |
                                      (pieces::xray_attacks<BISHOP>(m_kings[side], pieces(), pieces()) & pieces(enemy, BISHOP, QUEEN));
            
            // Iterate over x-ray attackers map to extract each pin square
            while (xray_attackers) {
                Square sq = bitboards::pop_lsb(xray_attackers);
                Bitboard discovery = (board::Paths[m_kings[side]][sq] & pieces(enemy)) ^ sq;

                // Potential discovery
                if (discovery)
                    pos().discoveries[enemy] |= discovery;
                // If it's not a potential discovery, then it's a pin
                else {
                    pos().pinned[side] |= board::Paths[m_kings[side]][sq] & pieces(Color(side));
                    pos().pinners[enemy] |= sq;
                }
            }

            // Discard king square from pin set, since king cannot be pinned
            // - The above algorithm can incorrectly classify king as pinned in case of a check
            pos().pinned[side] &= ~m_kings[side];
        }
    }

}