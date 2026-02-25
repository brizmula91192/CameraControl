

#include "pch.h"
#include "Canny.h"
#include <vector>
#include <iostream>

/// <summary>
/// Pointers for interop between ffmpeg & directx
/// </summary>
typedef CL_API_ENTRY cl_int(CL_API_CALL* clGetDeviceIDsFromD3D11KHR_fn)(
    cl_platform_id platform, cl_uint d3d_device_source, void* d3d_object,
    cl_uint d3d_control, cl_uint num_entries, cl_device_id* devices, cl_uint* num_devices);


/// <summary>
/// Initialize OpenCL with directx device context
/// </summary>
/// <param name="d3dDevice"></param>
/// <returns></returns>
bool CannyProcessor::Initialize(ID3D11Device* d3dDevice) {
    cl_int err;

    // 1. Get Platform
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);

    // 2. Load the Extension Function Address
    auto clGetDeviceIDsFromD3D11KHR = (clGetDeviceIDsFromD3D11KHR_fn)
        clGetExtensionFunctionAddressForPlatform(platform, "clGetDeviceIDsFromD3D11KHR");

    if (!clGetDeviceIDsFromD3D11KHR) return false;

    // 3. Get the OpenCL device associated with your DX11 Device
    cl_device_id device;
    err = clGetDeviceIDsFromD3D11KHR(platform, CL_D3D11_DEVICE_KHR, d3dDevice,
        CL_PREFERRED_DEVICES_FOR_D3D11_KHR, 1, &device, NULL);

    // 4. Create Context with D3D11 Sharing
    cl_context_properties props[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
        CL_CONTEXT_D3D11_DEVICE_KHR, (cl_context_properties)d3dDevice,
        0
    };
    context = clCreateContext(props, 1, &device, NULL, NULL, &err);
    queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);

    // 5. Compile Kernels (Sobel, NMS, Hysteresis)
    // BuildProgram(cannyKernelSource); // You would implement this to call clBuildProgram

    return (err == CL_SUCCESS);
}


/// <summary>
/// Canny Kernel Source Code:
/// Sobel Filter
/// Non Maximum
/// Hysteresis
/// </summary>
const std::string cannyKernelSource = R"cl(
__kernel void sobel_gradient(__read_only image2d_t input, ...) {
    // Your Sobel code from earlier...
}

__kernel void nms_kernel(__read_only image2d_t mag_in, ...) {
    // Your NMS code from earlier...
}

__kernel void hysteresis_kernel(__read_only image2d_t nms_in, ...) {
    // Your Hysteresis code from earlier...
}
)cl";
