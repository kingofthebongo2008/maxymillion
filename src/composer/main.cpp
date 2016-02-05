#include "precompiled.h"

#include <cstdint>
#include <iostream>

#include <ppl.h>

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
    struct photo_models
    {
        d3d11::itexture2d_ptr               m_photo_model_horizontal_texture;
        d3d11::ishaderresourceview_ptr      m_photo_model_srv;

        d3d11::itexture2d_ptr               m_photo_model_vertical_texture;
        d3d11::ishaderresourceview_ptr      m_photo_model_srv;
    };

    struct compose_context
    {
        d3d11::system_context               m_system;

        d3d11::id3d11rendertargetview_ptr   m_render_target;
        d3d11::itexture2d_ptr               m_render_target_texture;
        d3d11::itexture2d_ptr               m_read_back_texture;

        d3d11::iblendstate_ptr              m_blend_state;
        d3d11::irasterizerstate_ptr         m_rasterizer_state;
        d3d11::idepthstencilstate_ptr       m_depth_state;
        d3d11::isamplerstate_ptr            m_sampler;
        D3D11_VIEWPORT                      m_view_port;

        shader_crop_vs                      m_vs_crop;
        shader_crop_ps                      m_ps_crop;

        photo_models                        m_photo_models;
    };

    struct compose_image_context
    {
        compose_context*               m_ctx;

        d3d11::itexture2d_ptr          m_photo_texture;
        d3d11::ishaderresourceview_ptr m_photo_srv;
    };

    ID3D11DeviceContext* context(compose_context& c)
    {
        return c.m_system.m_immediate_context.get();
    }

    ID3D11Texture2D* read_back_texture(compose_context& c)
    {
        return c.m_read_back_texture.get();
    }

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

    inline d3d11::itexture2d_ptr upload_to_gpu(ID3D11Device* device, const imaging::cpu_texture& t)
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

    inline d3d11::itexture2d_ptr create_read_back_texture(ID3D11Device* device, uint32_t width, uint32_t height)
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

        r.m_read_back_texture       = composer::create_read_back_texture(d, width, height);
        r.m_render_target_texture   = composer::create_render_target_texture(d, width, height);
        r.m_render_target           = d3d11::create_render_target_view(d, r.m_render_target_texture); //todo: makesrgb
        r.m_blend_state             = gx::create_opaque_blend_state(d);
        r.m_sampler                 = gx::create_point_sampler_state(d);
        r.m_rasterizer_state        = gx::create_cull_none_rasterizer_state(d);
        r.m_depth_state             = gx::create_depth_test_disable_state(d);
        
        r.m_view_port.Width  = static_cast<float>(width);
        r.m_view_port.Height = static_cast<float>(height);
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

    inline d3d11::itexture2d_ptr compose_images( compose_context* ctx )
    {
        ID3D11DeviceContext* device_context = ctx->m_system.m_immediate_context.get();

        d3d11::vs_set_shader(device_context, ctx->m_vs_crop);
        d3d11::ps_set_shader(device_context, ctx->m_ps_crop);

        d3d11::gs_set_shader(device_context, nullptr);
        d3d11::ds_set_shader(device_context, nullptr);
        d3d11::hs_set_shader(device_context, nullptr);

        //set render target as the back buffer, goes to the operating system
        d3d11::om_set_render_target(device_context, ctx->m_render_target);

        //set a view port for rendering
        device_context->RSSetViewports(1, &ctx->m_view_port);

        //clear the back buffer
        const float fraction = 0.0f;
        d3d11::clear_render_target_view(device_context, ctx->m_render_target, math::set(0, 0, fraction, 1.0f));

        d3d11::om_set_depth_state(device_context,ctx->m_depth_state);
        d3d11::rs_set_state(device_context,ctx->m_rasterizer_state);
        d3d11::om_set_blend_state(device_context, ctx->m_blend_state);

        d3d11::ia_set_index_buffer(device_context, nullptr);
        d3d11::ia_set_vertex_buffer(device_context, nullptr, 0);
        d3d11::ia_set_input_layout(device_context, nullptr);
        d3d11::ia_set_primitive_topology(device_context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->Draw(3, 0);

        return ctx->m_render_target_texture;
    }

    inline d3d11::itexture2d_ptr copy_texture(compose_context* ctx, d3d11::itexture2d_ptr t)
    {
        ID3D11DeviceContext* device_context = ctx->m_system.m_immediate_context.get();
        d3d11::copy_resource(device_context, ctx->m_read_back_texture, ctx->m_render_target_texture);
        return ctx->m_read_back_texture;
    }

    inline imaging::cpu_texture copy_texture_to_cpu(D3D11_TEXTURE2D_DESC d, D3D11_MAPPED_SUBRESOURCE m )
    {
        //assume 24 bit

        auto bpp = 24; 
        auto row_pitch = m.RowPitch;
        auto row_height = d.Height;
        auto row_width  = d.Width;
        auto image_size = row_pitch * row_height;

        std::unique_ptr<uint8_t[]> temp(new (std::nothrow) uint8_t[image_size]);
        uint8_t*                   source = reinterpret_cast<uint8_t*> (m.pData);
        uint8_t*                   destination = reinterpret_cast<uint8_t*> ( temp.get() );

        for (auto i = 0U; i < d.Height; ++i)
        {
            //position on the beginning of the row
            uint8_t* d_row = destination + i * row_pitch;
            uint8_t* s_row = source + i * row_pitch;

            //read the rgb values and skip the alpha
            for (auto j = 0U; j < row_width; ++j )
            {
                *d_row++ = *s_row++;
                *d_row++ = *s_row++;
                *d_row++ = *s_row++;

                s_row++;
            }
        }

        return imaging::cpu_texture( d.Width, d.Height, bpp, static_cast<uint32_t>(image_size), static_cast<uint32_t>(row_pitch), imaging::image_type::rgb, temp.release() );
    }

    inline  imaging::cpu_texture copy_texture_to_cpu( ID3D11DeviceContext* context, ID3D11Texture2D*  texture)
    {
        D3D11_MAPPED_SUBRESOURCE m = {};
        D3D11_TEXTURE2D_DESC     d;

        texture->GetDesc(&d);

        os::windows::throw_if_failed<d3d11::map_resource_exception > (context->Map(texture, 0, D3D11_MAP_READ, 0, &m));

        auto r = copy_texture_to_cpu(d, m);

        context->Unmap( texture, 0 );
        
        return r;
    }
}

