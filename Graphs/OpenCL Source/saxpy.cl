__kernel
void saxpy(const float                 a,
           __global const float* const x,
           __global float*       const y)
{
    const size_t globalId = get_global_id(0);

    y[globalId] = (a * x[globalId]) + y[globalId];
}