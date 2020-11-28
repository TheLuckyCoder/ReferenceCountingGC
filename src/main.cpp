#include <iostream>
#include <thread>
#include <vector>

#include "gc/gc.h"

static std::atomic_bool wait_atomic{ true };

struct int_test_struct
{
	int *ptr;

	explicit int_test_struct(const int index) noexcept
	{
		ptr = new int(index);
	}

	~int_test_struct() noexcept
	{
		delete ptr;
	}

	int get() const noexcept
	{
		return *ptr;
	}
};

int run_test()
{
	constexpr auto test_size = 65536;
	std::vector<gc::ref<int_test_struct>> vec;
	vec.reserve(test_size / 2);

	while (wait_atomic)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

	gc::ref<int> sum(0);

	for (int i = 0; i < test_size; ++i)
	{
		gc::ref<int_test_struct> a(i);
		*sum += a->get();
		if (i & 1)
			vec.push_back(std::move(a));
	}

	return *sum;
}

int main()
{
	using namespace std::chrono;
	using namespace std::chrono_literals;

	{
		constexpr auto thread_count = 128;
		std::vector<std::thread> threads;
		threads.reserve(thread_count);

		for (int i = 0; i < thread_count; ++i)
			threads.emplace_back(run_test);

		gc::start();
		std::this_thread::sleep_for(3s);

		const auto start_time = high_resolution_clock::now();
		wait_atomic = false;

		for (auto &t : threads)
			t.join();

		const auto time_needed = duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count();
		std::cout << "Time Needed: " << time_needed << "ms" << std::endl;
	}

	std::this_thread::sleep_for(3s);
	gc::shutdown();
	return 0;
}
