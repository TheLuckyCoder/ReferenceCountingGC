#pragma once

#include <array>
#include <memory>
#include <mutex>
#include <vector>

#include "destroyer.h"

#ifndef GC_PAGE_SIZE
#define GC_PAGE_SIZE 8192
#endif

namespace gc
{
	class page
	{
	public:
		using value_type = gc::destroyer;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = value_type*;
		using const_iterator = const value_type*;

		page()
		{
			emplace_front();
        }
		page(const page &) = delete;
		page(page &&) = delete;
		~page()
        {
			std::lock_guard lock{ _mutex };
			_data.clear();
        }

		page &operator=(const page &) = delete;
		page &operator=(page &&) = delete;

	private:
		static constexpr std::size_t CAPACITY = GC_PAGE_SIZE;

		void emplace_front()
		{
			//_data.emplace(std::make_unique<page_array>());
			_data.emplace(_data.begin(), std::make_unique<page_array>());
		}

	public:
		void add(gc::destroyer &&destroyer)
        {
            std::lock_guard lock{ _mutex };
			if (_last_empty_index == CAPACITY)
			{
				emplace_front();
				_last_empty_index = 0;
			}

            auto &arr = *_data.front();
            arr[_last_empty_index++] = std::move(destroyer);
        }

		void clear()
        {
		    while (true)
            {
		        std::lock_guard lock{ _mutex };
				if (_data.size() <= 1)
					break;
				_data.pop_back();
                //_data.pop();
            }
        }

	private:
		using page_array = std::array<destroyer, CAPACITY>;

	    mutable std::mutex _mutex{};
		std::vector<std::unique_ptr<page_array>> _data{};
		std::size_t _last_empty_index{};
	};
}

#undef GC_PAGE_SIZE
