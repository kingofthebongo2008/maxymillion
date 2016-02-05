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

#include "gpu_helper.h"
#include "gpu_texture_resource.h"

namespace composer
{
    struct photo_models
    {
        gpu::texture_resource               m_photo_model_horizontal;
        gpu::texture_resource               m_photo_model_vertical;
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
        gpu::texture_resource          m_photo_texture;
    };

    ID3D11DeviceContext* context(compose_context& c)
    {
        return c.m_system.m_immediate_context.get();
    }

    ID3D11Texture2D* read_back_texture(compose_context& c)
    {
        return c.m_read_back_texture.get();
    }

    inline compose_context create_context(d3d11::system_context system, uint32_t width, uint32_t height)
    {
        compose_context r;
        r.m_system = system;

        auto d                      = r.m_system.m_device.get();

        r.m_read_back_texture       = composer::gpu::create_read_back_texture(d, width, height);
        r.m_render_target_texture   = composer::gpu::create_render_target_texture(d, width, height);
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

    inline d3d11::itexture2d_ptr compose_images( compose_context* ctx, const gpu::texture_resource image )
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

        d3d11::ps_set_sampler_state(device_context, ctx->m_sampler);
    
        ID3D11ShaderResourceView* views[] =
        {
            ctx->m_photo_models.m_photo_model_horizontal,
            image
        };

        device_context->PSSetShaderResources(0, 2, views);

        //d3d11::ps_set_shader_resources(device_context, image, ctx->m_photo_models.m_photo_model_horizontal);// , image );

        device_context->Draw(3, 0);

        return ctx->m_render_target_texture;
    }

    inline d3d11::itexture2d_ptr copy_texture(compose_context* ctx, d3d11::itexture2d_ptr t)
    {
        ID3D11DeviceContext* device_context = ctx->m_system.m_immediate_context.get();
        d3d11::copy_resource(device_context, ctx->m_read_back_texture, ctx->m_render_target_texture);
        return ctx->m_read_back_texture;
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

        std::tuple < shader_crop_vs, shader_crop_ps, photo_models > create_resources(ID3D11Device* d, ID3D11DeviceContext* c, const wchar_t* url0, const wchar_t* url1)
        {
            concurrency::task_group g;

            shader_crop_vs v;
            shader_crop_ps p;

            gpu::texture_resource m0;
            gpu::texture_resource m1;

            g.run([&v, d]  { v = create_crop_vs(d).get(); });
            g.run([&p, d]  { p = create_crop_ps(d).get(); });
            g.wait();

            m0 = gpu::create_texture_resource(d, c, url0).get();
            m1 = gpu::create_texture_resource(d, c, url1).get();
                
            return std::make_tuple(std::move(v), std::move(p), photo_models{ std::move(m0), std::move(m1) } );
        }
    }
}

int32_t main( int32_t , char const* [] )
{
    using namespace     os::windows;
    com_initializer     com;

    fs::media_source source(L"./media/");

    //read a texture
    auto url0 = fs::build_media_url(source, L"002JD31JAn16.JPG");

    auto url1 = fs::build_media_url(source, L"model/Adele & Anthony.tif");
    auto url2 = fs::build_media_url(source, L"model/Adele & Anthony VERT.tif");

    //read the png texture

    auto sys = d3d11::create_system_context();
    auto d   = composer::create_context(sys, 2284, 1632 );
    
    auto shaders = composer::async::create_resources(sys.m_device, sys.m_immediate_context, url1.get_path(), url2.get_path());

    d.m_vs_crop = std::get<0>(shaders);
    d.m_ps_crop = std::get<1>(shaders);
    d.m_photo_models = std::get<2>(shaders);

    auto l   = composer::gpu::create_texture_resource(sys.m_device, sys.m_immediate_context, url0.get_path());

    auto t0  = composer::compose_images(&d, l.get() );
    auto t1  = composer::copy_texture(&d, t0);
    auto r   = composer::gpu::copy_texture_to_cpu( context(d), t1 );

    imaging::write_texture(r, L"test.jpg");

    return 0;
}


