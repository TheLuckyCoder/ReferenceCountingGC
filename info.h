#pragma once

#include <atomic>

namespace gc
{
	class info
	{
		template <typename T>
		struct manager
		{
			static void object_destructor(void **p_void)
			{
				auto object = reinterpret_cast<T*>(p_void);
				object->~T();
			}

			static void object_deleter(void **p_void)
			{
				auto ptr = static_cast<T*>(*p_void);
				delete ptr;
			}

			static void array_deleter(void **p_void)
			{
				auto ptr = static_cast<T*>(*p_void);
				delete[] ptr;
			}

			constexpr static bool is_small_type = sizeof(T) <= sizeof(void*);

			manager() = delete;
			manager(const manager &) = delete;
			manager(manager &&) = delete;
			~manager() = delete;
			manager &operator=(const manager &) = delete;
			manager &operator=(manager &&) = delete;
		};

	public:		
		info() noexcept = default;

		info(const info &) = delete;
		info(info &&other) = delete;

		~info() noexcept
		{
			destroy();
		}

		info &operator=(const info &) = delete;
		info &operator=(info &&other) = delete;

		template <typename T, bool Array>
		void construct() noexcept
		{
			_ref_count = 1;
			if constexpr (manager<T>::is_small_type)
				_ptr = nullptr;
			else
				_ptr = operator new(sizeof(T));

			if constexpr (Array)
				_deleter = &manager<T>::array_deleter;
			else if constexpr (manager<T>::is_small_type)
				_deleter = &manager<T>::object_destructor;
			else
				_deleter = &manager<T>::object_deleter;
		}

		template <typename T, bool Array>
		T *get() const
		{
			auto t_ptr = static_cast<T*>(_ptr);
			if constexpr (Array || !manager<T>::is_small_type)
				return t_ptr;
			else
				return (T*)&_ptr;
		}

		void destroy() noexcept
		{
			if (_deleter)
				_deleter(&_ptr);
			_ptr = nullptr;
			_deleter = nullptr;
		}

		bool is_valid() const noexcept
		{
			return _ptr;
		}

		bool has_references() const noexcept
		{
			return _ref_count != 0;
		}

		void inc_references() noexcept
		{
			++_ref_count;
		}

		void dec_references() noexcept
		{
			--_ref_count;
		}

	private:
		void (*_deleter)(void **) = nullptr;
		std::atomic_size_t _ref_count{};
		void *_ptr = nullptr;
	};
}
