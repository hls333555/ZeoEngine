#include "Test/TestLayer.h"

#include <doctest.h>

namespace ZeoEngine {

	TestLayer::TestLayer()
		: Layer("Test")
	{
	}

	void TestLayer::OnAttach()
	{
		doctest::Context context;
		context.run();
	}

	TEST_CASE("Test")
	{

	}

}
