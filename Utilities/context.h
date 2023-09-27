#pragma once

#include <CL\cl.h>

#include <vector>

#include "platform.h"


namespace context
{
    [[nodiscard]] cl_int GetDevices(cl_context                 context,
                                    std::vector<cl_device_id>& devices);

    [[nodiscard]] cl_int Create(platform::UniSelectionStrategy strategy,
                                cl_platform_id&                selectedPlatform,
                                cl_context&                    context);

}