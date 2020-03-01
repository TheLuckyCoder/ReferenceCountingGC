#include <iostream>

#include "gc.h"
#include "ref.h"

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
	std::vector<ref<test_struct>> vec;

	ref<int> sum;
	for (int i = 0; i < 4096; ++i)
	{
		ref<test_struct> a(i);
		if (i & 1)
			vec.push_back(a);

		*sum += *(a->ptr);
	}
	return *sum;
}

int main()
{
	std::vector<std::thread> threads;
	for (int i = 0; i < 8; ++i)
	{
		threads.emplace_back(run_test);
	}
	
	ref<int> a;

	for (auto &t : threads)
		t.join();
	gc::close_gc();
	return 0;
}
