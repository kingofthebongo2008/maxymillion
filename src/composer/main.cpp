#include "precompiled.h"

#include <cstdint>
#include <filesystem>
#include <iostream>

#include <unordered_set>

#include <ppl.h>

#include <fs/fs_media.h>
#include <gx/gx_render_resource.h>
#include <imaging/imaging_utils.h>
#include <os/windows/com_initializer.h>


#include <d3d11/d3d11_system.h>
#include <d3d11/d3d11_pointers.h>
#include <d3d11/d3d11_helpers.h>

#include "shader_crop_horizontal_vs.h"
#include "shader_crop_vertical_vs.h"
#include "shader_crop_ps.h"

#include "gpu_helper.h"
#include "gpu_texture_resource.h"

namespace composer
{
    enum photo_mode
    {
        horizontal,
        vertical
    };

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

        shader_crop_vertical_vs             m_vs_crop_vertical;
        shader_crop_horizontal_vs           m_vs_crop_horizontal;
        shader_crop_ps                      m_ps_crop;

        photo_models                        m_photo_models;

        uint32_t width() const
        {
            return static_cast<uint32_t> (m_view_port.Width - m_view_port.TopLeftX);
        }

        uint32_t height() const
        {
            return static_cast<uint32_t> (m_view_port.Height - m_view_port.TopLeftY);
        }
    };

    template <typename t> inline photo_mode get_mode(const t& v)
    {
        return v.width() > v.height() ? photo_mode::horizontal : photo_mode::vertical;
    }

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

        r.m_vs_crop_horizontal  = create_shader_crop_horizontal_vs(d);
        r.m_vs_crop_vertical    = create_shader_crop_vertical_vs(d);
        r.m_ps_crop             = create_shader_crop_ps(d);

        return r;
    }

    inline d3d11::itexture2d_ptr compose_images( compose_context* ctx, const gpu::texture_resource image )
    {
        ID3D11DeviceContext* device_context = ctx->m_system.m_immediate_context.get();

        if ( get_mode(*ctx) == photo_mode::horizontal)
        {
            d3d11::vs_set_shader(device_context, ctx->m_vs_crop_horizontal);
        }
        else
        {
            d3d11::vs_set_shader(device_context, ctx->m_vs_crop_vertical);
        }

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

        if (get_mode(*ctx) == photo_mode::horizontal)
        {
            d3d11::ps_set_shader_resources(device_context, ctx->m_photo_models.m_photo_model_horizontal, image);
        }
        else
        {
            d3d11::ps_set_shader_resources(device_context, ctx->m_photo_models.m_photo_model_vertical, image);
        }
        
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
        concurrency::task < shader_crop_horizontal_vs > create_crop_horizontal_vs(ID3D11Device* d)
        {
            return concurrency::create_task([d]
            {
                return composer::create_shader_crop_horizontal_vs(d);
            });
        }

        concurrency::task < shader_crop_vertical_vs > create_crop_vertical_vs(ID3D11Device* d)
        {
            return concurrency::create_task([d]
            {
                return composer::create_shader_crop_vertical_vs(d);
            });
        }

        concurrency::task < shader_crop_ps > create_crop_ps(ID3D11Device* d)
        {
            return concurrency::create_task([d]
            {
                return composer::create_shader_crop_ps(d);
            });
        }

        std::tuple < shader_crop_horizontal_vs, shader_crop_vertical_vs, shader_crop_ps, photo_models > create_resources(ID3D11Device* d, ID3D11DeviceContext* c, const wchar_t* url0, const wchar_t* url1)
        {
            concurrency::task_group g;

            shader_crop_horizontal_vs v0;
            shader_crop_vertical_vs   v1;
            shader_crop_ps p;

            gpu::texture_resource m0;
            gpu::texture_resource m1;

            g.run([&v0, d]  { v0 = create_crop_horizontal_vs(d).get(); });
            g.run([&v1, d] { v1 = create_crop_vertical_vs(d).get(); });
            g.run([&p, d]  { p = create_crop_ps(d).get(); });
            g.wait();

            m0 = gpu::create_texture_resource(d, c, url0).get();
            m1 = gpu::create_texture_resource(d, c, url1).get();
                
            return std::make_tuple(std::move(v0), std::move(v1), std::move(p), photo_models{ std::move(m0), std::move(m1) } );
        }
    }
}

