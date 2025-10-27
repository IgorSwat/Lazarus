#pragma once

#include <cinttypes>
#include <climits>
#include <numeric>
#include <random>


namespace utilities::random {

    // ---------------------------------
	// Random number generator interface
	// ---------------------------------

    // Set of type-dependable interfaces which should be implemented by all below generators
    template <typename NumType>
    class Generator
    {
    public:
        Generator(NumType seed) : m_seed(seed) {}

        virtual NumType random() = 0;

    protected:
        NumType m_seed;
    };


    // ----------------------------------
	// Random number generators - general
	// ----------------------------------

    // A standard, high-quality random integer generator
    // - Utilizes the Mersenne-Twister algorithm to generate random numbers in range [NumType::min, NumType::max]
    template <typename IntType>
    class StandardGenerator : public Generator<IntType>
    {
    public:
        StandardGenerator(IntType seed) : Generator<IntType>(seed), m_generator(seed), 
                                          m_distribution(std::numeric_limits<IntType>::min(), std::numeric_limits<IntType>::max()) {}

        IntType random() override { return m_distribution(m_generator); }

    private:
        std::mt19937_64 m_generator;
	    std::uniform_int_distribution<IntType> m_distribution;
    };


    // ---------------------------------------------------------
	// Random number generators - specialized - magics generator
	// ---------------------------------------------------------

    // Magics generator is a sparse generator, that generates sparse uin64_t numbers for the purpose of magics initialization
    // - By sparse number we mean a number with relatively low amount of 1 bits
    // - To generate sparse numbers, we simply utilize multiple bitwise AND operations on random numbers
    class MagicsGenerator : public Generator<uint64_t>
    {
    public:
        MagicsGenerator(uint64_t seed) : Generator<uint64_t>(seed) {}

        uint64_t random()
        {
            return _random() & _random() & _random();
        }

    private:
        // I don't know where did I get it from, but it is a borrowed external formula
        uint64_t _random()
        {
            static constexpr uint64_t MULT = 0x9FB21C651E98DF25;

            m_seed ^= ((m_seed << 49) | (m_seed >> 15)) ^ ((m_seed << 24) | (m_seed >> 40));
		    m_seed *= MULT;
		    m_seed ^= m_seed >> 35;
		    m_seed *= MULT;
		    m_seed ^= m_seed >> 28;

		    return m_seed;
        }
    };

}