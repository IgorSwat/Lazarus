#include "test.h"
#include "../src/engine/bitboards/bitboards.h"


namespace testing {

    using namespace chess;

    // NOTE: We do not test basic algorithms like popcount or lsb, since their implementation
    //       is relatively simple and very rarely changes

    // Test shifts
    REGISTER_TEST(bitboards_shift_test)
    {
        const Bitboard diagonal = 0x8040201008040201;

        // Test correctness of static shift
        ASSERT_EQUALS(0x4020100804020100, bitboards::shift<NORTH>(diagonal));
        ASSERT_EQUALS(0x8040201008040200, bitboards::shift<NORTH_EAST>(diagonal));
        ASSERT_EQUALS(0x0000804020100804, bitboards::shift<SOUTH_EAST>(diagonal));

        return true;
    }

    // Test fills
    REGISTER_TEST(bitboards_fill_test)
    {
        const Bitboard b1 = as_bitboard(SQ_E4);
        const Bitboard b2 = as_bitboard(SQ_E4) | as_bitboard(SQ_E5);

        // Vertical fills
        ASSERT_EQUALS(0x1010101010000000, bitboards::fill<NORTH>(b1));
        ASSERT_EQUALS(0x1010101010000000, bitboards::fill<NORTH>(b2));
        ASSERT_EQUALS(0x0000000010101010, bitboards::fill<SOUTH>(b1));
        ASSERT_EQUALS(0x0000001010101010, bitboards::fill<SOUTH>(b2));

        // Other directions
        ASSERT_EQUALS(0x00000010180c0603, bitboards::fill<SOUTH_WEST>(b2));
        ASSERT_EQUALS(0x000000f0f0000000, bitboards::fill<EAST>(b2));

        return true;
    }

}