#pragma once

#include <string>
#include <unordered_set>


namespace require
{


inline extern const std::string requiredPlatformProfile = "FULL_PROFILE";


inline extern const std::unordered_set<std::string> requiredPlatformNames =
{
    "NVIDIA CUDA",
    "AMD ROCM"
};


} // require