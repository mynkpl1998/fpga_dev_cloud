#define SIZE 100000

__kernel void add(__global const float * restrict A, __global const float * restrict B, __global float * restrict out)
{
    for(int index=0; index<SIZE; index++)
        out[index] = A[index] + B[index];
}