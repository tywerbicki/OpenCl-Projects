#pragma once

#include <CL/cl.h>

#include <array>
#include <vector>

#include "debug.h"
#include "device.h"
#include "platform.h"


namespace context
{


cl_int GetDevices(const cl_context context, std::vector<cl_device_id>& devices)
{
    cl_int  result   = CL_SUCCESS;
	cl_uint nDevices = 0;

    // Query number of devices associated with the context.
	result = clGetContextInfo(
		context,
		CL_CONTEXT_NUM_DEVICES,
		sizeof(nDevices),
		&nDevices,
		nullptr
	);
	OPENCL_RETURN_ON_ERROR(result);

    devices.resize(nDevices);

    // Query the devices associated with the context.
	result = clGetContextInfo(
		context,
		CL_CONTEXT_DEVICES,
		devices.size() * sizeof(cl_device_id),
		devices.data(),
		nullptr
	);
	OPENCL_PRINT_ON_ERROR(result);

    return result;
}


cl_int GetAllAvailable(std::vector<cl_context>& contexts)
{
	cl_int                      result             = CL_SUCCESS;
	std::vector<cl_platform_id> availablePlatforms = {};

	result = platform::GetAllAvailable(availablePlatforms);
    OPENCL_RETURN_ON_ERROR(result);

    for (const auto platform : availablePlatforms)
    {
        #ifdef _DEBUG

        result = platform::DisplayInfo(platform);
        OPENCL_RETURN_ON_ERROR(result);

        #endif // _DEBUG

        bool platformIsConformant = false;
        result                    = platform::IsConformant(platform, platformIsConformant);
        OPENCL_RETURN_ON_ERROR(result);

        if (platformIsConformant)
        {
            std::vector<cl_device_id> availableDevices  = {};
            std::vector<cl_device_id> conformantDevices = {};

            result = device::GetAllAvailable(platform, availableDevices);
            OPENCL_RETURN_ON_ERROR(result);

            for (const auto device : availableDevices)
            {
                #ifdef _DEBUG

                result = device::DisplayGeneralInfo(device);
                OPENCL_RETURN_ON_ERROR(result);

                #endif // _DEBUG

                bool deviceIsConformant = false;
                result                  = device::IsConformant(device, deviceIsConformant);
                OPENCL_RETURN_ON_ERROR(result);

                if (deviceIsConformant)
                {
                    conformantDevices.push_back(device);
                }
            }

            if (conformantDevices.size() > 0)
            {
                const std::array<const cl_context_properties, 3> properties{
                    CL_CONTEXT_PLATFORM,
                    reinterpret_cast<cl_context_properties>(platform),
                    0
                };

                const cl_context context = clCreateContext(
                    properties.data(),
                    static_cast<cl_uint>(conformantDevices.size()),
                    conformantDevices.data(),
                    nullptr,
                    nullptr,
                    &result
                );
                OPENCL_RETURN_ON_ERROR(result);

                contexts.push_back(context);
            }
        }
    }

    return result;
}


} // context