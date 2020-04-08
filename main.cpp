#include <iostream>
#include <chrono>
#include <CL/cl.h>
#include <cmath>

#define SIZE 100000
#define FLT_EPSILON 0.0001

bool comparewithGolden(float *golden, float *out)
{
    for(int i=0; i<SIZE; i++)
    {
        if((fabs(golden[i] - out[i])) > FLT_EPSILON)
        {
            printf("%.4f, %.4f\n", golden[i], out[i]);
            return false;
        }
    }
    return true;
}

void handleStatus(int status)
{
    if(status<0)
        std::cout<<"Error Code : "<<status<<std::endl;
}

int read_file(unsigned char **output, size_t *size, const char *name) {
  FILE* fp = fopen(name, "rb");
  if (!fp) {
    return -1;
  }

  fseek(fp, 0, SEEK_END);
  *size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  *output = (unsigned char *)malloc(*size);
  if (!*output) {
    fclose(fp);
    return -1;
  }

  fread(*output, *size, 1, fp);
  fclose(fp);
  return 0;
}

void initMatrix(float *arr, int size)
{
    for(int i=0; i<size; i++)
        arr[i] = (float) rand() / RAND_MAX / 10.0;
}

void hostadd(float* A, float* B, float* out, int size)
{
    for(int index=0; index<size; index++)
        out[index] = A[index] + B[index];
}

