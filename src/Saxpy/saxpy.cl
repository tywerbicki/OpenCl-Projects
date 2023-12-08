__kernel void saxpy(         const float                 a,
                    __global const float* const restrict pXDevice,
                    __global const float* const restrict pYDevice,
                    __global       float* const restrict pZDevice,
                             const ulong                 len)
{
    const size_t globalId = get_global_id(0);

    if (globalId < len)
    {
        pZDevice[globalId] = (a * pXDevice[globalId]) + pYDevice[globalId];
    }
}
