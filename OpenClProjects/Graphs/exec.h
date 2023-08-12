#pragma once

#include <CL/cl.h>

#include <iostream>
#include <vector>

#include "debug.h"
#include "device.h"
#include "platform.h"


namespace exec
{


struct ExecResources
{
	cl_platform_id            platform;
	std::vector<cl_device_id> devices;
};


cl_int GetResources(std::vector<ExecResources>& resources)
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
        result = platform::IsConformant(platform, platformIsConformant);
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
                result = device::IsConformant(device, deviceIsConformant);
                OPENCL_RETURN_ON_ERROR(result);

                if (deviceIsConformant)
                {
                    conformantDevices.push_back(device);
                }
            }

            if (conformantDevices.size() > 0)
            {
                resources.emplace_back(platform, std::move(conformantDevices));
            }
        }
    }

    return result;
}


} // exec