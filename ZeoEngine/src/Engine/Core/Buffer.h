#pragma once

namespace ZeoEngine {

	/** Non-owning raw buffer class */
	struct Buffer
	{
		U8* Data = nullptr;
		U64 Size = 0;

		Buffer() = default;
		Buffer(U64 size)
		{
			Allocate(size);
		}
		Buffer(const Buffer&) = default;
		Buffer(Buffer&& other) noexcept
			: Data(other.Data)
			, Size(other.Size)
		{
			other.Data = nullptr;
			other.Size = 0;
		}
		~Buffer() = default;
		Buffer& operator=(const Buffer&) = default;
		Buffer& operator=(Buffer&& other) noexcept
		{
			if (&other != this)
			{
				Release();

				Data = other.Data;
				Size = other.Size;
				other.Data = nullptr;
				other.Size = 0;
			}
			return *this;
		}

		static Buffer Copy(Buffer other)
		{
			const Buffer result(other.Size);
			memcpy(result.Data, other.Data, other.Size);
			return result;
		}

		void Allocate(U64 size)
		{
			Release();
			Data = new U8[size];
			Size = size;
		}

		void Release()
		{
			delete[] Data;
			Data = nullptr;
			Size = 0;
		}

		template<typename T>
		T* As() const
		{
			return reinterpret_cast<T*>(Data);
		}

		operator bool() const { return Data; }
	};

	struct ScopedBuffer
	{
		ScopedBuffer(Buffer buffer)
			: m_Buffer(std::move(buffer))
		{
		}

		ScopedBuffer(U64 size)
			: m_Buffer(size)
		{
		}

		~ScopedBuffer()
		{
			m_Buffer.Release();
		}

		U8* Data() const { return m_Buffer.Data; }
		U64 Size() const { return m_Buffer.Size; }

		template<typename T>
		T* As() const
		{
			return m_Buffer.As<T>();
		}

		operator bool() const { return m_Buffer; }

	private:
		Buffer m_Buffer;
	};
	
}
