#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "Engine/Core/Core.h"
#include "Engine/Core/Log.h"
#include "Engine/Profile/Instrumentor.h"

#ifdef ZE_PLATFORM_WINDOWS
	#include <Windows.h>
#endif // ZE_PLATFORM_WINDOWS
