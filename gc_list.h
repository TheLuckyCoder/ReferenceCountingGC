#pragma once

#include <vector>

namespace gc
{
	template <class T>
	class gc_list
	{
		static constexpr std::size_t LIST_SIZE = 4096;
		
	public:
		using vector_type = std::vector<T>;
		using size_type = typename vector_type::size_type;
		using value_type = typename vector_type::value_type;
		using pointer = typename vector_type::pointer;
		using const_pointer = typename vector_type::const_pointer;
		using reference = typename vector_type::reference;
		using const_reference = typename vector_type::const_reference;
		using iterator = typename vector_type::iterator;
		using const_iterator = typename vector_type::const_iterator;
		using reverse_iterator = typename vector_type::reverse_iterator;
		using const_reverse_iterator = typename vector_type::const_reverse_iterator;

		gc_list()
		{
			_data.reserve(LIST_SIZE);
			_free_indexes.reserve(LIST_SIZE);
		}

		gc_list(const gc_list &) noexcept = default;
		gc_list(gc_list &&) noexcept = default;
		~gc_list() = default;

		gc_list &operator=(const gc_list &) noexcept = default;
		gc_list &operator=(gc_list &&) noexcept = default;

		// Element Access
		reference at(const size_type pos) noexcept(false) { return _data.at(pos); }
		const_reference at(const size_type pos) const noexcept(false) { return _data.at(pos); }

		reference operator[](const size_type pos) noexcept(false) { return _data[pos]; }
		const_reference operator[](const size_type pos) const noexcept(false) { return _data[pos]; }

		reference front() noexcept { return _data.front(); }
		const_reference front() const noexcept { return _data.front(); }

		reference back() noexcept { return _data.back(); }
		const_reference back() const noexcept { return _data.back(); }

		reference data() noexcept { return _data.data(); }
		const_reference data() const noexcept { return _data.data(); }

		// Iterators
		auto begin() noexcept { return _data.begin(); }
		auto begin() const noexcept { return _data.begin(); }

		auto end() noexcept { return _data.end(); }
		auto end() const noexcept { return _data.end(); }

		auto rbegin() noexcept { return _data.rbegin(); }
		auto rbegin() const noexcept { return _data.rbegin(); }

		auto rend() noexcept { return _data.rend(); }
		auto rend() const noexcept { return _data.rend(); }

		// Capacity
		bool empty() const noexcept { return _data.empty(); }
		auto size() const noexcept { return _data.size(); }
		auto used_size() const noexcept { return _data.size() - _free_indexes.size(); }
		auto capacity() noexcept { return _data.capacity(); }

		// Modifiers
		void clear() noexcept
		{
			_data.clear();
			_free_indexes.clear();
		}

		/**
		 * @returns true if the last element was popped
		 */
		bool erase(const_iterator it)
		{
			if (it == end() - 1)
			{
				pop_back();
				return true;
			}

			(*it).~T();
			_free_indexes.push_back(it);
			return false;
		}

		template <class... Args>
		reference emplace_back(Args &&... args)
		{
			return _data.emplace_back(std::forward<Args>(args)...);
		}

		template <class... Args>
		reference emplace(const_iterator it, Args &&... args)
		{
			return *_data.emplace(it, std::forward<Args>(args)...);
		}

		template <class... Args>
		reference add_element(Args &&... args)
		{
			if (_free_indexes.empty())
				return emplace_back(std::forward<Args>(args)...);

			const auto emptyIterator = _free_indexes.back();
			_free_indexes.pop_back();

			return emplace(emptyIterator, std::forward<Args>(args)...);
		}

		void pop_back() noexcept { _data.pop_back(); }

	private:
		std::vector<T> _data;
		std::vector<const_iterator> _free_indexes;
	};
}
