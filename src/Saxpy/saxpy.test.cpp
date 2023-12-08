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


namespace
{
    enum saxpyHostToDeviceResolve : unsigned int
    {
        x = 0,
        y,
        count,
    };
}


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
                                s_program);

        ASSERT_EQ(result, CL_SUCCESS);

        result = clUnloadPlatformCompiler(platform.value());
        ASSERT_EQ(result, CL_SUCCESS);
    }

    void SetUp() override final
    {
        cl_int                                                    result  = CL_SUCCESS;
        std::array<cl_kernel, build::saxpy::clKernelNames.size()> kernels = {};
        std::vector<cl_device_id>                                 devices = {};

        result = program::CreateKernels(s_program,
                                        build::saxpy::clKernelNames,
                                        kernels);

        ASSERT_EQ(result, CL_SUCCESS);

        result = context::GetDevices(s_context, devices);
        ASSERT_EQ(result, CL_SUCCESS);

        m_kernel                  = kernels[0];
        const cl_device_id device = devices[0];

        const std::array<const cl_queue_properties, 3> queueProperties
        {
            CL_QUEUE_PROPERTIES,
            CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
            0
        };

        m_queue = clCreateCommandQueueWithProperties(s_context,
                                                     device,
                                                     queueProperties.data(),
                                                     &result);

        ASSERT_EQ(result, CL_SUCCESS);
    }

    void TearDown() noexcept override final
    {
        cl_int result = CL_SUCCESS;

        result = clReleaseCommandQueue(m_queue);
        EXPECT_EQ(result, CL_SUCCESS);

        result = clReleaseKernel(m_kernel);
        EXPECT_EQ(result, CL_SUCCESS);
    }

    static void TearDownTestSuite() noexcept
    {
        cl_int result = CL_SUCCESS;

        result = clReleaseProgram(s_program);
        EXPECT_EQ(result, CL_SUCCESS);

        result = clReleaseContext(s_context);
        EXPECT_EQ(result, CL_SUCCESS);
    }

    static float GetRandFloat() noexcept
    {
        return static_cast<float>(std::rand());
    }

    void EnqueueSaxpyDeviceExecution(const size_t problemSize) noexcept
    {
        cl_int result = CL_SUCCESS;

        result = saxpy::EnqueueKernel(A,
                                      m_xDevice,
                                      m_yDevice,
                                      m_zDevice,
                                      problemSize,
                                      m_queue,
                                      m_kernel,
                                      m_hostToDeviceResolves,
                                      m_saxpyExec);

        ASSERT_EQ(result, CL_SUCCESS);
    }

    void FlushQueueAndSyncHost() noexcept
    {
        cl_int   result     = CL_SUCCESS;
        cl_event flushQueue = nullptr;

        result = clEnqueueBarrierWithWaitList(m_queue,
                                              0,
                                              nullptr,
                                              &flushQueue);

        ASSERT_EQ(result, CL_SUCCESS);

        result = clWaitForEvents(1, &flushQueue);
        ASSERT_EQ(result, CL_SUCCESS);

        result = clReleaseEvent(flushQueue);
        EXPECT_EQ(result, CL_SUCCESS);
    }

    void ReleaseDeviceBuffers() noexcept
    {
        cl_int result = CL_SUCCESS;

        result = clReleaseMemObject(m_xDevice);
        EXPECT_EQ(result, CL_SUCCESS);
        result = clReleaseMemObject(m_yDevice);
        EXPECT_EQ(result, CL_SUCCESS);
        result = clReleaseMemObject(m_zDevice);
        EXPECT_EQ(result, CL_SUCCESS);
    }

    void ReleaseResolveEvents() noexcept
    {
        cl_int result = CL_SUCCESS;

        for (cl_event event : m_hostToDeviceResolves)
        {
            result = clReleaseEvent(event);
            EXPECT_EQ(result, CL_SUCCESS);
        }

        result = clReleaseEvent(m_zDeviceToHostResolve);
        EXPECT_EQ(result, CL_SUCCESS);
    }

    cl_kernel        m_kernel    = nullptr;
    cl_command_queue m_queue     = nullptr;
    cl_event         m_saxpyExec = nullptr;

    std::array<cl_event, saxpyHostToDeviceResolve::count> m_hostToDeviceResolves = {};
    cl_event                                              m_zDeviceToHostResolve = nullptr;

    cl_mem m_xDevice = nullptr;
    cl_mem m_yDevice = nullptr;
    cl_mem m_zDevice = nullptr;

    static cl_context s_context;
    static cl_program s_program;

    static const std::array<size_t, 6> ProblemSizes;
    static const float                 A;
};

