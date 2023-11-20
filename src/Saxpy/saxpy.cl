__kernel void saxpy(         const float                 a,
                    __global const float* const restrict pXDevice,
                    __global const float* const restrict pYDevice,
                    __global       float* const restrict pZDevice,
                             const ulong                 len)
{
    const size_t globalId   = get_global_id(0);
    const size_t globalSize = get_global_size(0);

    for (size_t idx = globalId; idx < len; idx += globalSize)
    {
        pZDevice[idx] = (a * pXDevice[idx]) + pYDevice[idx];
    }
}