#pragma once

#include <CL\cl.h>

#include <vector>

#include "platform.h"


namespace context
{
	cl_int GetDevices(const cl_context           context,
					  std::vector<cl_device_id>& devices);

	cl_int Create(platform::SelectionStrategy strategy,
				  cl_context&                 context);

}