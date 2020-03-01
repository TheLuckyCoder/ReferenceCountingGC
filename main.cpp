#include <iostream>

#include "gc.h"

template <typename T>
class ref
{
public:
	template <typename... Args>
	ref(Args && ...args)
	{
		ptr = new T(std::forward<Args>(args)...);
		info = gc::new_ref(ptr);
	}

	ref(ref &&other) noexcept
	{
		ptr = other.ptr;
		info = other.info;

		other.ptr = nullptr;
		other.info = nullptr;
	}

	~ref() noexcept
	{
		if (info)
			info->decrementRef();
	}

private:
	T *ptr = nullptr;
	gc::info *info = nullptr;
};

struct test_struct
{
	int i;

	test_struct(const int index) noexcept : i(index)
	{
		std::cout << "Constructed: " << index << '\n';
	}

	~test_struct() noexcept
	{
		std::cout << "Destroyed: " << i << '\n';
	}
};

void run_test()
{
	std::vector<ref<test_struct>> vec;

	for (int i = 0; i < 1024; ++i)
	{
		ref<test_struct> a(i);
		if (i & 1)
			vec.push_back(std::move(a));
	}
}

int main()
{
	run_test();

	gc::close_gc();
	return 0;
}
