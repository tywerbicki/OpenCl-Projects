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


namespace
{


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


cl_int CreateFromBinary(const cl_context context, cl_program& program)
{
	cl_int                    result  = CL_SUCCESS;
	std::vector<cl_device_id> devices = {};

	result = context::GetDevices(context, devices);
	OPENCL_RETURN_ON_ERROR(result);

	std::vector<std::vector<unsigned char>> clBinaries(devices.size());
	unsigned int nClBinariesAcquired = 0;

	for (size_t i = 0; i < devices.size(); i++)
	{
		std::filesystem::path clBinaryPath;
		std::error_code       ec;

		result = device::GetClBinaryPath(devices[i], clBinaryPath);
		OPENCL_RETURN_ON_ERROR(result);

		const bool clBinaryExists = std::filesystem::exists(clBinaryPath, ec);

		if (!ec && clBinaryExists)
		{
			std::ifstream clBinaryIfStream(
				clBinaryPath,
				std::ios_base::in | std::ios_base::binary | std::ios_base::ate
			);

			if (!clBinaryIfStream.is_open())
			{
				std::cerr << "Failed to open input stream for OpenCL program binary: "
						  << clBinaryPath
						  << "\n";
				break;
			}

			const auto clBinaryEndPos   = clBinaryIfStream.tellg();
			clBinaryIfStream.seekg(std::ios_base::beg);
			const auto clBinaryBeginPos = clBinaryIfStream.tellg();

			if (clBinaryIfStream.fail())
			{
				std::cerr << "Failed to determine size in bytes of OpenCL program binary: "
					      << clBinaryPath
						  << "\n";
				break;
			}

			clBinaries[i].resize(clBinaryEndPos - clBinaryBeginPos);

			clBinaryIfStream.read(
				reinterpret_cast<char*>(clBinaries[i].data()),
				clBinaries[i].size()
			);
			
			if (clBinaryIfStream.fail())
			{
				std::cerr << "Failed to read OpenCL program binary from file: "
					      << clBinaryPath
					      << "\n";
				break;
			}

			nClBinariesAcquired++;
		}
		else
		{
			break;
		}
	}

	if (nClBinariesAcquired == devices.size())
	{
		std::vector<const unsigned char*> clBinaryPtrs(    nClBinariesAcquired);
		std::vector<size_t>               clBinarySizes(   nClBinariesAcquired);
		std::vector<cl_int>               clBinaryStatuses(nClBinariesAcquired);

		for (size_t i = 0; i < nClBinariesAcquired; i++)
		{
			clBinaryPtrs[i]  = clBinaries[i].data();
			clBinarySizes[i] = clBinaries[i].size();
		}

		program = clCreateProgramWithBinary(
			context,
			static_cast<cl_uint>(devices.size()),
			devices.data(),
			clBinarySizes.data(),
			clBinaryPtrs.data(),
			clBinaryStatuses.data(),
			&result
		);

#ifdef _DEBUG
		if (result == CL_SUCCESS)
		{
			std::cout << "Successfully created program "
					  << program
					  << " from binary for context: "
				      << context
					  << "\n";
		}
		else
		{
			for (size_t i = 0; i < devices.size(); i++)
			{
				if (clBinaryStatuses[i] != CL_SUCCESS)
				{
					std::string  uniqueId    = {};
					const cl_int debugResult = device::QueryUniqueId(devices[i], uniqueId);
					OPENCL_RETURN_ON_ERROR(debugResult);

					std::cerr << "Error loading program binary for "
						      << uniqueId
							  << ": "
							  << clBinaryStatuses[i]
							  << "\n";
				}
			}
		}
#endif // _DEBUG

		if (result != CL_SUCCESS)
		{
			program = nullptr;
			result  = CL_SUCCESS;
		}
	}
	else
	{
		program = nullptr;
	}

	return result;
}


cl_int CreateFromSource(const cl_context context, cl_program& program)
{
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

	std::array<const char*, build::nClSourceFiles> clSourceCStrs = {};
	std::array<size_t, build::nClSourceFiles>      clSourceSizes = {};

	for (size_t i = 0; i < build::nClSourceFiles; i++)
	{
		clSourceCStrs[i] = clSource[i].c_str();
		clSourceSizes[i] = clSource[i].size();
	}

	cl_int result = CL_SUCCESS;

	program = clCreateProgramWithSource(
		context,
		static_cast<cl_uint>(clSourceCStrs.size()),
		clSourceCStrs.data(),
		clSourceSizes.data(),
		&result
	);

#ifdef _DEBUG

	if (result == CL_SUCCESS)
	{
		std::cout << "Successfully created program "
				  << program
				  << " from source for context: "
				  << context
				  << "\n";
	}
	else
	{
		debug::_OpenClDisplayError(result, __FILE__, __LINE__);
	}

#endif // _DEBUG

	return result;
}


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


cl_int StoreBinaries(const cl_program program)
{
	cl_int                    result  = CL_SUCCESS;
	std::vector<cl_device_id> devices = {};

	result = GetDevices(program, devices);
	OPENCL_RETURN_ON_ERROR(result);

	std::vector<std::vector<unsigned char>> clBinaries(   devices.size());
	std::vector<unsigned char*>             clBinaryPtrs( devices.size());
	std::vector<size_t>                     clBinarySizes(devices.size());

	result = clGetProgramInfo(
		program,
		CL_PROGRAM_BINARY_SIZES,
		clBinarySizes.size() * sizeof(size_t),
		clBinarySizes.data(),
		nullptr
	);
	OPENCL_RETURN_ON_ERROR(result);

	for (size_t i = 0; i < clBinaries.size(); i++)
	{
		clBinaries[i].resize(clBinarySizes[i]);
		clBinaryPtrs[i] = clBinaries[i].data();
	}

	result = clGetProgramInfo(
		program,
		CL_PROGRAM_BINARIES,
		clBinaryPtrs.size() * sizeof(unsigned char*),
		clBinaryPtrs.data(),
		nullptr
	);
	OPENCL_RETURN_ON_ERROR(result);

	unsigned int nClBinariesPersisted = 0;

	for (size_t i = 0; i < devices.size(); i++)
	{
		std::filesystem::path clBinaryDir;

		result = device::GetClBinaryDir(devices[i], clBinaryDir);
		OPENCL_RETURN_ON_ERROR(result);

		try
		{
			std::filesystem::create_directories(clBinaryDir);
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			std::cerr << "Failed to make directory "
					  << clBinaryDir
					  << " to store program binary: "
					  << e.what()
					  << " (error code: "
					  << e.code()
					  << ")\n";
			break;
		}

		const std::filesystem::path clBinaryPath = clBinaryDir / build::clBinariesFileName;

		std::ofstream clBinaryOfStream(
			clBinaryPath,
			std::ios_base::out | std::ios_base::binary | std::ios_base::trunc
		);

		if (!clBinaryOfStream.is_open())
		{
			std::cerr << "Failed to open output stream for OpenCL program binary: "
					  << clBinaryPath
					  << "\n";
			break;
		}

		clBinaryOfStream.write(
			reinterpret_cast<const char*>(clBinaries[i].data()),
			clBinaries[i].size()
		);

		if (clBinaryOfStream.fail())
		{
			std::cerr << "Failed to write OpenCL program binary to file: "
				      << clBinaryPath
					  << "\n";
			break;
		}

		nClBinariesPersisted++;
	}

#ifdef _DEBUG

	if (nClBinariesPersisted == clBinaries.size())
	{
		std::cout << "Successfully persisted all binaries for program: "
				  << program
				  << "\n";
	}

#endif // _DEBUG

	return result;
}


}


cl_int Build(const cl_context context, cl_program& program)
{
	cl_int result                   = CL_SUCCESS;
	bool   programCreatedFromBinary = true;

	result = CreateFromBinary(context, program);
	OPENCL_RETURN_ON_ERROR(result);

	if (!program)
	{
		result = CreateFromSource(context, program);
		OPENCL_RETURN_ON_ERROR(result);

		programCreatedFromBinary = false;
	}

	std::vector<cl_device_id> devices = {};
	result                            = context::GetDevices(context, devices);
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
#ifdef _DEBUG

		std::cout << "Successfully built program "
				  << program
				  << " for context: "
				  << context
				  << "\n";

#endif // _DEBUG

		if (!programCreatedFromBinary)
		{
			result = StoreBinaries(program);
			OPENCL_RETURN_ON_ERROR(result);
		}

		break;
	}

	default:
		OPENCL_PRINT_ON_ERROR(result);
		return result;

	}

	return result;
}


} // program