#pragma once

#include <cassert>
#include <mutex>

#include "info.h"

#ifndef GC_PAGE_SIZE
#define GC_PAGE_SIZE 8192
#endif

namespace gc
{
	class page
	{
	public:
		using size_type = std::uint32_t;
		using value_type = info;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = pointer;
		using const_iterator = const_pointer;

		page()
		{
			for (size_type i{}; i < capacity(); ++i)
				_freed_data[i] = i;
			_freed_data_size = capacity();
		}

		page(const page &) noexcept = delete;
		page(page &&) noexcept = delete;
		~page()
		{
			for (info &item : _data)
				item.destroy();
		}

		page &operator=(const page &) noexcept = delete;
		page &operator=(page &&) noexcept = delete;

		// Capacity
		static constexpr size_type capacity() noexcept { return GC_PAGE_SIZE; }
		size_type unused_space() const noexcept { return _freed_data_size; }
		size_type used_size() const noexcept { return capacity() - unused_space(); }
		bool empty() const noexcept { return capacity() == unused_space(); }
		bool full() const noexcept { return unused_space() == 0; }

		// Element Access
		reference operator[](const size_type pos) noexcept { return _data[pos]; }
		const_reference operator[](const size_type pos) const noexcept { return _data[pos]; }

		// Iterators
		iterator begin() noexcept { return _data; }
		const_iterator begin() const noexcept { return _data; }

		iterator end() noexcept { return _data + capacity(); }
		const_iterator end() const noexcept { return _data + capacity(); }

		// Modifiers
		void clear() noexcept
		{
			for (info &item : _data)
				item.destroy();
			_freed_data_size = 0;
		}

		void erase(const iterator it)
		{
			erase(static_cast<size_type>(it - begin()));
		}

		void erase(const size_type index)
		{
			assert(index < capacity());
			_data[index].destroy();
			_freed_data[_freed_data_size++] = index;
			assert(_freed_data_size <= capacity());
		}

		template <class T, bool Array>
		reference add_element(T *arg)
		{
			assert(_freed_data_size > 0);
			const size_type index = _freed_data[--_freed_data_size];

			assert(index < capacity());
			info &info = _data[index];
			info.construct<T, Array>(arg);
			return info;
		}

		// Synchronization
		std::mutex &get_mutex() const noexcept { return lock; }

	private:
		mutable std::mutex lock;
		info _data[GC_PAGE_SIZE]{};
		size_type _freed_data[GC_PAGE_SIZE]{};
		size_type _freed_data_size{};
	};
}

#undef GC_PAGE_SIZE
