#pragma once

#include <CL/cl.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#include "build.h"
#include "debug.h"
#include "context.h"
#include "device.h"


namespace program
{


cl_int GetBuildLog(
	const cl_program   program,
	const cl_device_id device,
	std::string&       buildLog)
{
	cl_int result                = CL_SUCCESS;
	size_t paramValueSizeInBytes = 0;

	result = clGetProgramBuildInfo(
		program,
		device,
		CL_PROGRAM_BUILD_LOG,
		0,
		nullptr,
		&paramValueSizeInBytes
	);
	OPENCL_RETURN_ON_ERROR(result);

	std::vector<char> paramValue(paramValueSizeInBytes);

	result = clGetProgramBuildInfo(
		program,
		device,
		CL_PROGRAM_BUILD_LOG,
		paramValueSizeInBytes,
		paramValue.data(),
		nullptr
	);
	OPENCL_RETURN_ON_ERROR(result);

	buildLog.assign(
		paramValue.data(),
		paramValueSizeInBytes - 1
	);

	return result;
}


cl_int GetDevices(const cl_program program, std::vector<cl_device_id>& devices)
{
	cl_int  result   = CL_SUCCESS;
	cl_uint nDevices = 0;

	result = clGetProgramInfo(
		program,
		CL_PROGRAM_NUM_DEVICES,
		sizeof(nDevices),
		&nDevices,
		nullptr
	);
	OPENCL_RETURN_ON_ERROR(result);

	devices.resize(nDevices);

	result = clGetProgramInfo(
		program,
		CL_PROGRAM_DEVICES,
		devices.size() * sizeof(cl_device_id),
		devices.data(),
		nullptr
	);
	OPENCL_PRINT_ON_ERROR(result);

	return result;
}


cl_int StoreBinaries(const cl_program program)
{
	cl_int                    result  = CL_SUCCESS;
	std::vector<cl_device_id> devices = {};

	result = GetDevices(program, devices);
	OPENCL_RETURN_ON_ERROR(result);

	std::vector<size_t> binarySizes(devices.size());

	result = clGetProgramInfo(
		program,
		CL_PROGRAM_BINARY_SIZES,
		binarySizes.size() * sizeof(size_t),
		binarySizes.data(),
		nullptr
	);
	OPENCL_RETURN_ON_ERROR(result);

	std::vector<std::vector<char>> binaries(binarySizes.size());

	for (size_t i = 0; i < binaries.size(); i++)
	{
		binaries[i].resize(binarySizes[i]);
	}

	std::vector<char*> binaryRawPtrs(binaries.size());

	for (size_t i = 0; i < binaries.size(); i++)
	{
		binaryRawPtrs[i] = binaries[i].data();
	}

	result = clGetProgramInfo(
		program,
		CL_PROGRAM_BINARIES,
		binaryRawPtrs.size() * sizeof(char*),
		binaryRawPtrs.data(),
		nullptr
	);
	OPENCL_RETURN_ON_ERROR(result);

	for (size_t i = 0; i < devices.size(); i++)
	{
		std::filesystem::path clBinaryDir = {};
		result                            = device::GetClBinaryLoc(devices[i],
																   true,
																   clBinaryDir);
		OPENCL_BREAK_ON_ERROR(result);

		try
		{
			std::filesystem::create_directories(clBinaryDir);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to make directory "
					  << clBinaryDir
					  << " to store program binary: "
					  << e.what();
			break;
		}

		std::filesystem::path clBinaryPath;
		try
		{
			clBinaryPath = clBinaryDir / build::clBinariesFileName;
		}
		catch (const std::bad_alloc&)
		{
			result = CL_OUT_OF_HOST_MEMORY;
		}
		OPENCL_BREAK_ON_ERROR(result);

		std::ofstream clBinaryOfStream(
			clBinaryPath,
			std::ios_base::out | std::ios_base::binary | std::ios_base::trunc
		);

		if (!clBinaryOfStream.is_open())
		{
			std::cerr << "Failed to open program binary output stream: "
					  << clBinaryPath
					  << "\n";
			break;
		}

		clBinaryOfStream.write(binaries[i].data(), binaries[i].size());

		if (clBinaryOfStream.fail())
		{
			std::cerr << "Failed to write program binary to storage: "
				      << clBinaryPath
					  << "\n";
			break;
		}
	}

	return result;
}


cl_int Build(const cl_context context, cl_program& program)
{
	cl_int                    result  = CL_SUCCESS;
	std::vector<cl_device_id> devices = {};

	result = context::GetDevices(context, devices);
	OPENCL_RETURN_ON_ERROR(result);

	// Check if we already have binaries for the program.
	// TODO.
	std::vector<std::string> clBinaries(devices.size());

	for (size_t i = 0; i < devices.size(); i++)
	{
		std::filesystem::path clBinaryPath;
		std::error_code       ec;

		result = device::GetClBinaryLoc(
			devices[i],
			false,
			clBinaryPath
		);
		OPENCL_RETURN_ON_ERROR(result);

		const bool clBinaryExists = std::filesystem::exists(clBinaryPath, ec);

		if (!ec && clBinaryExists)
		{
			// Read it into vector of strings.
		}
		else
		{
			// Indicate we need to compile from source and break the loop.
		}
	}

	std::array<std::string, build::nClSourceFiles> clSource;

	for (size_t i = 0; i < build::nClSourceFiles; i++)
	{
		std::ifstream clSourceIfStream(build::clSourceRoot / build::clSourceFileNames[i]);

		if (!clSourceIfStream.is_open())
		{
			std::cerr << "Failed to open OpenCL source text input stream for source file: "
					  << build::clSourceFileNames[i]
					  << "\n";
			return CL_BUILD_PROGRAM_FAILURE;
		}

		std::ostringstream clSourceOsStream;
		clSourceOsStream << clSourceIfStream.rdbuf();

		if (clSourceOsStream.fail())
		{
			std::cerr << "Failed to read OpenCL source text for source file: "
					  << build::clSourceFileNames[i]
					  << "\n";
			return CL_BUILD_PROGRAM_FAILURE;
		}

		clSource[i] = std::move(clSourceOsStream.str());
	}

	const char* clSourceCStrs[build::nClSourceFiles];
	size_t      clSourceSizes[build::nClSourceFiles];

	for (size_t i = 0; i < build::nClSourceFiles; i++)
	{
		clSourceCStrs[i] = clSource[i].c_str();
		clSourceSizes[i] = clSource[i].size();
	}

	program = clCreateProgramWithSource(
		context,
		build::nClSourceFiles,
		clSourceCStrs,
		clSourceSizes,
		&result
	);
	OPENCL_RETURN_ON_ERROR(result);

	result = clBuildProgram(
		program,
		static_cast<cl_uint>(devices.size()),
		devices.data(),
		build::options.c_str(),
		nullptr,
		nullptr
	);

	switch (result)
	{
	case CL_BUILD_PROGRAM_FAILURE:
	{
		for (const auto device : devices)
		{
			cl_build_status buildStatus = CL_BUILD_NONE;

			result = clGetProgramBuildInfo(
				program,
				device,
				CL_PROGRAM_BUILD_STATUS,
				sizeof(buildStatus),
				&buildStatus,
				nullptr
			);
			OPENCL_RETURN_ON_ERROR(result);

			if (buildStatus != CL_BUILD_SUCCESS)
			{
				std::string buildLog = {};
				result               = GetBuildLog(program, device, buildLog);
				OPENCL_RETURN_ON_ERROR(result);

				std::cout << buildLog;
			}
		}

		return CL_BUILD_PROGRAM_FAILURE;
	}

	case CL_SUCCESS:
	{
		result = StoreBinaries(program);
		OPENCL_RETURN_ON_ERROR(result);

		break;
	}

	default:
		OPENCL_PRINT_ON_ERROR(result);
		return result;

	}

	return result;
}


} // program