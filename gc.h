#pragma once

#include <mutex>

#include "gc_list.h"
#include "info.h"

namespace gc
{
	struct internal
	{
		static std::mutex list_mutex;
		static gc_list<info> info_list;
	
		internal() = delete;
	};

	/**
	 * Force run the GC
	 */
	void run() noexcept(false);

	/**
	 * @returns a pointer to a newly initialized object
	 */
	template <typename T>
	info *new_ref(const T *ptr) noexcept;
	
	/**
	 * Stops the Garbage Collector Thread and frees up all the allocated objects
	 *
	 * This should only be called at the end of a program
	 */
	void close_gc() noexcept(false);
};

template <typename T>
gc::info *gc::new_ref(const T *ptr) noexcept
{
	std::lock_guard lock{ internal::list_mutex };
	return &internal::info_list.add_element(ptr);
}
