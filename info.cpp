#include "info.h"

#include <functional>

namespace gc
{
	info::info(info &&other) noexcept
		: ptr(other.ptr), deleter(other.deleter), ref_count(static_cast<std::size_t>(other.ref_count))
	{
		other.ptr = nullptr;
		other.deleter = nullptr;
	}

	info::~info()
	{
		if (ptr && deleter)
			std::invoke(deleter, ptr);
		ptr = nullptr;
		deleter = nullptr;
	}

	info &info::operator=(info &&other) noexcept
	{
		ptr = other.ptr;
		deleter = other.deleter;
		ref_count = static_cast<std::size_t>(other.ref_count);

		other.ptr = nullptr;
		other.deleter = nullptr;

		return *this;
	}

	bool info::is_valid() const noexcept
	{
		return ptr != nullptr;
	}

	void info::inc_references() noexcept
	{
		++ref_count;
	}

	void info::dec_references() noexcept
	{
		--ref_count;
	}

	bool info::no_references() const noexcept
	{
		return ref_count == 0;
	}
}
