__kernel
void vfadd(
	__global const float* a,
	__global const float* b,
	__global float*       result)
{
	size_t gId = get_global_id(0);

	result[gId] = a[gId] + b[gId];
}