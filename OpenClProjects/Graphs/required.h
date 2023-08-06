#pragma once

#include <string>
#include <unordered_set>


namespace required
{


inline extern const std::string PlatformProfile = "FULL_PROFILE";


inline extern const std::unordered_set<std::string> PlatformNames =
{
    "NVIDIA CUDA",
    "AMD ROCM"
};


} // required