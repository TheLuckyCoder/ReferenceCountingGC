#pragma once

#include <shared_mutex>
#include <list>

#include "page.h"

namespace gc
{
	struct internal
	{
		static std::list<gc::page> pages_list;
		static std::shared_mutex list_mutex;

		/**
		 * @returns a non-full gc::page for a new allocation
		 * 
		 * The returned gc::page is locked
		 */
		static gc::page &get_available_page();

		internal() = delete;
	};

	/**
	 * Forces the GC to run
	 */
	void run();

	bool is_paused() noexcept;

	/**
	 * Pauses future runs of the GC
	 *
	 * The GC won't run again until it is resumed
	 */
	void pause() noexcept;

	/**
	 * Resumes future runs of the GC
	 */
	void resume() noexcept;

	/**
	 * @returns a pointer to a newly initialized object
	 */
	template <typename T>
	info *new_info(const T *ptr);

	/**
	 * Stops the Garbage Collector Thread and frees up all the allocated objects
	 *
	 * This should only be called at the end of a program
	 */
	void shutdown();
};

template <typename T>
gc::info *gc::new_info(const T *ptr)
{
	auto &page = internal::get_available_page();
	auto &info = page.add_element(ptr);

	// get_available_page locks the list so we must unlock
	page.get_mutex().unlock();
	return &info;
}
