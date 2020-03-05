#pragma once

#include <shared_mutex>
#include <list>

#include "gc_list.h"

namespace gc
{
	struct internal
	{
		static std::list<gc_list> info_list;
		static std::shared_mutex list_mutex;

		/**
		 * @returns an available gc_list for a new allocation
		 * 
		 * The returned list is locked
		 */
		static gc_list &get_available_list();
	
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
	void close() noexcept(false);
};

template <typename T>
gc::info *gc::new_ref(const T *ptr) noexcept
{
	auto &list = internal::get_available_list();
	auto &info = list.add_element(ptr);

	// get_available_list locks the list so we must unlock
	list.get_lock().unlock();
	return &info;
}
