#pragma once

#include <cstdint>
#include <future>

#include <d3d11/d3d11_error.h>
#include <d3d11/d3d11_pointers.h>


namespace composer
{
    typedef std::tuple< d3d11::ipixelshader_ptr, const void*, uint32_t> pixel_shader_create_info;

    namespace details
    {
        inline pixel_shader_create_info   create_shader_rotate_180_ps(ID3D11Device* device)
        {
            using namespace d3d11;
            d3d11::ipixelshader_ptr   shader;

            using namespace os::windows;

            //strange? see in the hlsl file
            static
            #include "shader_rotate_180_ps_compiled.hlsl"

                //load, compile and create a pixel shader with the code in the hlsl file, might get slow (this is a compilation), consider offloading to another thread
            throw_if_failed<create_pixel_shader>(device->CreatePixelShader(shader_rotate_180_ps_compiled, sizeof(shader_rotate_180_ps_compiled), nullptr, &shader));

            return std::make_tuple(shader, &shader_rotate_180_ps_compiled[0], static_cast<uint32_t> (sizeof(shader_rotate_180_ps_compiled)));
        }
    }

    class shader_rotate_180_ps final
    {

    public:
        shader_rotate_180_ps()
        {

        }

        explicit shader_rotate_180_ps(pixel_shader_create_info info) :
            m_shader(std::get<0>(info))
            , m_code(std::get<1>(info))
            , m_code_size(std::get<2>(info))
        {
        }

        operator ID3D11PixelShader* () const
        {
            return m_shader.get();
        }

        d3d11::ipixelshader_ptr      m_shader;
        const void*                  m_code;
        uint32_t                     m_code_size;
    };


    inline shader_rotate_180_ps   create_shader_rotate_180_ps(ID3D11Device* device)
    {
        return shader_rotate_180_ps(details::create_shader_rotate_180_ps(device));
    }

}


