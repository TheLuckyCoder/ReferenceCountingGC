#include "gc.h"

#include <atomic>
#include <iostream>
#include <thread>

namespace gc
{
	static std::mutex info_mutex;
	static gc_list<info> info_list;
	static std::atomic_bool gc_running{ true };
	static std::thread gc_thread{
		[]
		{
			while (gc_running)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(600));
				run();
			}
		}
	};

	void run() noexcept(false)
	{
		std::lock_guard lock{ info_mutex };

		for (auto it = info_list.begin(); it < info_list.end(); ++it)
		{
			auto &info = *it;
			if (info.isValid() && info.noReferences())
			{
				if (info_list.erase(it))
					break;
			}
		}

		std::cout << "List Size: " << info_list.size() << '\n';
	}

	gc_list<info> &get_info_list() noexcept
	{
		return info_list;
	}

	std::mutex &get_mutex() noexcept
	{
		return info_mutex;
	}

	/**
	 * Stop the Garbage Collection from running
	 *
	 * This should only be called at the end of a program
	 */
	void close_gc()
	{
		gc_running = false;
		gc_thread.join();
		info_list.clear();
	}
}
