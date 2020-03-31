#pragma once

#include <initializer_list>

#include "gc.h"

namespace gc
{
	template <class T>
	class ref
	{
	public:
		ref() noexcept = default;

		explicit ref(T &&arg)
			: _pointer(&gc::new_pointer()), _data(_pointer->init<T>())
		{
			new(_data) T(std::forward<T>(arg));
		}

		template <typename... Args>
		explicit ref(Args && ...args)
			: _pointer(&gc::new_pointer()), _data(_pointer->init<T>())
		{
			new(_data) T(std::forward<Args>(args)...);
		}

		ref(const ref &other) noexcept
		{
			_pointer = other._pointer;
			_data = other._data;
			_pointer->inc_references();
		}

		ref(ref &&other) noexcept
		{
			_pointer = other._pointer;
			_data = other._data;

			other._pointer = nullptr;
			other._data = nullptr;
		}

		~ref() noexcept
		{
			if (_pointer)
				_pointer->dec_references();

			_pointer = nullptr;
			_data = nullptr;
		}

		ref &operator=(const ref &rhs) noexcept
		{
			if (this == &rhs)
				return *this;

			rhs._pointer->inc_references();
			_pointer = rhs._pointer;
			_data = rhs._data;

			return *this;
		}

		ref &operator=(ref &&rhs) noexcept
		{
			_pointer = rhs._pointer;
			_data = rhs._data;

			rhs._pointer = nullptr;
			rhs._data = nullptr;

			return *this;
		}

		T &operator*() noexcept { return *_data; }
		const T &operator*() const noexcept { return *_data; }

		T *operator->() noexcept { return _data; }
		const T *operator->() const noexcept { return _data; }

	private:
		gc::pointer *_pointer = nullptr;
		T *_data = nullptr;
	};

	template <class T>
	class ref_array
	{
	public:
		explicit ref_array(const std::size_t size)
			: _pointer(&gc::new_pointer()), _data(_pointer->init_array<T>(size)), _size(size)
		{
		}

		ref_array(const std::initializer_list<T> init)
			: _pointer(&gc::new_pointer()), _data(_pointer->init_array<T>(init.size())), _size(init.size())
		{
			std::copy(init.begin(), init.end(), _data);
		}

		ref_array(const ref_array &other) noexcept
		{
			_pointer = other._pointer;
			_data = other._data;
			_size = other._size;

			_pointer->inc_references();
		}

		ref_array(ref_array &&other) noexcept
		{
			_pointer = other._pointer;
			_data = other._data;
			_size = other._size;

			other._pointer = nullptr;
			other._data = nullptr;
			other._size = 0;
		}

		~ref_array() noexcept
		{
			if (_pointer)
				_pointer->dec_references();

			_pointer = nullptr;
			_data = nullptr;
			_size = 0;
		}

		ref_array &operator=(const ref_array &other) noexcept
		{
			if (this != &other)
			{
				_pointer = other._pointer;
				_data = other._data;
				_size = other._size;

				_pointer->inc_references();
			}

			return *this;
		}

		ref_array &operator=(ref_array &&other) noexcept
		{
			_pointer = other._pointer;
			_data = other._data;
			_size = other._size;

			other._pointer = nullptr;
			other._data = nullptr;
			other._size = 0;

			return *this;
		}

		// Iterators
		T *begin() noexcept { return _data; }
		const T *begin() const noexcept { return _data; }

		T *end() noexcept { return _data + _size; }
		const T *end() const noexcept { return _data + _size; }

		T &operator*() noexcept { return *_data; }
		const T &operator*() const noexcept { return *_data; }

		T &operator[](const std::size_t i) noexcept { return _data[i]; }
		const T &operator[](const std::size_t i) const noexcept { return _data[i]; }

	private:
		gc::pointer *_pointer;
		T *_data;
		std::size_t _size;
	};
}

using gc::ref;
using gc::ref_array;
