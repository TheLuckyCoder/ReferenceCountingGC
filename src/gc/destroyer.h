#pragma once

#include <cstdint>
#include <utility>

namespace gc
{
	struct abstract_control_block
	{
		virtual ~abstract_control_block() = default;
	};

	template <class T, class C>
	struct control_block_object final : abstract_control_block
	{
		T _obj;
		C _counter{ 1 };

		template <typename ... Args>
		explicit control_block_object(Args &&... args) : _obj(std::forward<Args>(args)...) {}
	};

	template <class T, class C>
	struct control_block_ptr final : abstract_control_block
	{
		T *_ptr;
		C _counter{ 1 };

		explicit control_block_ptr(T *ptr) : _ptr(ptr) {}

		~control_block_ptr() override
		{
			delete _ptr;
		}
	};

	template <class T, class C>
	struct control_block_array final : abstract_control_block
	{
		T *_ptr;
		C _counter{ 1 };

		explicit control_block_array(const std::size_t size) : _ptr(operator new[](sizeof(T) * size)) {}

		explicit control_block_array(T *ptr) : _ptr(ptr) {}

		~control_block_array() override
		{
			delete[] _ptr;
		}
	};

	class destroyer
	{
	public:

		destroyer() noexcept = default;

		explicit destroyer(abstract_control_block *ptr) noexcept
			: _ptr(ptr)
		{
		}

		destroyer(const destroyer &) = delete;

		destroyer(destroyer &&rhs) noexcept
			: _ptr(rhs._ptr)
		{
			rhs._ptr = nullptr;
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

				rhs._ptr = nullptr;
			}

			return *this;
		}

	private:
		void destroy() noexcept
		{
			abstract_control_block *ptr = _ptr;
			_ptr = nullptr;

			if (ptr == nullptr) return;

			try
			{
				delete ptr;
			} catch (...)
			{
			}
		}

		abstract_control_block *_ptr = nullptr;
	};
}
