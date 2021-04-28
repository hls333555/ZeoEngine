#include "Test/TestLayer.h"

#include <doctest.h>

#include "Test/TestComponent.h"
#include "Reflection/DataWidget.h"

namespace ZeoEngine {

	TestLayer::TestLayer()
		: EngineLayer("Test")
	{
	}

	void TestLayer::OnAttach()
	{
		EngineLayer::OnAttach();

		doctest::Context context;
		int doctestRes = context.run();
	}

	TEST_CASE("Test data widget reflection")
	{
		entt::registry reg;
		auto entity = reg.create();
		auto& testComp = reg.emplace<TestComponent>(entity);
		entt::meta_any compInstance{ std::ref(testComp) }; // NOTE: We must use std::ref() to create aliases for unmanaged objects
		const auto compType = compInstance.type();
		std::deque<entt::meta_data> datas;
		for (const auto data : compType.data())
		{
			datas.push_front(data);
		}
		for (const auto data : datas)
		{
			DataSpec dataSpec{ data, compInstance, compInstance, false, false };
			std::vector<DataStackSpec> dataStack;
			ConstructBasicDataWidget(dataSpec, data.type(), true)->Test(reg, entity, dataStack);
		}
	}

}
