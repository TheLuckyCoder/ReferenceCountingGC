#pragma once

#include <initializer_list>

#include "gc.h"

namespace gc
{
	template <class T>
	class ref
	{
	public:
		explicit ref(T &&arg)
			: _info(gc::new_info<T, false>()), _ptr(_info->get<T, false>())
		{
			new(_ptr) T(std::forward<T>(arg));
		}

		template <typename... Args>
		explicit ref(Args && ...args)
			: _info(gc::new_info<T, false>()), _ptr(_info->get<T, false>())
		{
			new(_ptr) T(std::forward<Args>(args)...);
		}

		ref(const ref &other) noexcept
		{
			_ptr = other._ptr;
			_info = other._info;
			_info->inc_references();
		}

		ref(ref &&other) noexcept
		{
			_ptr = other._ptr;
			_info = other._info;

			other._ptr = nullptr;
			other._info = nullptr;
		}

		~ref() noexcept
		{
			if (_info)
				_info->dec_references();

			_ptr = nullptr;
			_info = nullptr;
		}

		ref &operator=(const ref &other) noexcept
		{
			if (this != &other)
			{
				_ptr = other._ptr;
				_info = other._info;
				_info->inc_references();
			}

			return *this;
		}

		ref &operator=(ref &&other) noexcept
		{
			_ptr = other._ptr;
			_info = other._info;

			other._ptr = nullptr;
			other._info = nullptr;

			return *this;
		}

		T &operator*() noexcept { return *_ptr; }
		const T &operator*() const noexcept { return *_ptr; }

		T *operator->() noexcept { return _ptr; }
		const T *operator->() const noexcept { return _ptr; }

	private:
		gc::info *_info;
		T *_ptr;
	};

	template <class T>
	class ref_array
	{
	public:
		explicit ref_array(const std::size_t size)
			: _info(gc::new_info<T, true>(_ptr)), _ptr(_info->get<T, true>()), _size(size)
		{
		}

		ref_array(const std::initializer_list<T> init)
			: _info(gc::new_info<T, true>(_ptr)), _ptr(_info->get<T, true>()), _size(init.size())
		{
			std::copy(init.begin(), init.end(), _ptr);
		}

		ref_array(const ref_array &other) noexcept
		{
			_ptr = other._ptr;
			_info = other._info;
			_size = other._size;
			_info->inc_references();
		}

		ref_array(ref_array &&other) noexcept
		{
			_ptr = other._ptr;
			_info = other._info;
			_size = other._size;

			other._ptr = nullptr;
			other._info = nullptr;
			other._size = 0;
		}

		~ref_array() noexcept
		{
			if (_info)
				_info->dec_references();

			_ptr = nullptr;
			_info = nullptr;
			_size = 0;
		}

		ref_array &operator=(const ref_array &other) noexcept
		{
			if (this != &other)
			{
				_ptr = other._ptr;
				_info = other._info;
				_size = other._size;

				_info->inc_references();
			}

			return *this;
		}

		ref_array &operator=(ref_array &&other) noexcept
		{
			_ptr = other._ptr;
			_info = other._info;
			_size = other._size;

			other._ptr = nullptr;
			other._info = nullptr;
			other._size = nullptr;

			return *this;
		}

		// Iterators
		T *begin() noexcept { return _ptr; }
		const T *begin() const noexcept { return _ptr; }

		T *end() noexcept { return _ptr + _size; }
		const T *end() const noexcept { return _ptr + _size; }

		T &operator*() noexcept { return *_ptr; }
		const T &operator*() const noexcept { return *_ptr; }

		T &operator[](const std::size_t i) noexcept { return _ptr[i]; }
		const T &operator[](const std::size_t i) const noexcept { return _ptr[i]; }

	private:
		gc::info *_info;
		T *_ptr;
		std::size_t _size;
	};
}

using gc::ref;
using gc::ref_array;
