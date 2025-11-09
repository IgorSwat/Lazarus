#include "move.h"
#include <bitset>
#include <iomanip>


namespace chess {

    // -----------------------
    // Move methods - printing
    // -----------------------

    std::ostream& operator<<(std::ostream& os, const Move& move)
    {
        if (move == moves::null)
            os << "null_move";
        else {
            os << "Move: " << move.from() << " -> " << move.to();
            os << " (" << std::bitset<4>(move.flags()) << ")";      // Print flags in binary (4 bits)
        }

        return os;
    }
}