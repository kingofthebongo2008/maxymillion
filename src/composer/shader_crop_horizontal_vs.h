#pragma once

#include <cstdint>
#include <future>

#include <d3d11/d3d11_error.h>
#include <d3d11/d3d11_pointers.h>


namespace composer
{
    typedef std::tuple< d3d11::ivertexshader_ptr, const void*, uint32_t> vertex_shader_create_info;

    namespace details
    {
        inline vertex_shader_create_info   create_shader_crop_horizontal_vs(ID3D11Device* device)
        {
            using namespace d3d11;
            d3d11::ivertexshader_ptr   shader;

            using namespace os::windows;

            //strange? see in the hlsl file
            static
            #include "shader_crop_horizontal_vs_compiled.hlsl"

                //load, compile and create a pixel shader with the code in the hlsl file, might get slow (this is a compilation), consider offloading to another thread
            throw_if_failed<create_vertex_shader>(device->CreateVertexShader(shader_crop_horizontal_vs_compiled, sizeof(shader_crop_horizontal_vs_compiled), nullptr, &shader));

            return std::make_tuple(shader, &shader_crop_horizontal_vs_compiled[0], static_cast<uint32_t> (sizeof(shader_crop_horizontal_vs_compiled)));
        }
    }

    class shader_crop_horizontal_vs final
    {

    public:

        shader_crop_horizontal_vs()
        {

        }

        explicit shader_crop_horizontal_vs( vertex_shader_create_info info ) :
            m_shader(std::get<0>(info))
            , m_code(std::get<1>(info))
            , m_code_size(std::get<2>(info))
        {
        }


        operator ID3D11VertexShader* () const
        {
            return m_shader.get();
        }

        d3d11::ivertexshader_ptr     m_shader;
        const void*                  m_code;
        uint32_t                     m_code_size;
    };


    inline shader_crop_horizontal_vs   create_shader_crop_horizontal_vs(ID3D11Device* device)
    {
        return shader_crop_horizontal_vs(details::create_shader_crop_horizontal_vs(device));
    }

}