static std::vector< std::wstring > file_paths( const wchar_t* file_path)
{
    namespace fs = std::experimental::filesystem;

    fs::path someDir(file_path);
    fs::directory_iterator end_iter;

    typedef std::vector< std::wstring > result_set_t;
    result_set_t result_set;
    

    if (fs::exists(someDir) && fs::is_directory(someDir))
    {
        for (fs::directory_iterator dir_iter(someDir); dir_iter != end_iter; ++dir_iter)
        {
            if (fs::is_regular_file(dir_iter->status()))
            {
                result_set.push_back( dir_iter->path() );
            }
        }
    }

    return result_set;
}

static std::vector< std::wstring > file_paths2( const std::vector< std::wstring >& file_path, const wchar_t* out_path )
{
    namespace fs = std::experimental::filesystem;

    typedef std::vector< std::wstring > result_set_t;
    result_set_t result_set;

    result_set.reserve(file_path.size());

    std::wstring dir(out_path);

    for (auto& v : file_path)
    {
        fs::path p(v);
        result_set.push_back(dir + p.filename().generic_wstring() );
    }

    return result_set;
}

static void convert_texture( composer::compose_context* ctx, const wchar_t* in, const wchar_t* out )
{
    auto l = composer::gpu::create_texture_resource(ctx->m_system.m_device, ctx->m_system.m_immediate_context, in);
    auto t = l.get();

    if ( composer::get_mode ( t ) == composer::get_mode ( *ctx) )
    {
        auto t0 = composer::compose_images(ctx, t);
        auto t1 = composer::copy_texture(ctx, t0);

        auto r = composer::gpu::copy_texture_to_cpu(context(*ctx), t1);
        auto w = std::wstring(out);

        concurrency::create_task([r, w ]
        {
            imaging::write_texture(r, w.c_str() );
        });
    }
}

int32_t main( int32_t , char const* [] )
{
    using namespace     os::windows;
    com_initializer     com;

    fs::media_source base(L"./media/");
    fs::media_source source_in(L"./media/in/");
    fs::media_source source_out(L"./media/out/");

    auto p = file_paths(source_in.get_path());
    auto p2 = file_paths2(p, source_out.get_path());

    //read a texture
    auto url0 = fs::build_media_url(source_in, L"002JD31JAn16.JPG");

    auto url1 = fs::build_media_url(base, L"model/Adele & Anthony.tif");
    auto url2 = fs::build_media_url(base, L"model/Adele & Anthony VERT.tif");

    //read the png texture

    auto sys = d3d11::create_system_context();

    auto d0  = composer::create_context(sys, 2284, 1632 );
    auto d1  = composer::create_context(sys, 1632, 2284 );

    auto shaders = composer::async::create_resources(sys.m_device, sys.m_immediate_context, url1.get_path(), url2.get_path());

    d0.m_vs_crop_horizontal = std::get<0>(shaders);
    d0.m_vs_crop_vertical   = std::get<1>(shaders);
    d0.m_ps_crop = std::get<2>(shaders);
    d0.m_photo_models = std::get<3>(shaders);

    d1.m_vs_crop_horizontal = std::get<0>(shaders);
    d1.m_vs_crop_vertical   = std::get<1>(shaders);
    d1.m_ps_crop = std::get<2>(shaders);
    d1.m_photo_models = std::get<3>(shaders);

    for (auto i = 0U; i < p.size(); ++i)
    {
        std::cout << "Picture : " << i << " of " << p.size() << std::endl;
        auto in = p[i];
        auto out = p2[i];

        convert_texture(&d0, in.c_str(), out.c_str() );
        convert_texture(&d1, in.c_str(), out.c_str());
    }

    return 0;
}


