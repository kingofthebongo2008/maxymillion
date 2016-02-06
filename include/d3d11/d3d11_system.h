#ifndef __d3d11_SYSTEM_H__
#define __d3d11_SYSTEM_H__

#include <d3d11/d3d11_error.h>
#include <d3d11/d3d11_pointers.h>
#include <os/windows/dxgi_pointers.h>

namespace d3d11
{
    struct system_context
    {
        dxgi::iadapter_ptr          m_adapter;
        idevice_ptr	                m_device;
        idevicecontext_ptr          m_immediate_context;
    };

    system_context create_system_context();

    inline system_context create_system_context()
    {
        //auto flags = D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_DEBUGGABLE | D3D11_CREATE_DEVICE_SWITCH_TO_REF | D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT; // D3D11_CREATE_DEVICE_BGRA_SUPPORT;  //D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        auto flags = 0; 
        
#if defined(_DEBUG)
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        auto level = D3D_FEATURE_LEVEL_11_0;
        D3D_FEATURE_LEVEL           level_out;

        idevicecontext_ptr          context;
        dxgi::iadapter_ptr          adapter;
        dxgi::iswapchain_ptr        swap_chain;

        idevice_ptr                 device;


        dxgi::ifactory_ptr factory;

        using namespace os::windows;

        throw_if_failed<create_dxgi_factory_exception>(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory));

        auto hr = factory->EnumAdapters(0, &adapter);
        throw_if_failed<create_device_exception>(hr);

        using namespace os::windows;

        hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, 0, flags, &level, 1, D3D11_SDK_VERSION, &device, &level_out, &context);
        throw_if_failed<create_device_exception>(hr);

        system_context result = { adapter, device, context };
        
        return std::move(result);
    }
}

#endif

