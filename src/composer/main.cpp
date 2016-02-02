#include "precompiled.h"

#include <cstdint>
#include <iostream>

#include <fs/fs_media.h>
#include <gx/gx_render_resource.h>
#include <imaging/imaging_utils.h>
#include <os/windows/com_initializer.h>

#include <d3d11/d3d11_system.h>
#include <d3d11/d3d11_pointers.h>
#include <d3d11/d3d11_helpers.h>

#include "shader_crop_vs.h"
#include "shader_crop_ps.h"

namespace composer
{
    struct compose_context
    {
        d3d11::system_context               m_system;

        d3d11::id3d11rendertargetview_ptr   m_render_target;
        d3d11::itexture2d_ptr               m_render_target_texture;
        d3d11::itexture2d_ptr               m_staging_texture;

        d3d11::iblendstate_ptr              m_blend_state;
        d3d11::irasterizerstate_ptr         m_rasterizer_state;
        d3d11::isamplerstate_ptr            m_sampler;
        D3D11_VIEWPORT                      m_view_port;

        shader_crop_vs                      m_vs_crop;
        shader_crop_ps                      m_ps_crop;
    };

    DXGI_FORMAT translate_format(imaging::image_type t)
    {
        switch (t)
        {
        case imaging::image_type::grayscale:    return DXGI_FORMAT_R8_UNORM;
        case imaging::image_type::rgb:          return DXGI_FORMAT_B8G8R8X8_UNORM;
        case imaging::image_type::float32:      return DXGI_FORMAT_R32_FLOAT;
        default:    return DXGI_FORMAT_R8_UNORM;
        }
    }

    inline d3d11::itexture2d_ptr create_texture(ID3D11Device* device, const imaging::cpu_texture& t)
    {
        D3D11_TEXTURE2D_DESC d = {};

        d.Format = translate_format(t.get_image_type());
        d.ArraySize = 1;
        d.MipLevels = 1;
        d.SampleDesc.Count = 1;
        d.Height = t.get_height();
        d.Width = t.get_width();
        d.Usage = D3D11_USAGE_DEFAULT;
        d.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        auto proxy = t.get_pixels();

        D3D11_SUBRESOURCE_DATA sd = {};

        sd.pSysMem = proxy.get_pixels_cpu();
        sd.SysMemPitch = t.get_pitch();
        sd.SysMemSlicePitch = t.get_size();

        return d3d11::create_texture_2d(device, &d, &sd);
    }

    inline d3d11::itexture2d_ptr create_staging_texture(ID3D11Device* device, uint32_t width, uint32_t height)
    {
        D3D11_TEXTURE2D_DESC d = {};

        d.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
        d.ArraySize = 1;
        d.MipLevels = 1;
        d.SampleDesc.Count = 1;
        d.Height = height;
        d.Width = width;
        d.Usage = D3D11_USAGE_STAGING;
        d.BindFlags = 0;
        d.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

        return d3d11::create_texture_2d(device, &d, nullptr);
    }

    inline d3d11::itexture2d_ptr create_render_target_texture(ID3D11Device* device, uint32_t width, uint32_t height)
    {
        D3D11_TEXTURE2D_DESC d = {};

        d.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
        d.ArraySize = 1;
        d.MipLevels = 1;
        d.SampleDesc.Count = 1;
        d.Height = height;
        d.Width = width;
        d.Usage = D3D11_USAGE_DEFAULT;
        d.BindFlags = D3D11_BIND_RENDER_TARGET;
        return d3d11::create_texture_2d(device, &d, nullptr);
    }


    inline compose_context create_context(d3d11::system_context system, uint32_t width, uint32_t height)
    {
        compose_context r;

        r.m_system = system;

        auto d                      = r.m_system.m_device.get();

        r.m_staging_texture         = composer::create_staging_texture(d, width, height);
        r.m_render_target_texture   = composer::create_render_target_texture(d, width, height);
        r.m_render_target           = d3d11::create_render_target_view(d, r.m_render_target_texture); //todo: makesrgb
        r.m_blend_state             = gx::create_opaque_blend_state(d);
        r.m_sampler                 = gx::create_point_sampler_state(d);
        r.m_rasterizer_state        = gx::create_cull_none_rasterizer_state(d);
        
        r.m_view_port.Width  = width;
        r.m_view_port.Height = height;
        r.m_view_port.TopLeftX = 0;
        r.m_view_port.TopLeftY = 0;
        r.m_view_port.MinDepth = 0.0f;
        r.m_view_port.MaxDepth = 1.0f;

        r.m_vs_crop             = create_shader_crop_vs(d);
        r.m_ps_crop             = create_shader_crop_ps(d);

        return r;
    }


    /*
    imaging::cpu_texture convert_texture(const imaging::cpu_texture& texture, compose_context& context)
    {
        return imaging::cpu_texture();
    }
    */
}


int32_t main( int32_t , char const* [] )
{
    using namespace     os::windows;
    com_initializer     com;

    fs::media_source source(L"./media/");

    auto url0 = fs::build_media_url(source, L"001JD31JAn16.JPG");

    //read the png texture

    auto texture = imaging::read_texture(url0.get_path());
    auto pixels = texture.get_pixels();
    auto width  = texture.get_width();
    auto height = texture.get_height();

    std::cout << "width: " << width << std::endl;
    std::cout << "height: " << height << std::endl;

    auto d = composer::create_context(d3d11::create_system_context(), 2285, 1632 );

    return 0;
}


