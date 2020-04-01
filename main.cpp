#include <iostream>
#include <CL/cl.h>

void handleStatus(int status)
{
    if(status<0)
        std::cout<<"Error Code : "<<status<<std::endl;
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
    double *matA = new double[A_ROWS * A_COLS];
    double *matB = new double[B_ROWS * B_COLS];
    double *matC = new double[A_ROWS * B_COLS];
    double *golden = new double[A_ROWS * B_COLS];
    int *sizeM = new int;
    int *sizeN = new int;
    int *sizeK = new int;
    *(sizeM) = A_ROWS;
    *(sizeN) = B_COLS;
    *(sizeK) = A_COLS;
    
}