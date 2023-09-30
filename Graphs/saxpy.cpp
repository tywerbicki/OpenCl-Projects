#include <array>
#include <cassert>
#include <stdint.h>

#include "debug.h"
#include "saxpy.h"


namespace
{

	enum saxpyHostToDeviceWrites : uint32_t
	{
		x = 0,
		y,
		count,
	};

}


cl_int saxpy::HostPreExecute(const cl_context             context,
							 const cl_command_queue       commandQueue,
							 const cl_kernel              saxpyKernel,
							 const float                  a,
							 const std::span<const float> x,
							 const std::span<float>       y,
							 cl_event&                    event)
{
	assert(x.size() == y.size());

	cl_int                                               result             = CL_SUCCESS;
	std::array<cl_event, saxpyHostToDeviceWrites::count> hostToDeviceWrites = {};
	cl_event                                             deviceExecute      = nullptr;

	cl_mem xDeviceBuffer = clCreateBuffer(context,
										  CL_MEM_READ_ONLY,
										  x.size_bytes(),
										  nullptr,
										  &result);

	OPENCL_RETURN_ON_ERROR(result);

	result = clEnqueueWriteBuffer(commandQueue,
								  xDeviceBuffer,
								  CL_FALSE,
								  0,
								  x.size_bytes(),
								  x.data(),
								  0,
								  nullptr,
								  &hostToDeviceWrites[saxpyHostToDeviceWrites::x]);

	OPENCL_RETURN_ON_ERROR(result);

	cl_mem yDeviceBuffer = clCreateBuffer(context,
										  CL_MEM_READ_WRITE,
									      y.size_bytes(),
									      nullptr,
										  &result);

	OPENCL_RETURN_ON_ERROR(result);

	result = clEnqueueWriteBuffer(commandQueue,
								  yDeviceBuffer,
								  CL_FALSE,
								  0,
								  y.size_bytes(),
								  y.data(),
								  0,
								  nullptr,
								  &hostToDeviceWrites[saxpyHostToDeviceWrites::y]);

	OPENCL_RETURN_ON_ERROR(result);

	result = clSetKernelArg(saxpyKernel,
							0,
							sizeof(float),
							&a);

	OPENCL_RETURN_ON_ERROR(result);

	result = clSetKernelArg(saxpyKernel,
							1,
							sizeof(cl_mem),
							&xDeviceBuffer);

	OPENCL_RETURN_ON_ERROR(result);

	result = clSetKernelArg(saxpyKernel,
							2,
							sizeof(cl_mem),
							&yDeviceBuffer);

	OPENCL_RETURN_ON_ERROR(result);

	result = DeviceExecute(commandQueue,
						   saxpyKernel,
						   hostToDeviceWrites,
						   deviceExecute);

	OPENCL_RETURN_ON_ERROR(result);

	result = clEnqueueReadBuffer(commandQueue,
								 yDeviceBuffer,
								 CL_FALSE,
								 0,
								 y.size_bytes(),
								 y.data(),
								 1,
								 &deviceExecute,
								 &event);

	OPENCL_PRINT_ON_ERROR(result);
	return result;
}


cl_int saxpy::DeviceExecute(const cl_command_queue			commandQueue,
						    const cl_kernel					saxpyKernel,
						    const std::span<const cl_event> eventsToWaitOn,
						    cl_event&						saxpyEvent)
{
	cl_int result = CL_SUCCESS;

	// TODO: select desirable work group sizes.
	const size_t globalWorkSize = 1024;
	const size_t localWorkSize  = 32;

	result = clEnqueueNDRangeKernel(commandQueue,
									saxpyKernel,
									1,
									nullptr,
									&globalWorkSize,
									&localWorkSize,
									static_cast<cl_uint>(eventsToWaitOn.size()),
									eventsToWaitOn.data(),
									&saxpyEvent);

	OPENCL_PRINT_ON_ERROR(result);
	return result;
}