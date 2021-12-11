#pragma once

namespace ZeoEngine {

	class Bindable
	{
	public:
		virtual ~Bindable() = default;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void Unbind() const {}
	};

}
