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
		template <typename T>
		explicit info(T *t) : ptr((void*)(t)), deleter(&Manager<T>::deleter), ref_count(1)
		{
		}

		info(info &&other) noexcept;
		~info();

		info &operator=(info &&other) noexcept;

		bool isValid() const noexcept;
		void incrementRef() noexcept;
		void decrementRef() noexcept;
		bool noReferences() const noexcept;

	private:
		void *ptr;
		void (*deleter)(const void *);
		std::atomic_size_t ref_count;
	};
}
