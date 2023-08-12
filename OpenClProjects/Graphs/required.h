#pragma once

#include <CL/cl.h>

#include <string>


namespace required
{


// Platform requirements.
inline extern const std::string PlatformProfile  = "FULL_PROFILE";

inline extern const std::string PlatformNameNvda = "NVIDIA CUDA";
inline extern const std::string PlatformNameAmd  = "AMD ROCM";


// Device requirements.
inline extern const cl_device_type DeviceTypeGpu = CL_DEVICE_TYPE_GPU;


} // required