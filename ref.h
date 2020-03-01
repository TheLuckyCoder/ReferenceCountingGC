#pragma once

#include "gc.h"

template <typename T>
class ref
{
public:
	template <typename... Args>
	ref(Args && ...args)
	{
		_ptr = new T(std::forward<Args>(args)...);
		_info = gc::new_ref(_ptr);
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
	}

	ref &operator=(const ref &other) noexcept
	{
		_ptr = other._ptr;
		_info = other._info;
		_info->inc_references();

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

	T &operator*() noexcept
	{
		return *_ptr;
	}

	T *operator->() noexcept
	{
		return _ptr;
	}

private:
	T *_ptr = nullptr;
	gc::info *_info = nullptr;
};
