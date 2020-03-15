#include "info.h"

namespace gc
{
	void info::destroy() noexcept
	{
		if (ptr && deleter)
			deleter(ptr);
		ptr = nullptr;
		deleter = nullptr;
	}

	info::~info() noexcept
	{
		destroy();
	}

	bool info::is_valid() const noexcept
	{
		return ptr;
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
