#pragma once

#include "ref.h"

namespace gc
{
	/**
	 * Start the Garbage Collector Thread
	 * This should be called at the start of a program or before any gc::ref or gc::ref_array objects are created
	 *
	 * @param timeout the time period at which the GC should run periodically, in milliseconds
	 */
	void start(std::size_t timeout = 200);

	/**
	 * Suggests to the GC that it should run now
	 *
	 * The GC will not run if it is already running or it is paused
	 */
	void suggest_run();

	/**
	 * Check if the execution of the GC has been paused
	 */
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
	 * Stops the Garbage Collector Thread and frees up all the allocated objects
	 *
	 * This should only be called at the end of a program
	 * or when the GC and the objects allocated with it
	 * are no longer needed
	 */
	void shutdown();
};
