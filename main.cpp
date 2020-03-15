#include <iostream>
#include <thread>
#include <vector>

#include "ref.h"

static std::mutex sync{};

struct test_struct
{
	int *ptr;

	explicit test_struct(const int index) noexcept
	{
		ptr = new int(index);
	}

	~test_struct() noexcept
	{
		delete ptr;
	}
};

int run_test()
{
	constexpr auto test_size = 65536;
	std::vector<ref<test_struct>> vec;
	vec.reserve(test_size / 2);
	
	sync.lock();
	ref sum(0);
	sync.unlock();
	
	for (int i = 0; i < test_size; ++i)
	{
		ref<test_struct> a(i);
		if (i & 1)
			vec.push_back(a);

		*sum += *a->ptr;
	}

	return *sum;
}

int main()
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

	{
		sync.lock();
		constexpr auto thread_count = 128;
		std::vector<std::thread> threads;
		threads.reserve(thread_count);
		
		for (int i = 0; i < thread_count; ++i)
			threads.emplace_back(run_test);
		
		std::this_thread::sleep_for(1s);

		const auto start_time = steady_clock::now();
		sync.unlock();

		for (auto &t : threads)
			t.join();

        const auto time_needed = duration_cast<milliseconds>(steady_clock::now() - start_time).count();
        std::cout << "Time Needed: " << time_needed << std::endl;
	}

	std::this_thread::sleep_for(1s);
	gc::shutdown();
	return 0;
}
