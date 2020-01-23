#include "RandomEngine.h"

std::mt19937 RandomEngine::s_RandomEngine;
std::uniform_real_distribution<float> RandomEngine::s_Distribution(0, 1);
