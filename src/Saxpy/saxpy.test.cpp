#include "build.h"
#include "context.h"
#include "platform.h"
#include "program.h"
#include "saxpy.h"

#include <CL/cl.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <optional>
#include <stdlib.h>
#include <vector>


enum saxpyHostToDeviceResolve : unsigned int
{
    x = 0,
    y,
    count,
};


struct KernelArg
{
    cl_uint     index;
    size_t      sizeInBytes;
    const void* pValue;
};


class SaxpyTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        cl_int                        result   = CL_SUCCESS;
        std::optional<cl_platform_id> platform = std::nullopt;
        std::optional<cl_context>     context  = std::nullopt;

        result = context::Create(platform::MostGpus, platform, context);
        ASSERT_EQ(result, CL_SUCCESS);

        if (context.has_value())
        {
            s_context = context.value();
        }
        else
        {
            GTEST_SKIP() << "No OpenCL context was created.";
        }

        result = program::Build(s_context,
                                std::cref(build::saxpy::binaryCreator),
                                build::saxpy::sourceCreator,
                                build::saxpy::options,
                                s_saxpyProgram);

        ASSERT_EQ(result, CL_SUCCESS);

        result = clUnloadPlatformCompiler(platform.value());
        ASSERT_EQ(result, CL_SUCCESS);
    }

    void SetUp() override final
    {
        cl_int                                                    result  = CL_SUCCESS;
        std::array<cl_kernel, build::saxpy::clKernelNames.size()> kernels = {};
        std::vector<cl_device_id>                                 devices = {};

        result = program::CreateKernels(s_saxpyProgram,
                                        build::saxpy::clKernelNames,
                                        kernels);

        ASSERT_EQ(result, CL_SUCCESS);

        result = context::GetDevices(s_context, devices);
        ASSERT_EQ(result, CL_SUCCESS);

        m_saxpyKernel                  = kernels[0];
        const cl_device_id saxpyDevice = devices[0];

        const std::array<const cl_queue_properties, 3> queueProperties
        {
            CL_QUEUE_PROPERTIES,
            CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
            0
        };

        m_saxpyQueue = clCreateCommandQueueWithProperties(s_context,
                                                          saxpyDevice,
                                                          queueProperties.data(),
                                                          &result);

        ASSERT_EQ(result, CL_SUCCESS);
    }

    static void TearDownTestSuite() noexcept
    {

    }

    void TearDown() noexcept override final
    {

    }

    static float GetRandFloat() noexcept
    {
        return static_cast<float>(std::rand());
    }

    cl_kernel        m_saxpyKernel   = nullptr;
    cl_command_queue m_saxpyQueue    = nullptr;
    cl_event         m_saxpyComplete = nullptr;

    static cl_context s_context;
    static cl_program s_saxpyProgram;

    static const std::array<size_t, 6> ProblemSizes;
    static const float                 A;
};

cl_context SaxpyTest::s_context      = nullptr;
cl_program SaxpyTest::s_saxpyProgram = nullptr;

const std::array<size_t, 6> SaxpyTest::ProblemSizes =
{
    1, 33, 1024, (1024 + 1), (1024 + 31), 1000000
};

const float SaxpyTest::A = 2.75;


