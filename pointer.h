#pragma once

#include <atomic>

namespace gc
{
	class pointer
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

			static constexpr bool is_small_type = sizeof(T) <= sizeof(void*);

			manager() = delete;
			manager(const manager &) = delete;
			manager(manager &&) = delete;
			~manager() = delete;
			manager &operator=(const manager &) = delete;
			manager &operator=(manager &&) = delete;
		};

	public:		
		pointer() noexcept = default;

		pointer(const pointer &) = delete;
		pointer(pointer &&) = delete;

		~pointer() noexcept
		{
			destroy();
		}

		pointer &operator=(const pointer &) = delete;
		pointer &operator=(pointer &&) = delete;

		template <typename T>
		T *init() noexcept
		{
			_ref_count = 1;

			if constexpr (manager<T>::is_small_type)
			{
				_is_object = true;
				_ptr = nullptr;

				// We don't need a destructor if it's trivially destructible
				if constexpr (std::is_trivially_destructible_v<T>)
					_deleter = nullptr;
				else
					_deleter = &manager<T>::object_destructor;
				
				return (T*)&_ptr;
			}

			_is_object = false;
			// We only allocate the memory
			// we don't want to default initialize the object
			_ptr = operator new(sizeof(T));
			_deleter = &manager<T>::object_deleter;
			
			return static_cast<T*>(_ptr);
		}

		template <typename T>
		T *init_array(const std::size_t size) noexcept
		{
			_ref_count = 1;
			_is_object = false;
			
			if constexpr (std::is_default_constructible_v<T>)
				_ptr = new T[size]{};
			else
				_ptr = operator new[](sizeof(T) * size);

			_deleter = &manager<T>::array_deleter;

			return static_cast<T*>(_ptr);
		}

		void destroy() noexcept
		{
			if (_deleter)
			{
				try
				{
					_deleter(&_ptr);
				} catch (...)
				{
				}
			}
			_ptr = nullptr;
			_deleter = nullptr;
		}

		bool is_valid() const noexcept
		{
			return _is_object || _ptr;
		}

		bool has_references() const noexcept
		{
			return _ref_count != 0;
		}

		void set_references(const std::size_t count) noexcept
		{
			_ref_count = count;
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
		std::atomic_size_t _ref_count{};
		void (*_deleter)(void **) = nullptr;
		void *_ptr = nullptr;
		bool _is_object = false;
	};
}
