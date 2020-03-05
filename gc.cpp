#include "gc.h"

#include <atomic>
#include <condition_variable>
#include <thread>

namespace gc
{
	std::list<gc::page> internal::pages_list{};
	std::shared_mutex internal::list_mutex{};
	
	static std::atomic_bool gc_paused{};
	static std::atomic_bool gc_running{ true };
	static std::condition_variable gc_cv{};
	static std::mutex gc_mutex{};
	static std::thread gc_thread{
		[]
		{
			using namespace std::chrono_literals;

			while (gc_running)
			{
				std::unique_lock lock{ gc_mutex };
				gc_cv.wait_for(lock, 10ms, [] { return !gc_paused; });
				
				if (!gc_running)
					break;
				
				run();
			}
		}
	};

	gc::page &internal::get_available_page()
	{
		{
			std::shared_lock shared_lock{ list_mutex };
			for (auto &page : pages_list)
			{
				page.get_mutex().lock();
				if (!page.full())
					return page;
				
				page.get_mutex().unlock();
			}
		}

		// All lists are full
		gc_cv.notify_all(); // Notify the GC that it should run

		std::unique_lock lock{ list_mutex };
		// In the meantime, create a new gc::page and use it to make the allocation
		auto &new_page = pages_list.emplace_back();
		new_page.get_mutex().lock();
		return new_page;
	}

	void clean_list(gc::page &page)
	{
		for (auto it = page.begin(); it < page.end(); ++it)
		{
			const auto &info = *it;
			if (info.is_valid() && info.no_references())
				page.erase(it);
		}
	}

	void run()
	{
		long long empty_pages_count{};

		{
			std::shared_lock shared_lock{ internal::list_mutex };

			for (auto &page : internal::pages_list)
			{
				std::lock_guard lock{ page.get_mutex() };

				if (page.full() || page.used_size() > (gc::page::capacity() / 4) * 3)
					clean_list(page);
				else if (page.empty())
					++empty_pages_count;
			}
		}

		if (empty_pages_count > 3)
		{
			std::unique_lock unique_lock{ internal::list_mutex };
			
			internal::pages_list.remove_if([&empty_pages_count](const page &page)
			{
				return page.empty() && empty_pages_count-- > 3;
			});
		}
	}

	bool is_paused() noexcept
	{
		return gc_paused;
	}

	void pause() noexcept
	{
		gc_paused = true;
	}

	void resume() noexcept
	{
		gc_paused = false;
	}

	void shutdown()
	{
		gc_paused = false;
		gc_running = false;
		gc_thread.join();
		run();
		internal::pages_list.clear();
		gc_paused = true;
	}
}
