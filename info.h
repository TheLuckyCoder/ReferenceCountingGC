#pragma once

#include <atomic>

namespace gc
{
	class info
	{
		template <typename T>
		struct manager
		{
			static void deleter(const void *p_void)
			{
				auto ptr = static_cast<const T*>(p_void);
				delete ptr;
			}

			manager() = delete;
			manager(const manager &) = delete;
			manager(manager &&) = delete;
			~manager() = default;
			manager &operator=(const manager &) = delete;
			manager &operator=(manager &&) = delete;
		};

	public:
		info() = default;

		template <typename T>
		explicit info(T *t) : ptr((void*)(t)), deleter(&manager<T>::deleter), ref_count(1)
		{
		}

		template <typename T>
		void construct(T *t)
		{
			ptr = (void*)(t);
			deleter = &manager<T>::deleter;
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
