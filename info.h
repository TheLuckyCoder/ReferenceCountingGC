#pragma once

#include <atomic>

namespace gc
{
	class info
	{
		template <typename T>
		struct Manager
		{
			static void deleter(const void *pVoid)
			{
				auto ptr = static_cast<const T*>(pVoid);
				delete ptr;
			}
		};

	public:
		info() = default;
		
		template <typename T>
		explicit info(T *t) : ptr((void*)(t)), deleter(&Manager<T>::deleter), ref_count(1)
		{
		}

		template <typename T>
		void construct(T *t)
		{
			ptr = (void*)(t);
			deleter = &Manager<T>::deleter;
			ref_count = 1;
		}

		void destroy();

		info(info &&other) noexcept;
		~info();

		info &operator=(info &&other) noexcept;

		bool is_valid() const noexcept;
		void inc_references() noexcept;
		void dec_references() noexcept;
		bool no_references() const noexcept;

	private:
		void *ptr = nullptr;
		void (*deleter)(const void *) = nullptr;
		std::atomic_size_t ref_count{};
	};
}
