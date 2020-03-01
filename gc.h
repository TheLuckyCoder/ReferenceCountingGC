#pragma once

#include <mutex>

#include "gc_list.h"
#include "info.h"

namespace gc
{	
	/**
	 * Force run the GC
	 */
	void run() noexcept(false);

	/**
	 * @returns a pointer to a newly initialized object
	 */
	template <typename T>
	info *new_ref(const T *ptr) noexcept;

	gc_list<info> &get_info_list() noexcept;
	std::mutex &get_mutex() noexcept;
	
	/**
	 * Stops the Garbage Collector Thread and frees up all the allocated objects
	 */
	void close_gc() noexcept(false);
};

template <typename T>
gc::info *gc::new_ref(const T *ptr) noexcept
{
	std::lock_guard lock{ gc::get_mutex() };
	return &get_info_list().add_element(ptr);
}
