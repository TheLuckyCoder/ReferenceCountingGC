#pragma once

#include <array>
#include <cassert>
#include <vector>
#include <memory>
#include <mutex>

#include "pointer.h"

#ifndef GC_PAGE_SIZE
#define GC_PAGE_SIZE 8192
#endif

namespace gc
{
	class page
	{
	public:
		using size_type = std::uint32_t;
		using value_type = gc::pointer;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = value_type*;
		using const_iterator = const value_type*;

		page()
        {
			emplace_front();
        }
		page(const page &) noexcept = delete;
		page(page &&) noexcept = delete;
		~page()
        {
			std::lock_guard lock{_mutex };
			if (!_data.empty())
				_data.clear();
        }

		page &operator=(const page &) noexcept = delete;
		page &operator=(page &&) noexcept = delete;

	private:
		static constexpr size_type CAPACITY = GC_PAGE_SIZE;

		void emplace_front()
		{
			_data.emplace(_data.begin(), std::make_unique<page_array>());
		}

	public:
		void add(gc::pointer &&pointer)
        {
            std::lock_guard lock{_mutex };
			if (_last_empty_index == CAPACITY)
			{
				emplace_front();
				_last_empty_index = 0;
			}

            auto &arr = _data.front();
            arr->at(_last_empty_index++) = std::move(pointer);
        }

		void clear()
        {
		    while (true)
            {
		        std::lock_guard lock{_mutex };
                const auto begin = _data.begin();
                const auto end = --_data.end();

                if (begin == end)
                    break;

                _data.erase(end);
            }
        }

	private:
		using page_array = std::array<pointer, CAPACITY>;

	    mutable std::mutex _mutex{};
		std::vector<std::unique_ptr<page_array>> _data{};
		size_type _last_empty_index{};
	};
}

#undef GC_PAGE_SIZE
