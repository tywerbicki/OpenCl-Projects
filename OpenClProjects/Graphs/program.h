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

#include "debug.h"
#include "context.h"
#include "device.h"


namespace program
{


const std::filesystem::path clSourceRoot   = "OpenCL Source";
const std::filesystem::path clBinariesRoot = "OpenCL Binaries";

constexpr size_t nClSourceFiles = 1;

const std::array<const std::string, nClSourceFiles> clSourceFileNames
{
	"vfadd.cl"
};

// const std::string kernelBinarySuffix = ".clbin";

#ifdef _DEBUG

const std::string buildOptions       = "-cl-opt-disable -Werror -cl-std=CL2.0 -g";
const std::string clBinariesFileName = "OpenClBinaries_Debug.cl.bin";

#else

const std::string buildOptions       = "-Werror -cl-std=CL2.0";
const std::string clBinariesFileName = "ProgramBinaries_Release.cl.bin";

#endif // _DEBUG


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

	std::vector<std::vector<unsigned char>> binaries(binarySizes.size());

	for (size_t i = 0; i < binaries.size(); i++)
	{
		binaries[i].resize(binarySizes[i]);
	}

	std::vector<unsigned char*> binaryRawPtrs(binaries.size());

	for (size_t i = 0; i < binaries.size(); i++)
	{
		binaryRawPtrs[i] = binaries[i].data();
	}

	result = clGetProgramInfo(
		program,
		CL_PROGRAM_BINARIES,
		binaryRawPtrs.size() * sizeof(unsigned char*),
		binaryRawPtrs.data(),
		nullptr
	);
	OPENCL_RETURN_ON_ERROR(result);

	for (size_t i = 0; i < devices.size(); i++)
	{
		std::string deviceVendor = {};
		result                   = device::QueryParamValue(devices[i],
														   CL_DEVICE_VENDOR,
														   deviceVendor);
		OPENCL_RETURN_ON_ERROR(result);

		// Pretend that we generated a unique hash for the device.
		// This could be done by hashing all of the device information.
		const std::string deviceUniqueId = "QuadroP1000";

		const auto clBinaryDir = clBinariesRoot / deviceVendor / deviceUniqueId;

		std::error_code ec;
		if (!std::filesystem::exists(clBinaryDir, ec) && !ec)
		{
			std::filesystem::create_directories(clBinaryDir, ec);
		}

		if (ec)
		{
			std::cerr << ec.message();
			return CL_SUCCESS;
		}

		std::ofstream clBinaryOfStream(
			clBinaryDir / clBinariesFileName,
			std::ios_base::out | std::ios_base::binary | std::ios_base::trunc
		);

		if (!clBinaryOfStream.is_open())
		{
			std::cerr << "Failed to open program binary output stream for device: "
					  << deviceUniqueId
					  << "\n";
			continue;
		}

		// TODO: handle exceptions.
		clBinaryOfStream << binaries[i].data();
	}

	return result;
}


cl_int Build(const cl_context context, cl_program& program)
{
	// Check if we already have a binary for the program.
	// TODO.

	std::array<std::string, nClSourceFiles> clSource;

	for (size_t i = 0; i < nClSourceFiles; i++)
	{
		std::ifstream clSourceIfStream(clSourceRoot / clSourceFileNames[i]);

		if (!clSourceIfStream.is_open())
		{
			std::cerr << "Failed to open OpenCL source text input stream for source file: "
					  << clSourceFileNames[i]
					  << "\n";
			return CL_BUILD_PROGRAM_FAILURE;
		}

		std::ostringstream clSourceOsStream;
		// TODO: handle exceptions.
		clSourceOsStream << clSourceIfStream.rdbuf();

		clSource[i] = std::move(clSourceOsStream.str());
	}

	const char* clSourceCStrs[nClSourceFiles];
	size_t      clSourceSizes[nClSourceFiles];

	for (size_t i = 0; i < nClSourceFiles; i++)
	{
		clSourceCStrs[i] = clSource[i].c_str();
		clSourceSizes[i] = clSource[i].size();
	}

	cl_int result = CL_SUCCESS;

	program = clCreateProgramWithSource(
		context,
		nClSourceFiles,
		clSourceCStrs,
		clSourceSizes,
		&result
	);
	OPENCL_RETURN_ON_ERROR(result);

	std::vector<cl_device_id> devices = {};
	result                            = context::GetDevices(context, devices);
	OPENCL_RETURN_ON_ERROR(result);

	result = clBuildProgram(
		program,
		static_cast<cl_uint>(devices.size()),
		devices.data(),
		buildOptions.c_str(),
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