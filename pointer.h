#pragma once

#include <atomic>

namespace gc
{
	class pointer
	{
	    template <class T>
	    friend class ref;
        template<class T>
	    friend class ref_array;

		template <typename T>
		struct manager
		{
			static void object_destroyer(void *p_void)
			{
				reinterpret_cast<T*>(p_void)->~T();
			}

			static void object_deleter(void *p_void)
			{
				auto ptr = static_cast<T*>(p_void);
				delete ptr;
			}

			static void array_deleter(void *p_void)
			{
				auto ptr = static_cast<T*>(p_void);
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
		typedef void (*delete_func_type)(void *);

		pointer() noexcept = default;

		pointer(void *ptr, void *base_ptr, delete_func_type delete_func) noexcept
			: _ptr(ptr), _base_ptr(base_ptr), _delete_func(delete_func)
		{
		}

		pointer(const pointer &) = delete;
		pointer(pointer &&other) noexcept
			: _ptr(other._ptr), _base_ptr(other._base_ptr), _delete_func(other._delete_func)
		{
			other._ptr = nullptr;
			other._base_ptr = nullptr;
			other._delete_func = nullptr;
		}

		~pointer() noexcept
		{
			destroy();
		}

		pointer &operator=(const pointer &) = delete;
		pointer &operator=(pointer &&rhs) noexcept
		{
			if (this != &rhs)
			{
				_ptr = rhs._ptr;
				_base_ptr = rhs._base_ptr;
				_delete_func = rhs._delete_func;

				rhs._ptr = nullptr;
				rhs._base_ptr = nullptr;
				rhs._delete_func = nullptr;
			}

			return *this;
		}

		void destroy() noexcept
		{
			const auto ptr = _ptr;
			const auto base_ptr = _base_ptr;
            const auto delete_func = _delete_func;

			_ptr = nullptr;
			_base_ptr = nullptr;
			_delete_func = nullptr;

		    if (!ptr && !delete_func)
                return;

            try
            {
                if (ptr == base_ptr || base_ptr == nullptr)
                    delete_func(ptr);
                else
                {
                    delete_func(ptr);
                    operator delete(base_ptr);
                }
            } catch (...)
            {
            }
		}

	private:
		/**
         * Pointer to be object that is to be destroyed
         */
        void *_ptr = nullptr;
		/**
		 * Pointer to the memory that is to be freed
		 * May be the same as _ptr
		 */
		void *_base_ptr = nullptr;
        /**
         * Function pointer to a templated function which knows how to free/destroy the object pointer by the _ptr
         */
        delete_func_type _delete_func = nullptr;
	};
}
