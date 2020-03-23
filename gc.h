#pragma once

#include "page.h"

namespace gc
{
	struct internal
	{
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
	template <typename T, bool Array>
	info *new_info();

	/**
	 * Stops the Garbage Collector Thread and frees up all the allocated objects
	 *
	 * This should only be called at the end of a program
	 */
	void shutdown();
};

template <typename T, bool Array>
gc::info *gc::new_info()
{
	auto &page = internal::get_available_page();
	auto &info = page.add_element<T, Array>();

	// get_available_page locks the list so we must unlock it,
	// after we add the element
	page.get_mutex().unlock();
	return &info;
}
