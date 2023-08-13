#pragma once

#include <CL/cl.h>

#include <array>
#include <fstream>
#include <string>
#include <vector>

#include "debug.h"


namespace program
{


cl_int Build(const cl_context context)
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

	std::vector<cl_device_id> devices{ nDevices };

	// Query the devices associated with the context.
	result = clGetContextInfo(
		context,
		CL_CONTEXT_DEVICES,
		devices.size() * sizeof(decltype(devices)::value_type),
		devices.data(),
		nullptr
	);
	OPENCL_RETURN_ON_ERROR(result);

	const std::array<const std::string, 1> kernelPaths
	{
		"vfadd.cl"
	};

	std::array<const char*, kernelPaths.size()> kernelSource;

	for (const auto kernelPath : kernelPaths)
	{

	}

	return result;
}


} // program