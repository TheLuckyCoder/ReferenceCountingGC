#pragma once

#include <atomic>

namespace gc
{
	class info
	{
		template <typename T>
		struct manager
		{
			static void object_deleter(const void *p_void)
			{
				auto ptr = static_cast<const T*>(p_void);
				delete ptr;
			}

			static void array_deleter(const void *p_void)
			{
				auto ptr = static_cast<const T*>(p_void);
				delete[] ptr;
			}

			manager() = delete;
			manager(const manager &) = delete;
			manager(manager &&) = delete;
			~manager() = delete;
			manager &operator=(const manager &) = delete;
			manager &operator=(manager &&) = delete;
		};

	public:
		info() = default;

		info(const info &) = delete;
		info(info &&other) noexcept;
		~info() noexcept;

		info &operator=(const info &) = delete;
		info &operator=(info &&other) noexcept;

		template <typename T, bool Array>
		void construct(T *t) noexcept
		{
			ptr = (void*)(t);
			if constexpr (Array)
				deleter = &manager<T>::array_deleter;
			else
				deleter = &manager<T>::object_deleter;
			ref_count = 1;
		}

		void destroy() noexcept;

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
