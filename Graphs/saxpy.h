#pragma once

#include <CL/cl.h>

#include <span>


namespace saxpy
{

	cl_int Execute(float                  a,
				   std::span<const float> x,
				   std::span<float>       y);

}