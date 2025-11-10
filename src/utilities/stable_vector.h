#pragma once

#include "mem.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>


namespace utilities {

    // -------------
    // Static vector
    // -------------

    template <typename T, unsigned max_size>
    class __static_vector
    {
    public:
        // Constructor - only default construction enabled
        __static_vector() = default;

        // Modifiers - adding / removing elements
        // - Note that we do not check any bounds in release mode, to make it as optimized as possible
        void push_back() { assert(m_end - m_data < max_size); m_end++; }
        void push_back(const T& element) { assert(m_end - m_data < max_size); *(m_end++) = element; }
        void pop_back(const T& element) { assert(m_end > m_data); m_end--; }
        void resize(unsigned new_size) { assert(new_size <= max_size); m_end = m_data + new_size; }
        void resize(T* end) { assert(m_data <= end && end < m_data + max_size); m_end = end; }
        void clear() { m_end = m_data; }

        // Getters - indexing
        T& operator[](unsigned i) { assert(m_data + i < m_end); return m_data[i]; }
        const T& operator[](unsigned i) const { assert(m_data + i < m_end); return m_data[i]; }

        // Getters - other element access methods
        T& front() { assert(!empty()); return m_data[0]; }
        const T& front() const { assert(!empty()); return m_data[0]; }
        T& back() { assert(!empty()); return *(m_end - 1); }
        const T& back() const { assert(!empty()); return *(m_end - 1); }

        // Getters - size
        unsigned size() const { return m_end - m_data; }
        bool empty() const { return m_end == m_data; }
        bool full() const { return m_end == m_data + max_size; }

        // Getters - iterators
        T* begin() { return m_data; }
        const T* begin() const { return m_data; }
        T* end() { return m_end; }
        const T* end() const { return m_end; }

        // Printing
        friend std::ostream& operator<<(std::ostream& os, const __static_vector<T, max_size>& vec)
        {
            for (const T& element : vec)
                os << element << "\n";
        
            return os;
        }

    private:
        // Data container - a static array
        T m_data[max_size];

        // Range pointer - determines the logical end of array
        T* m_end = m_data;
    };


    // --------------
    // Dynamic vector
    // --------------

    // In comparision to static vector, this one introduces expanding & memory realocations when it runs out of space
    template <typename T, unsigned init_size>
    class __dynamic_vector
    {
    public:
        __dynamic_vector()
            : m_data(std::make_unique<T[]>(init_size)),
            m_capacity(init_size),
            m_size(0) {}

        // Modifiers - adding / removing elements
        void push_back() { if (m_size == m_capacity) reallocate(); m_size++; }
        void push_back(const T& element) { if (m_size == m_capacity) reallocate(); m_data[m_size++] = element; }
        void pop_back() { assert(m_size > 0); m_size--; }
        void resize(unsigned new_size) { while (new_size > m_capacity) reallocate(); m_size = new_size; }
        void clear() { m_size = 0; }

        // Getters - indexing
        T& operator[](unsigned i) { assert(i < m_size); return m_data[i]; }
        const T& operator[](unsigned i) const { assert(i < m_size); return m_data[i]; }

        // Getters - other element access methods
        T& front() { assert(!empty()); return m_data[0]; }
        const T& front() const { assert(!empty()); return m_data[0]; }
        T& back() { assert(!empty()); return m_data[m_size - 1]; }
        const T& back() const { assert(!empty()); return m_data[m_size - 1]; }

        // Getters - size
        unsigned size() const { return m_size; }
        bool empty() const { return m_size == 0; }
        bool full() const { return false; }             // For consistency with static vector

        // Getters - iterators
        T* begin() { return m_data.get(); }
        const T* begin() const { return m_data.get(); }
        T* end() { return m_data.get() + m_size; }
        const T* end() const { return m_data.get() + m_size; }

        // Printing
        friend std::ostream& operator<<(std::ostream& os, const __dynamic_vector<T, init_size>& vec)
        {
            for (const T& element : vec)
                os << element << "\n";
        
            return os;
        }

    private:
        // Helper function - dynamic memory realocation
        void reallocate()
        {
            unsigned new_capacity = m_capacity * 2;
            auto new_data = std::make_unique<T[]>(new_capacity);

            // Copy all constructed slots to preserve "stable" contents beyond m_size
            for (unsigned i = 0; i < m_capacity; ++i)
                new_data[i] = m_data[i];

            m_data = std::move(new_data);
            m_capacity = new_capacity;
        }

        // Raw storage
        std::unique_ptr<T[]> m_data = nullptr;

        // Range counters
        // - We use standard ints since they are a little bit quicker than 64-bit integers
        unsigned m_capacity = 0;                 // Count of constructed slots
        unsigned m_size = 0;                     // Logical size
    };


    // ---------------------------------
    // Stable vector - public definition
    // ---------------------------------

    template <memory::Storage storage, typename T, unsigned size>
    using StableVector = std::conditional<
        storage == memory::Storage::STATIC, 
        __static_vector<T, size>, 
        __dynamic_vector<T, size>
    >::type;

} // namespace utilities