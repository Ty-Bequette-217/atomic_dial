#include "util.h"

float lerp(const float value, const float inMin, const float inMax, const float min, const float max) {
    return ((value - inMin) / (inMax - inMin)) * (max - min) + min;
}