int main()
{   
    int status;
    int platformSelection, deviceSelection;

    // Query for platforms
    cl_uint numPlatforms = 0;
    cl_platform_id *platforms = NULL;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    handleStatus(status);
    printf("INFO: Found %d platforms.\n", numPlatforms);

    platforms = (cl_platform_id *)malloc(numPlatforms * sizeof(cl_platform_id));
    status = clGetPlatformIDs(numPlatforms, platforms, NULL);
    handleStatus(status);

    // Print platforms information
    for(int i=0; i<numPlatforms; i++)
    {
        char platformInfoBuff[100];
        status = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 100, platformInfoBuff, NULL);
        printf("%d. %s\n", i, platformInfoBuff);
        handleStatus(status);
    }
    printf("INFO: Select platform from above.\n");
    scanf("%d",&platformSelection);

    // Reterive number of devices
    cl_uint numDevices = 0;

    status = clGetDeviceIDs(platforms[platformSelection], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
    handleStatus(status);
    printf("INFO: Found %d devices.\n", numDevices);
    cl_device_id *devices = (cl_device_id *) malloc(sizeof(cl_device_id) * numDevices);
    status = clGetDeviceIDs(platforms[platformSelection], CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);
    handleStatus(status);

    for(int i=0; i<numDevices; i++)
    {
        char deviceInfoBuff[100];
        status = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 100, deviceInfoBuff, NULL);
        printf("%d. %s\n", i, deviceInfoBuff);
        handleStatus(status);
    }
    printf("INFO: Select one of the devices from the above.\n");
    scanf("%d", &deviceSelection);

    // Create context for the devices.
    printf("INFO: Creating context for the discovered devices.\n");
    cl_context context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &status);
    handleStatus(status);

    // Create a command - queue per device.
    cl_command_queue cmdQueue = clCreateCommandQueue(context, devices[deviceSelection], CL_QUEUE_PROFILING_ENABLE, &status);
    handleStatus(status);

    // Create buffers on host
    printf("INFO: Allocating buffers on host to hold the data.\n");
    float *matA = new float[SIZE];
    float *matB = new float[SIZE];
    float *out = new float[SIZE];
    float *golden = new float[SIZE];

    // Init matrices with random 32-bit double values
    printf("INFO: Initializing matrices\n");
    initMatrix(matA, SIZE);
    initMatrix(matB, SIZE);

    printf("INFO: Calculating golden results\n");
    auto cpu_start = std::chrono::high_resolution_clock::now();
    hostadd(matA, matB, golden, SIZE);
    auto cpu_end = std::chrono::high_resolution_clock::now();
    auto cpu_time = std::chrono::duration_cast<std::chrono::microseconds>(cpu_end - cpu_start);
    std::cout<<"INFO: CPU execution time : "<<cpu_time.count()<<" us."<<std::endl;
    
    printf("INFO: Creating OpenCL buffers to hold the data.\n");
    cl_mem buffmatA = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * SIZE, NULL, &status);
    handleStatus(status);
    cl_mem buffmatB = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * SIZE, NULL, &status);
    handleStatus(status);
    cl_mem buffOut = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * SIZE, NULL, &status);
    handleStatus(status);

    printf("INFO: Transferring buffers to the device.\n");
    status = clEnqueueWriteBuffer(cmdQueue, buffmatA, CL_TRUE, 0, sizeof(float) * SIZE, matA, 0, NULL, NULL);
    handleStatus(status);
    status = clEnqueueWriteBuffer(cmdQueue, buffmatB, CL_TRUE, 0, sizeof(float) * SIZE, matB, 0, NULL, NULL);
    handleStatus(status);
    status = clEnqueueWriteBuffer(cmdQueue, buffOut, CL_TRUE, 0, sizeof(float) * SIZE, out, 0, NULL, NULL);
    handleStatus(status);

    printf("INFO: Loading kernel binary.\n");
    unsigned char* program_file = NULL;
    size_t program_size = 0;
    std::string fileName = "bin/add/add.aocx";
    status = read_file(&program_file, &program_size, fileName.c_str());
    if(status == -1)
    {
        std::string errorMsg = "ERROR: Invalid kernel path.";
        throw std::invalid_argument(errorMsg);
    }

    cl_program program = clCreateProgramWithBinary(context, 1, devices, &program_size, (const unsigned char **)&program_file, NULL, &status);
    handleStatus(status);
    printf("INFO: Building kernel.\n");
    status = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
    handleStatus(status);
    printf("INFO: Extrating add kernel.\n");
    cl_kernel kernel = clCreateKernel(program, "add", &status);
    handleStatus(status);

    printf("INFO: Setting kernel Arguments\n");
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffmatA);
    handleStatus(status);
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffmatB);
    handleStatus(status);
    status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffOut);
    handleStatus(status);
    
    
    printf("INFO: Executing kernel.\n");
    int iters_completed;
    int num_iters = 100;
    cl_double elapsed = 0.0;
    cl_ulong start, end;
    for(int i=0; i<num_iters; i++)
    {
        cl_event kernelEvent;
        status = clEnqueueTask(cmdQueue, kernel, 0, NULL, &kernelEvent);
        clWaitForEvents(1, &kernelEvent);
        status = clGetEventProfilingInfo(kernelEvent, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
        handleStatus(status);
        status = clGetEventProfilingInfo(kernelEvent, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
        handleStatus(status);

        elapsed += (cl_double)(end - start) * (cl_double)(1e-03);
        std::cout<<"Performance after Iter "<<i+1<<" : "<<(elapsed/i+1)<<std::endl;
    }
    
    printf("INFO: Copying data back to the host.\n");
    status = clEnqueueReadBuffer(cmdQueue, buffOut, CL_TRUE, 0, sizeof(float) * SIZE, out, 0, NULL, NULL);
    handleStatus(status);

    printf("INFO: Checking for fuctional correctness.\n");
    if (comparewithGolden(golden, out) == false)
        printf("ERROR: Test failed.\n");
    else
        printf("INFO: Test Passed.\n");

    // Release all resources
    printf("INFO: Releasing resources.\n");
    delete[] matA, matB, golden, out;
    status = clReleaseMemObject(buffmatA);
    handleStatus(status);
    status = clReleaseMemObject(buffmatB);
    handleStatus(status);
    status = clReleaseMemObject(buffOut);
    handleStatus(status);
    status = clReleaseProgram(program);
    handleStatus(status);
    status = clReleaseKernel(kernel);
    handleStatus(status);
    status = clReleaseCommandQueue(cmdQueue);
    handleStatus(status);
    status = clReleaseContext(context);
    handleStatus(status);
    free(platforms);
    free(devices);
    free(program_file);
}