#pragma once

#include <atomic>

namespace gc
{
	class destroyer
	{
		template<class T>
		class manager
		{
			friend class destroyer;

			static void object_destroyer(void *p_void)
			{
				static_cast<T *>(p_void)->~T();
			}

			static void object_deleter(void *p_void)
			{
				auto ptr = static_cast<T *>(p_void);
				delete ptr;
			}

			static void array_deleter(void *p_void)
			{
				auto ptr = static_cast<T *>(p_void);
				delete[] ptr;
			}

		public:
			manager() = delete;
			manager(const manager &) = delete;
			manager(manager &&) = delete;
			~manager() = delete;
			manager &operator=(const manager &) = delete;
			manager &operator=(manager &&) = delete;
		};

		typedef void (*delete_func_type)(void *);

	public:
		enum class type : uint8_t
		{
			DESTROYER,
			DELETER,
			ARRAY_DELETER,
		};

		destroyer() noexcept = default;

	private:
		destroyer(void *ptr, const delete_func_type delete_func, void *allocation_ptr, const std::size_t allocation_size) noexcept
			: _ptr(ptr), _delete_func(delete_func), _allocation_ptr(allocation_ptr), _allocation_size(allocation_size)
		{
		}

	public:
		destroyer(const destroyer &) = delete;

		destroyer(destroyer &&rhs) noexcept
			: _ptr(rhs._ptr), _delete_func(rhs._delete_func), _allocation_ptr(rhs._allocation_ptr), _allocation_size(rhs._allocation_size)
		{
			rhs._ptr = nullptr;
			rhs._delete_func = nullptr;
			rhs._allocation_ptr = nullptr;
		}

		~destroyer() noexcept
		{
			destroy();
		}

		destroyer &operator=(const destroyer &) = delete;

		destroyer &operator=(destroyer &&rhs) noexcept
		{
			if (this != &rhs)
			{
				_ptr = rhs._ptr;
				_delete_func = rhs._delete_func;
				_allocation_ptr = rhs._allocation_ptr;
				_allocation_size = rhs._allocation_size;

				rhs._ptr = nullptr;
				rhs._delete_func = nullptr;
				rhs._allocation_ptr = nullptr;
			}

			return *this;
		}

		template <class T>
		static destroyer create_destroyer(void *ptr, const type deleter_type, void *allocation_ptr = nullptr, const std::size_t allocation_size = 0)
		{
			return destroyer(ptr, get_delete_function<T>(deleter_type), allocation_ptr, allocation_size);
		}

	private:
		void destroy() noexcept
		{
			const auto ptr = _ptr;
			_ptr = nullptr;
			const auto delete_func = _delete_func;
			_delete_func = nullptr;

			const auto allocation_ptr = _allocation_ptr;
			const auto allocation_size = _allocation_size;

			if (ptr == nullptr || delete_func == nullptr)
				return;

			try
			{
				if (allocation_ptr == nullptr)
					delete_func(ptr);
				else
				{
					delete_func(ptr);
					if (allocation_size)
						operator delete(allocation_ptr, allocation_size);
					else
						operator delete(allocation_ptr);
				}
			} catch (...)
			{
			}
		}

		template<class T>
		static delete_func_type get_delete_function(const type deleter_type) noexcept
		{
			switch (deleter_type)
			{
				case type::DESTROYER:
					return manager<T>::object_destroyer;
				case type::DELETER:
					return manager<T>::object_deleter;
				case type::ARRAY_DELETER:
					return manager<T>::array_deleter;
			}
			return nullptr;
		}

		/**
         * Pointer to be object that is to be destroyed
         */
		void *_ptr = nullptr;
		/**
		 * Function destroyer to a template function which knows how to free/destroy the object destroyer by the _ptr
		 */
		delete_func_type _delete_func = nullptr;
		/**
		 * Pointer to the memory that is to be freed
		 * May be null to indicate that _ptr should be freed directly
		 */
		void *_allocation_ptr = nullptr;
		std::size_t _allocation_size{};
	};
}
