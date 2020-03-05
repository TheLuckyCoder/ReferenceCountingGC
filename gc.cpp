#include "gc.h"

#include <atomic>
#include <condition_variable>
#include <thread>

namespace gc
{
	std::list<gc_list> internal::info_list{};
	std::shared_mutex internal::list_mutex{};
	static std::atomic_bool gc_running{ true };
	static std::condition_variable gc_cv{};
	static std::mutex gc_mutex{};
	static std::thread gc_thread{
		[]
		{
			using namespace std::chrono_literals;

			while (gc_running)
			{
				std::unique_lock<std::mutex> lock(gc_mutex);
				gc_cv.wait_for(lock, 10ms);
				run();
			}
		}
	};

	gc_list &internal::get_available_list()
	{
		{
			std::shared_lock shared_lock{ list_mutex };
			for (auto &list : info_list)
			{
				list.get_lock().lock();
				if (!list.full())
					return list;
				list.get_lock().unlock();
			}
		}

		// All lists are full
		gc_cv.notify_all(); // Notify the GC that it should run

		std::unique_lock lock{ list_mutex };
		auto &new_list = info_list.emplace_back(); // In the meantime, create a new list and use it
		new_list.get_lock().lock();
		return new_list;
	}

	void clean_list(gc_list &list)
	{
		for (auto it = list.begin(); it < list.end(); ++it)
		{
			const auto &info = *it;
			if (info.is_valid() && info.no_references())
			{
				list.erase(it);
			}
		}
	}

	void run() noexcept(false)
	{
		int empty_lists_count{};

		{
			std::shared_lock shared_lock{ internal::list_mutex };

			for (auto &list : internal::info_list)
			{
				std::lock_guard lock{ list.get_lock() };

				if (list.full() || list.used_size() > (list.capacity() / 4) * 3)
					clean_list(list);
				else if (list.empty())
					++empty_lists_count;
			}
		}

		if (empty_lists_count > 3)
		{
			std::unique_lock unique_lock{ internal::list_mutex };
			
			internal::info_list.remove_if([&empty_lists_count](const gc_list &list)
			{
				return list.empty() && empty_lists_count-- > 3;
			});
		}
	}

	void close()
	{
		gc_running = false;
		gc_thread.join();
		run();
		internal::info_list.clear();
	}
}