TEST_F(SaxpyTest, UsingMappedHostMemory)
{
    for (const size_t problemSize : ProblemSizes)
    {
        cl_int       result             = CL_SUCCESS;
        const size_t problemSizeInBytes = problemSize * sizeof(float);

        std::array<cl_event, saxpyHostToDeviceResolve::count> hostToDeviceResolves = {};
        cl_event                                              zDeviceToHostResolve = nullptr;

        const cl_mem xDevice = clCreateBuffer(s_context,
                                              CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                              problemSizeInBytes,
                                              nullptr,
                                              &result);

        ASSERT_EQ(result, CL_SUCCESS);

        {
            float* const xDeviceMappedForWrite = static_cast<float*>(
                clEnqueueMapBuffer(m_saxpyQueue,
                                   xDevice,
                                   CL_TRUE,
                                   CL_MAP_WRITE_INVALIDATE_REGION,
                                   0,
                                   problemSizeInBytes,
                                   0,
                                   nullptr,
                                   nullptr,
                                   &result)
            );

            ASSERT_EQ(result, CL_SUCCESS);

            std::generate(xDeviceMappedForWrite,
                          xDeviceMappedForWrite + problemSize,
                          GetRandFloat);

            result = clEnqueueUnmapMemObject(m_saxpyQueue,
                                             xDevice,
                                             xDeviceMappedForWrite,
                                             0,
                                             nullptr,
                                             &hostToDeviceResolves[saxpyHostToDeviceResolve::x]);

            ASSERT_EQ(result, CL_SUCCESS);
        }

        const cl_mem yDevice = clCreateBuffer(s_context,
                                              CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                              problemSizeInBytes,
                                              nullptr,
                                              &result);

        ASSERT_EQ(result, CL_SUCCESS);

        {
            float* const yDeviceMappedForWrite = static_cast<float*>(
                clEnqueueMapBuffer(m_saxpyQueue,
                                   yDevice,
                                   CL_TRUE,
                                   CL_MAP_WRITE_INVALIDATE_REGION,
                                   0,
                                   problemSizeInBytes,
                                   0,
                                   nullptr,
                                   nullptr,
                                   &result)
            );

            ASSERT_EQ(result, CL_SUCCESS);

            std::generate(yDeviceMappedForWrite,
                          yDeviceMappedForWrite + problemSize,
                          GetRandFloat);

            result = clEnqueueUnmapMemObject(m_saxpyQueue,
                                             yDevice,
                                             yDeviceMappedForWrite,
                                             0,
                                             nullptr,
                                             &hostToDeviceResolves[saxpyHostToDeviceResolve::y]);

            ASSERT_EQ(result, CL_SUCCESS);
        }

        const cl_mem zDevice = clCreateBuffer(s_context,
                                              CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_HOST_READ_ONLY,
                                              problemSizeInBytes,
                                              nullptr,
                                              &result);

        ASSERT_EQ(result, CL_SUCCESS);

        {
            const std::array<KernelArg, 5> saxpyKernelArgs =
            { {
                    { .index = 0, .sizeInBytes = sizeof(A) ,          .pValue = &A          },
                    { .index = 1, .sizeInBytes = sizeof(xDevice),     .pValue = &xDevice    },
                    { .index = 2, .sizeInBytes = sizeof(yDevice),     .pValue = &yDevice    },
                    { .index = 3, .sizeInBytes = sizeof(zDevice),     .pValue = &zDevice    },
                    { .index = 4, .sizeInBytes = sizeof(problemSize), .pValue = &problemSize}
            } };

            for (const KernelArg& arg : saxpyKernelArgs)
            {
                result = clSetKernelArg(m_saxpyKernel,
                                        arg.index,
                                        arg.sizeInBytes,
                                        arg.pValue);

                ASSERT_EQ(result, CL_SUCCESS);
            }
        }

        result = saxpy::DeviceExecute(m_saxpyQueue,
                                      m_saxpyKernel,
                                      hostToDeviceResolves,
                                      m_saxpyComplete);

        ASSERT_EQ(result, CL_SUCCESS);

        {
            float* const zDeviceMappedForRead = static_cast<float*>(
                clEnqueueMapBuffer(m_saxpyQueue,
                                   zDevice,
                                   CL_FALSE,
                                   CL_MAP_READ,
                                   0,
                                   problemSizeInBytes,
                                   1,
                                   &m_saxpyComplete,
                                   &zDeviceToHostResolve,
                                   &result)
            );

            ASSERT_EQ(result, CL_SUCCESS);

            float* const xDeviceMappedForRead = static_cast<float*>(
                clEnqueueMapBuffer(m_saxpyQueue,
                                   xDevice,
                                   CL_TRUE,
                                   CL_MAP_READ,
                                   0,
                                   problemSizeInBytes,
                                   0,
                                   nullptr,
                                   nullptr,
                                   &result)
            );

            ASSERT_EQ(result, CL_SUCCESS);

            float* const yDeviceMappedForRead = static_cast<float*>(
                clEnqueueMapBuffer(m_saxpyQueue,
                                   yDevice,
                                   CL_TRUE,
                                   CL_MAP_READ,
                                   0,
                                   problemSizeInBytes,
                                   0,
                                   nullptr,
                                   nullptr,
                                   &result)
            );

            ASSERT_EQ(result, CL_SUCCESS);

            std::vector<float> solution(problemSize);

            saxpy::HostExecute(A,
                               xDeviceMappedForRead,
                               yDeviceMappedForRead,
                               solution.data(),
                               solution.size());

            result = clWaitForEvents(1, &zDeviceToHostResolve);
            ASSERT_EQ(result, CL_SUCCESS);

            EXPECT_TRUE(std::equal(solution.cbegin(), solution.cend(), zDeviceMappedForRead)) <<
                "Host and device saxpy execution results are not equal";

            result = clEnqueueUnmapMemObject(m_saxpyQueue,
                                             xDevice,
                                             xDeviceMappedForRead,
                                             0,
                                             nullptr,
                                             nullptr);

            EXPECT_EQ(result, CL_SUCCESS);

            result = clEnqueueUnmapMemObject(m_saxpyQueue,
                                             yDevice,
                                             yDeviceMappedForRead,
                                             0,
                                             nullptr,
                                             nullptr);

            EXPECT_EQ(result, CL_SUCCESS);

            result = clEnqueueUnmapMemObject(m_saxpyQueue,
                                             zDevice,
                                             zDeviceMappedForRead,
                                             0,
                                             nullptr,
                                             nullptr);

            EXPECT_EQ(result, CL_SUCCESS);
        }

        {
            cl_event flushSaxpyQueue = nullptr;

            result = clEnqueueBarrierWithWaitList(m_saxpyQueue,
                                                  0,
                                                  nullptr,
                                                  &flushSaxpyQueue);

            ASSERT_EQ(result, CL_SUCCESS);

            result = clWaitForEvents(1, &flushSaxpyQueue);
            ASSERT_EQ(result, CL_SUCCESS);

            result = clReleaseEvent(flushSaxpyQueue);
            EXPECT_EQ(result, CL_SUCCESS);
        }

        result = clReleaseMemObject(xDevice);
        EXPECT_EQ(result, CL_SUCCESS);
        result = clReleaseMemObject(yDevice);
        EXPECT_EQ(result, CL_SUCCESS);
        result = clReleaseMemObject(zDevice);
        EXPECT_EQ(result, CL_SUCCESS);

        for (cl_event event : hostToDeviceResolves)
        {
            result = clReleaseEvent(event);
            EXPECT_EQ(result, CL_SUCCESS);
        }

        result = clReleaseEvent(zDeviceToHostResolve);
        EXPECT_EQ(result, CL_SUCCESS);
    }
}