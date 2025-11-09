#pragma once

#include <bitboards/types.h>
#include <cinttypes>
#include <cstring>
#include <iostream>
#include <type_traits>


namespace chess {

	// ---------------------------------------
	// Types definitions - board ranks & files
	// ---------------------------------------

	// - Both ranks and files can be perceived as 1-dimensional ingrediants of a chessboard
	// - IMPLEMENTATION: We start counting from 0 for compatibility with other type definitions and functions

	// Rank type definition
	enum Rank : uint32_t {
		RANK_1 = 0, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8,

		NULL_RANK = 8,
		RANK_RANGE = 8
	};

	// Flips the rank across the center symmetry line (between 4th and 5th rank)
	// - RANK_1 becomes RANK_8, RANK_2 becomes RANK_7 etc.
	constexpr inline Rank flip(Rank r)
	{
		return Rank(r ^ 7);
	}

	inline std::ostream& operator<<(std::ostream& os, Rank rank)
	{
		os << int(rank + 1);

		return os;
	}

	// File type definition
	enum File : uint32_t {
		FILE_A = 0, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,

		NULL_FILE = 8,
		FILE_RANGE = 8
	};

	// Flips the file across the center symmetry line (between D and E files)
	// - FILE_A becomes FILE_H, FILE_B becomes FILE_G etc.
	constexpr inline File flip(File f)
	{
		return File(f ^ 7);
	}

	inline std::ostream& operator<<(std::ostream& os, File file)
	{
		os << char('a' + file);

		return os;
	}


	// ---------------------------------
	// Types definitions - board squares
	// ---------------------------------

	// Square type definition
	// - Squares can be considered as 2-dimensional objects, since each square is determined by both rank and file
	// - IMPLEMENTATION: Again, start counting from 0 for compatibility with LSB and MSB intrinsics
	enum Square : uint32_t {
		SQ_A1 = 0, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
		SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
		SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
		SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
		SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
		SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
		SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
		SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,

		NULL_SQUARE = 64,
		SQUARE_RANGE = 64,
	};

	// Squares - rank & file operations
	// ________________________________

	constexpr inline File file_of(Square s)
	{
		return File(s & 0x7);
	}

	constexpr inline Rank rank_of(Square s)
	{
		return Rank(s >> 3);
	}

	constexpr inline Square make_square(Rank rank, File file)
	{
		return Square((rank << 3) | file);
	}

	// Flips either rank or file of given square creating a new square
	// - The flip direction can by specified by giving either Rank or File type as the template argument
	// - Returns a NULL_SQUARE if invalid direction specified
	template <typename Dir>
	constexpr inline Square flip(Square sq)
	{
		// std::is_same_v provides a compile-time type checks
		return std::is_same_v<Dir, Rank> ? Square(sq ^ 56) :
				std::is_same_v<Dir, File> ? Square(sq ^ 7) :
											NULL_SQUARE;
	}

	// Squares - bitwise operations
	// ____________________________

	// Explicit coversion to a bitboard
	constexpr inline Bitboard as_bitboard(Square s)
	{
		return 1ULL << s;
	}

	constexpr inline Bitboard operator~(Square sq)
	{
		return ~as_bitboard(sq);
	}

	constexpr inline Bitboard operator&(Bitboard bb, Square sq)
	{
		return bb & as_bitboard(sq);
	}

	inline Bitboard operator&=(Bitboard& bb, Square sq)
	{
		bb &= as_bitboard(sq);
		return bb;
	}

	constexpr inline Bitboard operator|(Bitboard bb, Square sq)
	{
		return bb | as_bitboard(sq);
	}

	inline Bitboard operator|=(Bitboard& bb, Square sq)
	{
		bb |= as_bitboard(sq);
		return bb;
	}

	constexpr inline Bitboard operator^(Bitboard bb, Square sq)
	{
		return bb ^ as_bitboard(sq);
	}

	inline Bitboard operator^=(Bitboard& bb, Square sq)
	{
		bb ^= as_bitboard(sq);
		return bb;
	}

	// Squares - other
	// _______________

	inline std::ostream& operator<<(std::ostream& os, Square sq)
	{
		if (sq != NULL_SQUARE)	os << file_of(sq) << rank_of(sq);
		else 					os << "null";

		return os;
	}


	// --------------------------------
	// Type definitions - square colors
	// --------------------------------

	// Square color type definition
	enum SquareColor : uint32_t {
		DARK_SQUARE = 0, LIGHT_SQUARE,

		SQUARE_COLOR_RANGE = 2
	};

	constexpr inline SquareColor color_of(Square sq)
	{
		return SquareColor((uint32_t(sq) ^ uint32_t(rank_of(sq))) & 0x1);
	}

	constexpr inline SquareColor operator~(SquareColor color)
	{
		return SquareColor(color ^ 0x1);
	}


	// -----------------------------
	// Type definitions - directions
	// -----------------------------

	// Direction is represented as classical geographic direction, which covers directions of all file, rank, and diagonal moves
	// - IMPLEMENTATION: Each direction is encoded with appropriate shift that direction implies to given square
	enum Direction : int32_t {
		NORTH = 8,
		SOUTH = -8,
		EAST = 1,
		WEST = -1,

		NORTH_EAST = 9,
		NORTH_WEST = 7,
		SOUTH_EAST = -7,
		SOUTH_WEST = -9,

		NULL_DIRECTION = 0
	};

	// Calculates opposite direction
	constexpr inline Direction operator~(Direction dir)
	{
		return Direction(-dir);
	}

