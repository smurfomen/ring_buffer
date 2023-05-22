#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <array>
#include <stdexcept>
#ifdef OPTIONAL_INCLUDED
#include <optional>
#endif

#ifdef QT_CORE_LIB
#include <QByteArray>
#endif

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

	using index_type = std::size_t;
	using value_type = T;

	std::array<T, S> m_storage;
	index_type m_cwrite {0};
	index_type m_cread {0};
	const index_type m_mask = S - 1;

public:
	ring_buffer()
		: m_cwrite(0), m_cread(0)
	{ }

	const value_type* raw() const & {
		return &m_storage[0];
	}

	/*! \brief  Read element to\e value from the end of buffer and drop element, returns true if operation was successed. */
	bool read(value_type & value) {
		if(is_empty()) return false;

		value = m_storage[m_cread++ & m_mask];
		return true;
	}

#ifdef OPTIONAL_INCLUDED
	optional<value_type> read() {
		if(!is_empty())
			return m_storage[m_cread++ & m_mask];

		return nullopt;
	}
#endif

	/*! \brief  Append \e value to the end of buffer, returns true if operation was successed. */
	bool write(const value_type & value) {
		if(is_full()) return false;

		m_storage[m_cwrite++ & m_mask] = value;
		return true;
	}


	bool write(value_type && value) {
		if(is_full()) return false;

		m_storage[m_cwrite++ & m_mask] = std::move(value);
		return true;
	}

#ifdef QT_CORE_LIB
	bool write(QByteArray && a) {
		if(size() - count() < a.size())
			return false;

		for(auto && ch : a) {
			write(ch);
		}

		return true;
	}
#endif

	bool write(const void * const p, ssize_t len) {
		if(!len || (size() - count()) < len)
			return false;

		const value_type * ptr = (const value_type*)p;

		int i = 0;
		while(i < len) write(ptr[i++]);

		return true;
	}

	/*! \brief  Returns true if buffer is fulled. */
	bool is_full() const noexcept
	{ return ((index_type)(m_cwrite - m_cread) & (index_type)~(m_mask)) != 0; }

	/*! \brief  Returns true if buffer is empty. */
	bool is_empty() const noexcept
	{ return m_cwrite == m_cread; }

	/*! \brief  Returns count of elements into buffer. */
	index_type count() const noexcept
	{ return (m_cwrite - m_cread) & m_mask; }

	/*! \brief  Returns size capacity of buffer. */
	constexpr size_t size() const noexcept
	{ return S; }

	/*! \brief  Returns first element from buffer. Don't drop element. May throw OOE exception. */
	value_type& first() &
	{ return operator[](0); }

	/*! \brief  Returns last element from buffer. May throw OOE exception. */
	value_type& last() &
	{ return operator[](count() - 1); }

	/*! \brief  Returns first element from buffer. Don't drop element. May throw OOE exception. */
	const value_type& first() const &
	{ return operator[](0); }

	/*! \brief  Returns last element from buffer. May throw OOE exception. */
	const value_type& last() const &
	{ return operator[](count() - 1); }

	/*! \brief  Reset count elements. */
	void clear() noexcept
	{ m_cread = 0; m_cwrite = 0; }

	ring_buffer& operator<<(const value_type & v)
	{
		write(v);
		return *this;
	}

	ring_buffer& operator<<(value_type && v)
	{
		write(std::forward<value_type>(v));
		return *this;
	}

	value_type& operator[](index_type i) &
	{
		if (is_empty() || i > count())
			throw std::out_of_range("index located in invalid range for access ring buffer");

		return m_storage[(m_cread + i) & m_mask];
	}

	const value_type operator[] (index_type i) const &
	{
		if (is_empty() || i > count())
			throw std::out_of_range("index located in invalid range for access ring buffer");

		return m_storage[(m_cread + i) & m_mask];
	}
};

#endif // RINGBUFFER_H
