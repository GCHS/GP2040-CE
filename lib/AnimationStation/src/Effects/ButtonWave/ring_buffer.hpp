#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <array>

namespace glowbox {
	template<typename Contained, int Capacity>
	class ring_buffer {
	private:
		std::array<Contained, Capacity> items;
		int _begin = 0;
		int _end = 0;
		int _size = 0;
		constexpr Contained& grow_back();
		constexpr Contained& grow_front();
		constexpr int grow_back(const int amount);	//returns beginning of new area
		constexpr int grow_front(const int amount);//returns beginning of new area
	public:
		constexpr ring_buffer() {};
		struct ring_iter {
		private:
			ring_buffer<Contained, Capacity>& buffer;
			int index;
		public:
			constexpr ring_iter(const ring_iter&);
			constexpr ring_iter(ring_buffer<Contained, Capacity>& buffer, const int index);
			constexpr ring_iter& operator++();
			constexpr ring_iter operator++(int);
			constexpr ring_iter& operator--();
			constexpr ring_iter operator--(int);
			constexpr bool operator==(const ring_iter& r) const;
			constexpr bool operator!=(const ring_iter& r) const;
			constexpr Contained& operator*() const;
			constexpr int operator-(const ring_iter& subtrahend) const;
			friend class ring_buffer;
		};
		template<typename RightContained, int RightCapacity>
		constexpr bool operator==(const ring_buffer<RightContained, RightCapacity>& r) {//identity
			return this == &r;
		}
		template<typename RightContained, int RightCapacity>
		constexpr bool operator!=(const ring_buffer<RightContained, RightCapacity>& r) {//identity
			return this != &r;
		}

		constexpr Contained& emplace_back(const Contained& c);
		constexpr Contained& emplace_back(Contained&& c);

		constexpr Contained& emplace_front(const Contained& c);
		constexpr Contained& emplace_front(Contained&& c);

		constexpr void clear();
		constexpr ring_iter begin();
		constexpr ring_iter end();
		constexpr int size();
		constexpr int capacity() { return Capacity; }
		constexpr Contained& operator[] (const int i);
	};

	template<typename Contained, int Capacity>
	constexpr ring_buffer<Contained, Capacity>::ring_iter::ring_iter(const ring_iter& ri) :
		buffer(ri.buffer), index(ri.index) {}

	template<typename Contained, int Capacity>
	constexpr ring_buffer<Contained, Capacity>::ring_iter::ring_iter(ring_buffer<Contained, Capacity>& buffer, const int index) :
		buffer(buffer), index(index) {}

	template<typename Contained, int Capacity>
	constexpr typename ring_buffer<Contained, Capacity>::ring_iter& ring_buffer<Contained, Capacity>::ring_iter::operator++() {
		++index;
		return *this;
	}

	template<typename Contained, int Capacity>
	constexpr typename ring_buffer<Contained, Capacity>::ring_iter ring_buffer<Contained, Capacity>::ring_iter::operator++(int) {
		auto ret = *this;
		++* this;
		return ret;
	}

	template<typename Contained, int Capacity>
	constexpr typename ring_buffer<Contained, Capacity>::ring_iter& ring_buffer<Contained, Capacity>::ring_iter::operator--() {
		--index;
		return *this;
	}

	template<typename Contained, int Capacity>
	constexpr typename ring_buffer<Contained, Capacity>::ring_iter ring_buffer<Contained, Capacity>::ring_iter::operator--(int) {
		auto ret = *this;
		--* this;
		return ret;
	}

	template<typename Contained, int Capacity>
	constexpr bool ring_buffer<Contained, Capacity>::ring_iter::operator==(const ring_iter& r) const {//value equality
		return buffer == r.buffer && index == r.index;
	}

	template<typename Contained, int Capacity>
	constexpr bool ring_buffer<Contained, Capacity>::ring_iter::operator!=(const ring_iter& r) const {//value inequality
		return !(*this == r);
	}

	template<typename Contained, int Capacity>
	constexpr Contained& ring_buffer<Contained, Capacity>::ring_iter::operator*() const {
		return buffer[index];
	}

	template<typename Contained, int Capacity>
	constexpr int ring_buffer<Contained, Capacity>::ring_iter::operator-(const ring_iter& subtrahend) const {
		return index - subtrahend.index + (buffer._end < buffer._begin ? buffer._size : 0);
	}

	template<typename Contained, int Capacity>
	constexpr Contained& ring_buffer<Contained, Capacity>::grow_back() {
		return (*this)[grow_back(1) - _begin];
	}

	template<typename Contained, int Capacity>
	constexpr Contained& ring_buffer<Contained, Capacity>::grow_front() {
		return (*this)[grow_front(1) - _begin];
	}

	template<typename Contained, int Capacity>
	constexpr int ring_buffer<Contained, Capacity>::grow_back(const int amount) {
		int old_end = _end;
		_end += amount;
		if(_size < Capacity) {
			_size += amount;
			return old_end;
		} else {//overwrite
			int old_begin = _begin;
			_begin += amount;
			return old_begin;
		}
	}

	template<typename Contained, int Capacity>
	constexpr int ring_buffer<Contained, Capacity>::grow_front(const int amount) {
		int old_begin = _begin;
		_begin -= amount;
		if(_size < Capacity) {
			_size += amount;
			return old_begin;
		} else {//overwrite
			int old_end = _end;
			_end -= amount;
			return old_end;
		}
	}

	template<typename Contained, int Capacity>
	constexpr Contained& ring_buffer<Contained, Capacity>::emplace_back(const Contained& c) {
		return grow_back() = c;
	}

	template<typename Contained, int Capacity>
	constexpr Contained& ring_buffer<Contained, Capacity>::emplace_back(Contained&& c) {
		return grow_back() = c;
	}

	template<typename Contained, int Capacity>
	constexpr Contained& ring_buffer<Contained, Capacity>::emplace_front(const Contained& c) {
		return grow_front() = c;
	}

	template<typename Contained, int Capacity>
	constexpr Contained& ring_buffer<Contained, Capacity>::emplace_front(Contained&& c) {
		return grow_front() = c;
	}

	template<typename Contained, int Capacity>
	constexpr void ring_buffer<Contained, Capacity>::clear() {
		_begin = _end = _size = 0;
	}

	template<typename Contained, int Capacity>
	constexpr typename ring_buffer<Contained, Capacity>::ring_iter ring_buffer<Contained, Capacity>::begin() {
		return ring_iter(*this, 0);
	}

	template<typename Contained, int Capacity>
	constexpr typename ring_buffer<Contained, Capacity>::ring_iter ring_buffer<Contained, Capacity>::end() {
		return ring_iter(*this, _size);
	}

	template<typename Contained, int Capacity>
	constexpr int ring_buffer<Contained, Capacity>::size() {
		return _size;
	}
	template<typename Contained, int Capacity>
	inline constexpr Contained& ring_buffer<Contained, Capacity>::operator[](const int i) {
		if(i >= 0) {
			return items[(i + _begin) % _size];
		} else {
			int correction_coeff = 1 - (i / _size);
			return items[(i + _begin + _size * correction_coeff) % _size];
		}
	}
}
#endif