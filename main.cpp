#include <iostream>
#include <vector>

#include "gc.h"
#include "ref.h"

static std::mutex sync{};

using namespace std::chrono_literals;

struct test_struct
{
	int *ptr;

	test_struct(const int index) noexcept
	{
		ptr = new int(index);
	}

	~test_struct()
	{
		delete ptr;
	}
};

int run_test()
{
	constexpr auto test_size =  8192 * 8;
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
	{
		sync.lock();
		constexpr auto thread_count = 128;
		std::vector<std::thread> threads;
		threads.reserve(thread_count);
		
		for (int i = 0; i < thread_count; ++i)
			threads.emplace_back(run_test);
		
		std::this_thread::sleep_for(3s);
		sync.unlock();
		
		for (auto &t : threads)
			t.join();
	}

	std::this_thread::sleep_for(3s);
	gc::shutdown();
	return 0;
}
