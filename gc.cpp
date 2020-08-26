#include "gc.h"

#include "page.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <vector>
#include <thread>
#include <iostream>

namespace gc
{
	static std::vector<gc::page*> pages_list{};
	static std::mutex pages_list_mutex{};

	static std::atomic_int32_t gc_count{};
	static std::atomic_bool gc_alive{};
	static std::atomic_bool gc_paused{};
	static std::condition_variable gc_cv{};
	static std::mutex gc_mutex{};
	static std::thread gc_thread{};

	static void run()
	{
		std::lock_guard lock{ pages_list_mutex };
		if (pages_list.empty())
			return;
		++gc_count;
		for (auto *ptr : pages_list)
			ptr->clear();
	}

	void start(const std::size_t timeout)
	{
		gc_alive = true;
		gc_paused = false;
		pages_list.reserve(128);

		gc_thread = std::thread([timeout]
		{
			while (gc_alive)
			{
				// Set up the Condition Variable
				std::unique_lock gc_lock{ gc_mutex };
				gc_cv.wait_for(gc_lock, std::chrono::milliseconds(timeout), [] { return !gc_paused; });

				if (!gc_alive)
					break;

				// Run the GC
				run();
			}
		});
	}

	void suggest_run()
	{
        gc_cv.notify_all();
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
		// Resume and stop the GC so it may quit the loop
		gc_paused = false;
		gc_alive = false;
		gc_cv.notify_all();

		if (gc_thread.joinable())
			gc_thread.join();

		// Free all the allocated memory
		std::lock_guard lock{ pages_list_mutex };
		std::cout << "Run Count: " << gc_count << '\n';

		pages_list.clear();
		gc_paused = true;
	}

    void delegate_destruction(destroyer &&destroyer)
    {
		struct thread_page
		{
			thread_page()
			{
				std::lock_guard lock{ pages_list_mutex };
				pages_list.emplace_back(&data);
			}

			thread_page(const thread_page &) = delete;
			thread_page(thread_page &&) = delete;

			~thread_page()
			{
				std::lock_guard lock{ pages_list_mutex };

				const auto it = std::remove(pages_list.begin(), pages_list.end(), &data);
				if (it != pages_list.end())
					pages_list.erase(it, pages_list.end());
			}

			thread_page &operator=(const thread_page &) = delete;
			thread_page &operator=(thread_page &&) = delete;

			gc::page data{};
		};

        static thread_local thread_page local_page{};

		local_page.data.add(std::move(destroyer));
    }
}
