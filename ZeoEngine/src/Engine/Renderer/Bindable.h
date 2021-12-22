#pragma once

namespace ZeoEngine {

	class Bindable
	{
	public:
		virtual ~Bindable() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const {}
	};

}
