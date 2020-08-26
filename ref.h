#pragma once

#include <atomic>
#include <type_traits>
#include <initializer_list>

#include "gc.h"

namespace gc
{
	template<class T, class C = std::atomic_uint8_t>
	class ref
	{
	public:
		using counter = std::enable_if_t<std::is_same_v<C, std::atomic_uint8_t>
												  || std::is_same_v<C, std::atomic_uint16_t>
												  || std::is_same_v<C, std::atomic_uint32_t>
												  || std::is_same_v<C, std::atomic_uint64_t>>;

		explicit ref(T *ptr)
			: _ptr(ptr),
			  _ref_count(new counter{ 1 }),
			  _one_alloc(false)
		{
		}

		explicit ref(T &&arg)
			: _one_alloc(true)
		{
			init_one_allocation();
			new(_ptr) T(std::forward<T>(arg));
		}

		template<typename... Args>
		explicit ref(Args &&...args)
			: _one_alloc(true)
		{
			init_one_allocation();
			new(_ptr) T(std::forward<Args>(args)...);
		}

		ref(const ref &rhs) noexcept
			: _ptr(rhs._ptr), _ref_count(rhs._ref_count), _one_alloc(rhs._one_alloc)
		{
			inc_ref();
		}

		ref(ref &&rhs) noexcept
			: _ptr(rhs._ptr), _ref_count(rhs._ref_count), _one_alloc(rhs._one_alloc)
		{
			rhs._ptr = nullptr;
			rhs._ref_count = nullptr;
		}

		~ref() noexcept
		{
			if (_ref_count && dec_ref() == 0)
			{
				if constexpr (std::is_trivially_destructible_v<T>)
				{
					// Free the memory directly if it is trivially destructible
                    if (_one_alloc)
                        operator delete(reinterpret_cast<void *>(_ref_count), TOTAL_SIZE);
                    else
                    {
                        delete _ref_count;
                        delete _ptr;
                    }
				} else
				{
					// Delegate the destruction to the garbage collector
					void *memory_ptr = nullptr;
					destroyer::type type;

					if (_one_alloc)
					{
						// one allocation was made so we specify the pointer to the memory which will be deleted after the object has been destroyed
						type = destroyer::type::DESTROYER;
						memory_ptr = reinterpret_cast<void *>(_ref_count);
					} else
						type = destroyer::type::DELETER;

					delegate_destruction(destroyer::create_destroyer<T>(_ptr, type, memory_ptr, TOTAL_SIZE));
				}

				_ptr = nullptr;
				_ref_count = nullptr;
			}
		}

		ref &operator=(const ref &rhs) noexcept
		{
			if (this == &rhs)
				return *this;

			rhs.inc();

			_ptr = rhs._ptr;
			_ref_count = rhs._ref_count;
			_one_alloc = rhs._one_alloc;

			return *this;
		}

		ref &operator=(ref &&rhs) noexcept
		{
			if (this == &rhs)
				return *this;

			_ptr = rhs._ptr;
			_ref_count = rhs._ref_count;
			_one_alloc = rhs._one_alloc;

			rhs._ptr = nullptr;
			rhs._ref_count = nullptr;
			rhs._deleter = nullptr;

			return *this;
		}

		const T *get() const
		{
			return _ptr;
		}

		T *get()
		{
			return _ptr;
		}

		T &operator*() noexcept { return *_ptr; }

		const T &operator*() const noexcept { return *_ptr; }

		T *operator->() noexcept { return _ptr; }

		const T *operator->() const noexcept { return _ptr; }

	private:
		
		static constexpr std::size_t TOTAL_SIZE = sizeof(counter) + sizeof(T);

		void init_one_allocation()
		{
			// Allocate the entire memory needed for the reference counter + the object itself
			void *memory = operator new(TOTAL_SIZE);

			// Store the reference counter in the first part of the allocation
			_ref_count = static_cast<counter *>(memory);
			new(_ref_count) counter{ 1 };
			// Move the current pointer past the location of the reference counter
			// and use that memory for the object
			_ptr = reinterpret_cast<T *>(_ref_count + 1);
		}

		auto inc_ref()
		{
			return ++(*_ref_count);
		}

		auto dec_ref()
		{
			return --(*_ref_count);
		}

		T *_ptr = nullptr;
		counter *_ref_count = nullptr;
		const bool _one_alloc;
	};

	template<class T, class C = std::atomic_uint8_t>
	class ref_array
	{
		using counter = C;

	public:
		explicit ref_array(const std::size_t size)
			: _data(new T[size]), _ref_count(new counter{ 1 }), _size(size)
		{
		}

		ref_array(const std::initializer_list<T> init)
			: _data(operator new[](sizeof(T) * init.size())), _ref_count(new counter{ 1 }), _size(init.size())
		{
			std::copy(init.begin(), init.end(), _data);
		}

		ref_array(const ref_array &rhs) noexcept
			: _data(rhs._data), _ref_count(rhs._ref_count), _size(rhs._size)
		{
			inc_ref();
		}

		ref_array(ref_array &&rhs) noexcept
			: _data(rhs._data), _ref_count(rhs._ref_count), _size(rhs._size)
		{
			rhs._data = nullptr;
			rhs._ref_count = nullptr;
			rhs._deleter = nullptr;
			rhs._size = 0;
		}

		~ref_array() noexcept
		{
			if (_ref_count && dec_ref() == 0)
			{
				delegate_destruction(destroyer::create_destroyer<T>(_data, destroyer::type::ARRAY_DELETER, _ref_count, sizeof(T) * size()));

				_data = nullptr;
				_ref_count = nullptr;
			}
		}

		ref_array &operator=(const ref_array &rhs) noexcept
		{
			if (this == &rhs)
				return *this;

			_data = rhs._data;
			_ref_count = rhs._ref_count;
			_size = rhs._size;

			inc_ref();

			return *this;
		}

		ref_array &operator=(ref_array &&rhs) noexcept
		{
			if (this == &rhs)
				return *this;

			_data = rhs._data;
			_ref_count = rhs._ref_count;
			_size = rhs._size;

			rhs._data = nullptr;
			rhs._ref_count = nullptr;
			rhs._size = 0;

			return *this;
		}

		// Iterators
		T *begin() noexcept { return _data; }

		const T *begin() const noexcept { return _data; }

		T *end() noexcept { return _data + _size; }

		const T *end() const noexcept { return _data + _size; }

		auto size() const noexcept { return _size; }


		// Access Operators
		T &operator*() noexcept { return *_data; }

		const T &operator*() const noexcept { return *_data; }

		T &operator[](const std::size_t i) noexcept { return _data[i]; }

		const T &operator[](const std::size_t i) const noexcept { return _data[i]; }

		T *data() noexcept { return _data; }

	private:
		auto inc_ref()
		{
			return ++(*_ref_count);
		}

		auto dec_ref()
		{
			return --(*_ref_count);
		}

		T *_data = nullptr;
		counter *_ref_count = nullptr;
		std::size_t _size{};
	};
}

using gc::ref;
using gc::ref_array;