namespace composer
{
    namespace async
    {
        concurrency::task < shader_crop_vs > create_crop_vs(ID3D11Device* d)
        {
            return concurrency::create_task([d]
            {
                return composer::create_shader_crop_vs(d);
            });
        }

        concurrency::task < shader_crop_ps > create_crop_ps(ID3D11Device* d)
        {
            return concurrency::create_task([d]
            {
                return composer::create_shader_crop_ps(d);
            });
        }

        std::tuple < shader_crop_vs, shader_crop_ps> create_sahders(ID3D11Device* d)
        {
            concurrency::task_group g;

            shader_crop_vs v;
            shader_crop_ps p;


            g.run([&v, d] { v = create_crop_vs(d).get(); });
            g.run([&p, d] { p = create_crop_ps(d).get(); });

            g.wait();
                
            return std::make_tuple(std::move(v), std::move(p));
        }
    }
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
    
    auto sys = d3d11::create_system_context();
    auto d   = composer::create_context(sys, 2284, 1632 );
    auto shaders = composer::async::create_sahders(sys.m_device);
    d.m_vs_crop = std::get<0>(shaders);
    d.m_ps_crop = std::get<1>(shaders);

    auto t0  = composer::compose_images(&d);
    auto t1  = composer::copy_texture(&d, t0);
    auto r   = composer::copy_texture_to_cpu(context(d), t1);



    imaging::write_texture(r, L"test.jpg");
    

    return 0;
}


