#pragma once

#include <atomic>
#include <type_traits>
#include <initializer_list>

#include "destroyer.h"

namespace gc
{
	/**
	 * This is an internal API that should not be used
	 */
	void _delegate_destruction(gc::destroyer &&destroyer);

	template <class T, class C = std::atomic_uint8_t>
	class ref
	{
	public:
		// This does not compile on MSVC but works on GCC and Clang
		using counter = C; /*typename std::enable_if_t<std::is_same_v<C, std::atomic_uint8_t>
												  || std::is_same_v<C, std::atomic_uint16_t>
												  || std::is_same_v<C, std::atomic_uint32_t>
												  || std::is_same_v<C, std::atomic_uint64_t>>;*/
	public:

		explicit ref(T *ptr)
			: _ptr(ptr)
		{
			auto block = new control_block_ptr<T, C>(ptr);
			_ref_count = &block->_counter;
			_block = block;
		}

		explicit ref(T &&arg)
		{
			auto block = new control_block_object<T, C>(std::forward<T>(arg));
			_ptr = &block->_obj;
			_ref_count = &block->_counter;
			_block = block;
		}

		template <typename... Args>
		explicit ref(Args &&...args)
		{
			auto block = new control_block_object<T, C>(std::forward<Args>(args)...);
			_ptr = &block->_obj;
			_ref_count = &block->_counter;
			_block = block;
		}

		ref(const ref &rhs) noexcept
			: _ptr(rhs._ptr), _ref_count(rhs._ref_count), _block(rhs._block)
		{
			inc_ref();
		}

		ref(ref &&rhs) noexcept
			: _ptr(rhs._ptr), _ref_count(rhs._ref_count), _block(rhs._block)
		{
			rhs._ptr = nullptr;
			rhs._ref_count = nullptr;
			rhs._block = nullptr;
		}

		~ref() noexcept
		{
			if (_ref_count && dec_ref() == 0)
			{
				_delegate_destruction(destroyer(_block));

				_ptr = nullptr;
				_ref_count = nullptr;
			}
		}

		ref &operator=(const ref &rhs) noexcept
		{
			if (this == &rhs)
				return *this;

			rhs.inc_ref();

			_ptr = rhs._ptr;
			_ref_count = rhs._ref_count;
			_block = rhs._block;

			return *this;
		}

		ref &operator=(ref &&rhs) noexcept
		{
			if (this == &rhs)
				return *this;

			_ptr = rhs._ptr;
			_ref_count = rhs._ref_count;
			_block = rhs._block;

			rhs._ptr = nullptr;
			rhs._ref_count = nullptr;
			rhs._block = nullptr;

			return *this;
		}

		const T *get() const { return _ptr; }

		T *get() { return _ptr; }

		T &operator*() noexcept { return *_ptr; }

		const T &operator*() const noexcept { return *_ptr; }

		T *operator->() noexcept { return _ptr; }

		const T *operator->() const noexcept { return _ptr; }

	private:

		auto inc_ref()
		{
			return ++(*_ref_count);
		}

		auto dec_ref()
		{
			return --(*_ref_count);
		}

		T *_ptr;
		counter *_ref_count;
		abstract_control_block *_block;
	};

	template <class T, class C = std::atomic_uint8_t>
	class ref_array
	{
		using counter = C;

	public:
		explicit ref_array(const std::size_t size)
			: _data(new T[size]()), _size(size)
		{
			auto block = new control_block_array(_data);
			_ref_count = &block->counter;
			_block = block;
		}

		ref_array(const std::initializer_list<T> init)
			: _size(init.size())
		{
			auto block = new control_block_array(init.size());
			_data = &block->ptr;
			_ref_count = &block->counter;
			_block = block;

			std::move(init.begin(), init.end(), _data);
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
			rhs._size = 0;
		}

		~ref_array() noexcept
		{
			if (_ref_count && dec_ref() == 0)
			{
				_delegate_destruction(destroyer(_block));

				_data = nullptr;
				_ref_count = nullptr;
			}
		}

		ref_array &operator=(const ref_array &rhs) noexcept
		{
			if (this == &rhs)
				return *this;

			inc_ref();

			_data = rhs._data;
			_ref_count = rhs._ref_count;
			_size = rhs._size;

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

		T *_data;
		counter *_ref_count;
		std::size_t _size;
		abstract_control_block *_block;
	};
}
