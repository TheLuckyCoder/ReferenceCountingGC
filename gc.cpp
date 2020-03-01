#include "gc.h"

#include <atomic>
#include <iostream>
#include <thread>

namespace gc
{
	std::mutex internal::list_mutex{};
	gc_list internal::info_list{};
	static std::atomic_bool gc_running{ true };
	static std::thread gc_thread{
		[]
		{
			while (gc_running)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				run();
			}
		}
	};

	void clean_list(gc_list &list)
	{
		for (auto it = list.begin(); it < list.end(); ++it)
		{
			auto &info = *it;
			if (info.is_valid() && info.no_references())
			{
				list.erase(it);
			}
		}

		std::cout << "Unused List Size: " << list.unused_space() << '\n';
	}

	void run() noexcept(false)
	{
		std::lock_guard lock{ internal::list_mutex };
		clean_list(internal::info_list);
	}

	void close_gc()
	{
		gc_running = false;
		gc_thread.join();
		internal::info_list.clear();
	}
}