	// Returns the common part of two directions
	// - For example, NORTH_EAST & NORTH = EAST, NORTH_EAST & WEST = NULL_DIRECTION
	constexpr inline Direction operator&(Direction dir, Direction subdir)
	{
		return subdir == NORTH ? Direction(NORTH * (dir > 1)) :
				subdir == SOUTH ? Direction(SOUTH * (dir < -1)) :
				subdir == EAST  ? Direction(EAST * ((dir & 0x3) == 0x1)) :
				subdir == WEST  ? Direction(WEST * ((dir & 0x3) == 0x3)) : 
									Direction(subdir * (subdir == dir));
	}

	// Returns shifted square, or a NULL_SQUARE if we go out of board bounds (either horizontaly or verticaly)
	constexpr inline Square operator+(Square sq, Direction dir)
	{
		// IMPLEMENTATION: overflows on signed integers in C++ are well defined and can be used in our favour
		uint32_t target = uint32_t(sq) + int32_t(dir);

		return ((sq ^ target) & 0xFFFFFFc7) < 7 ? Square(target) : NULL_SQUARE;
	}

	// Returns shifted square, or a NULL_SQUARE if we go out of board bounds (either horizontaly or verticaly)
	constexpr inline Square operator-(Square sq, Direction dir)
	{
		// IMPLEMENTATION: overflows on signed integers in C++ are well defined and can be used in our favour
		uint32_t target = uint32_t(sq) - int32_t(dir);

		return ((sq ^ target) & 0xFFFFFFc7) < 7 ? Square(target) : NULL_SQUARE;
	}


	// --------------------------------
	// Type definitions - playing sides
	// --------------------------------

	enum Color : uint32_t {
		WHITE = 0, BLACK,

		COLOR_RANGE = 2
	};

	// Calculates the opposite side
	constexpr inline Color operator~(Color side)
	{
		return Color(side ^ 0x1);
	}

	inline std::ostream& operator<<(std::ostream& os, Color color)
	{
		os << (color == WHITE ? "W" : "B");

		return os;
	}


	// -------------------------
	// Type definitions - pieces
	// -------------------------

	// - IMPLEMENTATION: Here we start counting from 1 instead

	// PieceType represents general class of a piece (eg. pawn, rook, etc.), regardless of piece color
	enum PieceType : uint32_t {
		PAWN = 1, 
		KNIGHT,
		BISHOP, 
		ROOK, 
		QUEEN, 
		KING,

		ALL_PIECES,

		NULL_PIECE_TYPE = 0,
		PIECE_TYPE_RANGE = 8
	};

	// Piece represents more concrete piece definition, with both type and color included
	enum Piece : uint32_t {
		W_PAWN = 1, 
		W_KNIGHT, 
		W_BISHOP, 
		W_ROOK, 
		W_QUEEN, 
		W_KING,

		BLACK_PIECE = 8,
		B_PAWN = W_PAWN | BLACK_PIECE, 
		B_KNIGHT = W_KNIGHT | BLACK_PIECE, 
		B_BISHOP = W_BISHOP | BLACK_PIECE, 
		B_ROOK = W_ROOK | BLACK_PIECE, 
		B_QUEEN = W_QUEEN | BLACK_PIECE,
		B_KING = W_KING | BLACK_PIECE,

		NO_PIECE = 0,
		PIECE_RANGE = 16,
	};

	constexpr inline Color color_of(Piece piece)
	{
		return Color(bool(piece & BLACK_PIECE));
	}

	constexpr inline PieceType type_of(Piece piece)
	{
		return PieceType(piece & 0x7);
	}

	constexpr inline Piece make_piece(Color color, PieceType type)
	{
		return Piece(type | (color << 3));
	}

	inline std::ostream& operator<<(std::ostream& os, Piece piece)
	{
		PieceType ptype = type_of(piece);
		char symbol = ptype == PAWN ? 'P' : ptype == KNIGHT ? 'N' : ptype == BISHOP ? 'B' : 
					ptype == ROOK ? 'R' : ptype == QUEEN ? 'Q' : ptype == KING ? 'K' : ' ';

		os << char((color_of(piece) == WHITE ? symbol : tolower(symbol)));
		
		return os;
	}


	// ---------------------------
	// Type definitions - castling
	// ---------------------------

	// Castle type
	// - Enumerate from 1 to simplify castling rights formula
	enum Castle {
		KINGSIDE_CASTLE = 1,
		QUEENSIDE_CASTLE,

		NO_CASTLE = 0
	};

	// Castling rights can be represented as 4-bit integer
	// - Every bit represents one of two castling options for one of the sides
	// - Least significant bit represents kingside castle for white, then queenside castle for white, and then black rights in same order
	using CastlingRights = std::uint32_t;

	// Map castle type to castling right 4-bit integer
	constexpr inline CastlingRights make_castle_right(Color side, Castle castle)
	{
		return CastlingRights(castle << side * 2);
	}

	// Castling rights - common constants
	// __________________________________

	constexpr CastlingRights WHITE_OO = make_castle_right(WHITE, KINGSIDE_CASTLE);
	constexpr CastlingRights WHITE_OOO = make_castle_right(WHITE, QUEENSIDE_CASTLE);
	constexpr CastlingRights BLACK_OO = make_castle_right(BLACK, KINGSIDE_CASTLE);
	constexpr CastlingRights BLACK_OOO = make_castle_right(BLACK, QUEENSIDE_CASTLE);

	constexpr CastlingRights WHITE_BOTH = WHITE_OO | WHITE_OOO;
	constexpr CastlingRights BLACK_BOTH = BLACK_OO | BLACK_OOO;
	constexpr CastlingRights ALL_RIGHTS = WHITE_BOTH | BLACK_BOTH;
	constexpr CastlingRights NO_RIGHTS = 0;

} // namespace Chess