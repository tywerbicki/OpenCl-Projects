#ifndef UTILITIES_CONTEXT_H
#define UTILITIES_CONTEXT_H

#include "platform_types.h"

#include <CL\cl.h>

#include <vector>


namespace context
{
    [[nodiscard]] cl_int GetDevices(cl_context                 context,
                                    std::vector<cl_device_id>& devices);

    [[nodiscard]] cl_int Create(platform::UniSelectionStrategy strategy,
                                cl_platform_id&                selectedPlatform,
                                cl_context&                    context);
}


#endif // UTILITIES_CONTEXT_H