#include "build.h"
#include "context.h"
#include "platform.h"
#include "program.h"
#include "saxpy.h"

#include <CL/cl.h>

#include <gtest/gtest.h>


class SaxpyTest : public testing::Test
{
protected:
    static void SetUpTestSuite() noexcept
    {
        cl_int         result   = CL_SUCCESS;
        cl_platform_id platform = nullptr;

        result = context::Create(platform::MostGpus, platform, s_context);
        ASSERT_EQ(result, CL_SUCCESS);

        if (!s_context)
        {
            GTEST_SKIP() << "No OpenCL context was created.";
        }

        cl_program program = nullptr;
        result             = program::Build(s_context,
                                            std::cref(build::saxpy::binaryCreator),
                                            build::saxpy::sourceCreator,
                                            build::saxpy::options,
                                            program);

        ASSERT_EQ(result, CL_SUCCESS);

        result = clUnloadPlatformCompiler(platform);
        ASSERT_EQ(result, CL_SUCCESS);

        std::array<cl_kernel, build::saxpy::clKernelNames.size()> kernels = {};

        result = program::CreateKernels(program,
                                        build::saxpy::clKernelNames,
                                        kernels);

        ASSERT_EQ(result, CL_SUCCESS);

        const cl_kernel saxpyKernel = kernels[0];
    }

    void SetUp() noexcept override final
    {
        cl_int                    result  = CL_SUCCESS;
        std::vector<cl_device_id> devices = {};

        result = context::GetDevices(s_context, devices);
        ASSERT_EQ(result, CL_SUCCESS);

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

    cl_command_queue m_saxpyQueue    = nullptr;
    cl_event         m_saxpyComplete = nullptr;

    static cl_context s_context;

    static const std::array<size_t, 6> ProblemSizes;
    static const float                 A;
};

cl_context SaxpyTest::s_context = nullptr;

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

        ASSERT_EQ(result, CL_SUCCESS);
    }
}