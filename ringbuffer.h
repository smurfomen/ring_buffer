#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <array>
#include <stdexcept>
#include <limits>

namespace  {
	/* S must be one of the number series of powers of 2 */
    template<size_t S>
	struct ring_buffer_size_validator__ {
		constexpr static bool value = S > 0 && (S & (S - 1)) == 0;
    };
}

template<typename T, size_t S>
class ring_buffer
{
	static_assert(ring_buffer_size_validator__<S>::value, "S must be one of the number series of powers of 2");

    using index_t = unsigned long long;
    std::array<T, S> m_storage;
    size_t m_cwrite {0};
    size_t m_cread {0};
    const index_t m_mask = S - 1;

public:
    ring_buffer()
        : m_cwrite(0), m_cread(0)
    { }

    /*! \brief  Read element to\e value from the end of buffer and drop element, returns true if operation was successed. */
    bool read(T & value)
    {
        if(is_empty()) return false;

        value = m_storage[m_cread++ & m_mask];
        return true;
    }

    /*! \brief  Append \e value to the end of buffer, returns true if operation was successed. */
    bool write(T value)
    {
        if(is_full()) return false;

        m_storage[m_cwrite++ & m_mask] = value;
        return true;
    }

    /*! \brief  Returns true if buffer is fulled. */
    bool is_full()
    { return ((index_t)(m_cwrite - m_cread) & (index_t)~(m_mask)) != 0; }

    /*! \brief  Returns true if buffer is empty. */
    bool is_empty()
    { return m_cwrite == m_cread; }

    /*! \brief  Returns count of elements into buffer. */
    index_t count() const
    { return (m_cwrite - m_cread) & m_mask; }

    /*! \brief  Returns size capacity of buffer. */
    constexpr size_t size() const
    { return S; }

    /*! \brief  Returns first element from buffer. Don't drop element. May throw OOE exception. */
    T first() const
    { return operator[](0); }

    /*! \brief  Returns last element from buffer. May throw OOE exception. */
    T last() const
    { return operator[](count() - 1); }

    /*! \brief  Reset count elements. */
    void clear()
    { m_cread = 0; m_cwrite = 0; }

    ring_buffer & operator<<(T v)
    {
        write(v);
        return *this;
    }

    T & operator[](index_t i)
    {
        if (is_empty() || i > count())
            throw std::out_of_range("index located in invalid range for access ring buffer");

        return m_storage[(m_cread + i) & m_mask];
    }

    const T operator[] (index_t i) const
    {
        if (is_empty() || i > count())
            throw std::out_of_range("index located in invalid range for access ring buffer");

        return m_storage[(m_cread + i) & m_mask];
    }
};

#endif // RINGBUFFER_H
