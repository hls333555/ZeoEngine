#pragma once

namespace ZeoEngine {

	class BufferResource
	{
	public:
		virtual ~BufferResource() = default;

		virtual void BindAsBuffer() const = 0;
		virtual void UnbindAsBuffer() const = 0;
	};

}
