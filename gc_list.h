#pragma once

#include <mutex>
#include <vector>

#include "info.h"

#ifndef GC_PAGE_SIZE
#define GC_PAGE_SIZE 8192
#endif

namespace gc
{
	class gc_list
	{
	public:
		using size_type = std::size_t;
		using value_type = info;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = pointer;
		using const_iterator = const_pointer;

		gc_list()
		{
			_free_indexes.reserve(GC_PAGE_SIZE);
			for (size_type i = GC_PAGE_SIZE - 1; i > 0; --i)
				_free_indexes.push_back(i);
			_free_indexes.push_back(0);
		}

		gc_list(const gc_list &) noexcept = delete;
		gc_list(gc_list &&) noexcept = delete;
		~gc_list() = default;

		gc_list &operator=(const gc_list &) noexcept = delete;
		gc_list &operator=(gc_list &&) noexcept = delete;

		// Element Access
		reference operator[](const size_type pos) noexcept(false) { return _data[pos]; }
		const_reference operator[](const size_type pos) const noexcept(false) { return _data[pos]; }

		// Capacity
		size_type capacity() const noexcept { return GC_PAGE_SIZE; }
		bool empty() const noexcept { return capacity() == _free_indexes.size(); }
		bool full() const noexcept { return _free_indexes.empty(); }
		size_type used_size() const noexcept { return capacity() - _free_indexes.size(); }
		size_type unused_space() const noexcept { return _free_indexes.size(); }

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
			_free_indexes.clear();
		}

		void erase(const iterator it)
		{
			(*it).destroy();
			_free_indexes.push_back(it - begin());
		}

		void erase(const size_type index)
		{
			_data[index].destroy();
			_free_indexes.push_back(index);
		}

		template <class T>
		reference emplace(const size_type index, const T *arg)
		{
			info &info = _data[index];
			info.construct(arg);
			return info;
		}

		template <class T>
		reference add_element(const T *arg)
		{
			const size_type emptyIndex = _free_indexes.back();
			_free_indexes.pop_back();

			return emplace(emptyIndex, arg);
		}

		// Synchronization
		std::mutex &get_lock() const noexcept { return lock; }

	private:
		mutable std::mutex lock;
		info _data[GC_PAGE_SIZE]{};
		std::vector<size_type> _free_indexes;
	};
}

#undef GC_PAGE_SIZE