cl_context SaxpyTest::s_context = nullptr;
cl_program SaxpyTest::s_program = nullptr;

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

        m_xDevice = clCreateBuffer(s_context,
                                   CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                   problemSizeInBytes,
                                   nullptr,
                                   &result);

        ASSERT_EQ(result, CL_SUCCESS);

        {
            float* const pXDeviceMappedForWrite = static_cast<float*>(
                clEnqueueMapBuffer(m_queue,
                                   m_xDevice,
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

            std::generate(pXDeviceMappedForWrite,
                          pXDeviceMappedForWrite + problemSize,
                          GetRandFloat);

            result = clEnqueueUnmapMemObject(m_queue,
                                             m_xDevice,
                                             pXDeviceMappedForWrite,
                                             0,
                                             nullptr,
                                             &m_hostToDeviceResolves[saxpyHostToDeviceResolve::x]);

            ASSERT_EQ(result, CL_SUCCESS);
        }

        m_yDevice = clCreateBuffer(s_context,
                                   CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                   problemSizeInBytes,
                                   nullptr,
                                   &result);

        ASSERT_EQ(result, CL_SUCCESS);

        {
            float* const pYDeviceMappedForWrite = static_cast<float*>(
                clEnqueueMapBuffer(m_queue,
                                   m_yDevice,
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

            std::generate(pYDeviceMappedForWrite,
                          pYDeviceMappedForWrite + problemSize,
                          GetRandFloat);

            result = clEnqueueUnmapMemObject(m_queue,
                                             m_yDevice,
                                             pYDeviceMappedForWrite,
                                             0,
                                             nullptr,
                                             &m_hostToDeviceResolves[saxpyHostToDeviceResolve::y]);

            ASSERT_EQ(result, CL_SUCCESS);
        }

        m_zDevice = clCreateBuffer(s_context,
                                   CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_HOST_READ_ONLY,
                                   problemSizeInBytes,
                                   nullptr,
                                   &result);

        ASSERT_EQ(result, CL_SUCCESS);

        EnqueueSaxpyDeviceExecution(problemSize);

        {
            float* const pZDeviceMappedForRead = static_cast<float*>(
                clEnqueueMapBuffer(m_queue,
                                   m_zDevice,
                                   CL_FALSE,
                                   CL_MAP_READ,
                                   0,
                                   problemSizeInBytes,
                                   1,
                                   &m_saxpyExec,
                                   &m_zDeviceToHostResolve,
                                   &result)
            );

            ASSERT_EQ(result, CL_SUCCESS);

            float* const pXDeviceMappedForRead = static_cast<float*>(
                clEnqueueMapBuffer(m_queue,
                                   m_xDevice,
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

            float* const pYDeviceMappedForRead = static_cast<float*>(
                clEnqueueMapBuffer(m_queue,
                                   m_yDevice,
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

            saxpy::HostExec(A,
                            pXDeviceMappedForRead,
                            pYDeviceMappedForRead,
                            solution.data(),
                            solution.size());

            result = clWaitForEvents(1, &m_zDeviceToHostResolve);
            ASSERT_EQ(result, CL_SUCCESS);

            EXPECT_TRUE(std::equal(solution.cbegin(), solution.cend(), pZDeviceMappedForRead)) <<
                "Host and device saxpy execution results are not equal";

            result = clEnqueueUnmapMemObject(m_queue,
                                             m_xDevice,
                                             pXDeviceMappedForRead,
                                             0,
                                             nullptr,
                                             nullptr);

            EXPECT_EQ(result, CL_SUCCESS);

            result = clEnqueueUnmapMemObject(m_queue,
                                             m_yDevice,
                                             pYDeviceMappedForRead,
                                             0,
                                             nullptr,
                                             nullptr);

            EXPECT_EQ(result, CL_SUCCESS);

            result = clEnqueueUnmapMemObject(m_queue,
                                             m_zDevice,
                                             pZDeviceMappedForRead,
                                             0,
                                             nullptr,
                                             nullptr);

            EXPECT_EQ(result, CL_SUCCESS);
        }

        FlushQueueAndSyncHost();

        ReleaseDeviceBuffers();

        ReleaseResolveEvents();
    }
}


TEST_F(SaxpyTest, UsingAsyncDataTransfers)
{
    for (const size_t problemSize : ProblemSizes)
    {
        cl_int       result             = CL_SUCCESS;
        const size_t problemSizeInBytes = problemSize * sizeof(float);

        std::vector<float> xHost(problemSize);
        std::vector<float> yHost(problemSize);
        std::vector<float> zHost(problemSize);
        std::vector<float> solution(problemSize);

        std::generate(xHost.begin(), xHost.end(), GetRandFloat);

        m_xDevice = clCreateBuffer(s_context,
                                   CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                                   problemSizeInBytes,
                                   nullptr,
                                   &result);

        ASSERT_EQ(result, CL_SUCCESS);

        result = clEnqueueWriteBuffer(m_queue,
                                      m_xDevice,
                                      CL_FALSE,
                                      0,
                                      problemSizeInBytes,
                                      xHost.data(),
                                      0,
                                      nullptr,
                                      &m_hostToDeviceResolves[saxpyHostToDeviceResolve::x]);

        ASSERT_EQ(result, CL_SUCCESS);

        std::generate(yHost.begin(), yHost.end(), GetRandFloat);

        m_yDevice = clCreateBuffer(s_context,
                                   CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                                   problemSizeInBytes,
                                   nullptr,
                                   &result);

        ASSERT_EQ(result, CL_SUCCESS);

        result = clEnqueueWriteBuffer(m_queue,
                                      m_yDevice,
                                      CL_FALSE,
                                      0,
                                      problemSizeInBytes,
                                      yHost.data(),
                                      0,
                                      nullptr,
                                      &m_hostToDeviceResolves[saxpyHostToDeviceResolve::y]);

        ASSERT_EQ(result, CL_SUCCESS);

        m_zDevice = clCreateBuffer(s_context,
                                   CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY,
                                   problemSizeInBytes,
                                   nullptr,
                                   &result);

        ASSERT_EQ(result, CL_SUCCESS);

        EnqueueSaxpyDeviceExecution(problemSize);

        result = clEnqueueReadBuffer(m_queue,
                                     m_zDevice,
                                     CL_FALSE,
                                     0,
                                     problemSizeInBytes,
                                     zHost.data(),
                                     1,
                                     &m_saxpyExec,
                                     &m_zDeviceToHostResolve);

        ASSERT_EQ(result, CL_SUCCESS);

        saxpy::HostExec(A,
                        xHost.data(),
                        yHost.data(),
                        solution.data(),
                        solution.size());

        result = clWaitForEvents(1, &m_zDeviceToHostResolve);
        ASSERT_EQ(result, CL_SUCCESS);

        EXPECT_EQ(solution, zHost) << "Host and device saxpy execution results are not equal";

        ReleaseDeviceBuffers();

        ReleaseResolveEvents();
    }
}
