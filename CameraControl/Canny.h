#pragma once


#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <CL\cl.h>
#include <CL\cl_d3d11.h>
#include <string>

class CannyProcessor {
public:
    CannyProcessor();
    ~CannyProcessor();

    /// <summary>
    /// Initialize OpenCL and connect it to
    /// directx resource
    /// </summary>
    /// <param name="d3dDevice"></param>
    /// <returns></returns>
    bool Initialize(ID3D11Device* d3dDevice);

    // The main entry point for your FFmpeg loop
    void ProcessFrame(ID3D11Texture2D* inputTexture, ID3D11Texture2D* outputTexture);

private:
    cl_context       context;
    cl_command_queue queue;
    cl_program       program;
    cl_kernel        kSobel, kNMS, kHysteresis;

    // Helper to compile kernels from string or file
    bool BuildProgram(const std::string& source);
};
