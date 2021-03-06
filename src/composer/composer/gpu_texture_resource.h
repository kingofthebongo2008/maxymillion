#pragma once

#include <tuple>
#include <ppl.h>

#include <d3d11/d3d11_helpers.h>

#include "gpu_helper.h"

namespace composer
{
    namespace gpu
    {
        struct texture_resource
        {
            public:

            texture_resource()
            {

            }

            texture_resource(ID3D11Device* d, d3d11::itexture2d_ptr texture) : m_texture(texture), m_texture_srv(d3d11::create_shader_resource_view(d, m_texture))
            {


            }

            operator ID3D11Texture2D* () const
            {
                return m_texture.get();
            }

            operator ID3D11ShaderResourceView* () const
            {
                return m_texture_srv.get();
            }

            uint32_t width() const
            {
                D3D11_TEXTURE2D_DESC d;
                m_texture->GetDesc(&d);
                return d.Width;
            }

            uint32_t height() const
            {
                D3D11_TEXTURE2D_DESC d;
                m_texture->GetDesc(&d);
                return d.Height;
            }

            D3D11_TEXTURE2D_DESC get_desc() const
            {
                D3D11_TEXTURE2D_DESC d;
                m_texture->GetDesc(&d);
                return d;
            }

            d3d11::itexture2d_ptr               m_texture;
            d3d11::ishaderresourceview_ptr      m_texture_srv;
        };

        inline std::tuple<texture_resource, rgb> create_texture_resource2( ID3D11Device* d, ID3D11DeviceContext*c, const wchar_t* file_name)
        {
            std::wstring url(file_name);

            auto texture0 = imaging::read_texture(url.c_str());
            auto center = sample_texture_center(texture0);
            auto gpu_texture = upload_to_gpu(d, c, texture0);
            return std::make_tuple(texture_resource(d, gpu_texture), center);
        }

        inline texture_resource create_texture_resource(ID3D11Device* d, ID3D11DeviceContext*c, const wchar_t* file_name)
        {
            std::wstring url(file_name);

            auto texture0 = imaging::read_texture(url.c_str());
            auto gpu_texture = upload_to_gpu(d, c, texture0);
            return texture_resource(d, gpu_texture);
        }
    }
}
