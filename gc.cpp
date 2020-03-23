#include "gc.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <shared_mutex>
#include <vector>
#include <thread>

namespace gc
{
	static std::vector<gc::page*> pages_list{};
	static std::shared_mutex pages_list_mutex{};

	static std::atomic_bool gc_paused{};
	static std::atomic_bool gc_running{ true };
	static std::condition_variable gc_cv{};
	static std::mutex gc_mutex{};
	static std::thread gc_thread{
		[]
		{
			pages_list.reserve(1024);
			using namespace std::chrono_literals;

			while (gc_running)
			{
				std::unique_lock lock{ gc_mutex };
				gc_cv.wait_for(lock, 20ms, [] { return !gc_paused; });

				if (!gc_running)
					break;

				run();
			}
		}
	};

	gc::page &internal::get_available_page()
	{
		{
			std::size_t i{};
			std::shared_lock shared_lock{ pages_list_mutex };

			for (auto *page_ptr : pages_list)
			{
				auto &page = *page_ptr;
				if (!page.get_mutex().try_lock())
					continue;
				if (!page.full())
					return page;
				page.get_mutex().unlock();

				if (i > 5)
				{
					// Knowing the pages should be sorted
					// we stop searching for an available page
					// since it's very unlikely to find one
					break;
				}
				++i;
			}
		}

		// Create a new gc::page and use it to make the allocation
		auto *new_page = new gc::page{};
		new_page->get_mutex().lock();

		{
			std::unique_lock lock{ pages_list_mutex };
			pages_list.emplace(pages_list.begin(), new_page);
		}

		gc_cv.notify_all(); // Notify the GC that it should run

		return *new_page;
	}

	void clean_list(gc::page &page)
	{
		for (auto it = page.begin(); it < page.end(); ++it)
		{
			const auto &info = *it;
			if (info.is_valid() && !info.has_references())
				page.erase(it);
		}
	}

	void run()
	{
		{
			std::shared_lock shared_lock{ pages_list_mutex };

			for (auto &page_ptr : pages_list)
			{
				auto &page = *page_ptr;
				std::lock_guard lock{ page.get_mutex() };

				if (page.full() || page.used_size() > gc::page::capacity() / 10)
					clean_list(page);
			}
		}

		std::unique_lock unique_lock{ pages_list_mutex };

		std::sort(pages_list.begin(), pages_list.end(), [](const gc::page *lhs, const gc::page *rhs)
		{
			return lhs->used_size() < rhs->used_size();
		});

		auto it = pages_list.begin();
		auto end = pages_list.end();

		while (it != end)
		{
			const auto &page = *(*it);
			if (page.empty())
			{
				delete *it;
				it = pages_list.erase(it);
				end = pages_list.end();
			} else
			{
				// We already sorted the vector so if this page
				// is not empty, then the next won't be either
				break;
			}
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
		// Unpause and stop the GC so it may quit the loop
		gc_paused = false;
		gc_running = false;
		gc_cv.notify_all();

		gc_thread.join();

		// Free all the allocated memory
		pages_list.clear();
		gc_paused = true;
	}
}
