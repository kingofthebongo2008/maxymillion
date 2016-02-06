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

#include "gpu_pool_resources.h"

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

    class shared_compose_context
    {
        public:

        shared_compose_context( const::std::shared_ptr<d3d11::system_context> system, const std::wstring& url_horizontal, const std::wstring& url_vertical ) : m_system(system)
        {
            auto d = system->m_device.get();
            auto c = system->m_immediate_context.get();

            m_vs_crop_horizontal = create_shader_crop_horizontal_vs(d);
            m_vs_crop_vertical = create_shader_crop_vertical_vs(d);
            m_ps_crop = create_shader_crop_ps(d);

            m_blend_state = gx::create_opaque_blend_state(d);
            m_sampler = gx::create_point_sampler_state(d);
            m_rasterizer_state = gx::create_cull_none_rasterizer_state(d);
            m_depth_state = gx::create_depth_test_disable_state(d);

            m_photo_models.m_photo_model_horizontal = gpu::create_texture_resource(d, c, url_horizontal.c_str()).get();
            m_photo_models.m_photo_model_vertical   = gpu::create_texture_resource(d, c, url_vertical.c_str() ).get();
        }

        operator ID3D11Device* () const
        {
            return m_system->m_device.get();
        }

        operator ID3D11DeviceContext* () const
        {
            return m_system->m_immediate_context.get();
        }

        ID3D11VertexShader* get_crop_shader_vs(photo_mode mode) const
        {
            if (mode == photo_mode::horizontal)
            {
                return m_vs_crop_horizontal;
            }
            else
            {
                return m_vs_crop_vertical;
            }
        }

        ID3D11PixelShader* get_crop_shader_ps() const
        {
            return m_ps_crop;
        }

        ID3D11ShaderResourceView* get_model_texture(photo_mode mode) const
        {
            if (mode == photo_mode::horizontal)
            {
                return this->m_photo_models.m_photo_model_horizontal.m_texture_srv.get();
            }
            else
            {
                return this->m_photo_models.m_photo_model_vertical.m_texture_srv.get();
            }
        }

        operator ID3D11BlendState*()  const
        {
            return m_blend_state.get();
        }

        operator ID3D11RasterizerState*() const
        {
            return m_rasterizer_state.get();
        }

        operator ID3D11DepthStencilState*() const
        {
            return m_depth_state.get();
        }

        operator ID3D11SamplerState*() const
        {
            return m_sampler.get();
        }

        //shared
        private:

        std::shared_ptr<d3d11::system_context>              m_system;

        //shared read only
        d3d11::iblendstate_ptr              m_blend_state;
        d3d11::irasterizerstate_ptr         m_rasterizer_state;
        d3d11::idepthstencilstate_ptr       m_depth_state;
        d3d11::isamplerstate_ptr            m_sampler;

        //shared read only
        shader_crop_vertical_vs             m_vs_crop_vertical;
        shader_crop_horizontal_vs           m_vs_crop_horizontal;
        shader_crop_ps                      m_ps_crop;

        //shared read only
        photo_models                        m_photo_models;
    };

    template <typename t> inline photo_mode get_mode(const t& v)
    {
        return v.width() > v.height() ? photo_mode::horizontal : photo_mode::vertical;
    }

    struct compose_context
    {
        public:

        compose_context( const std::shared_ptr< shared_compose_context>& shared_context, uint32_t width, uint32_t height ) : m_shared_context( shared_context )
        {
            m_read_back_texture = gpu::create_read_back_texture(*m_shared_context, width, height);
            m_render_target_texture = gpu::create_render_target_texture(*m_shared_context, width, height);

            m_render_target = d3d11::create_render_target_view(*m_shared_context, m_render_target_texture); //todo: makesrgb

            m_view_port.Width = static_cast<float>(width);
            m_view_port.Height = static_cast<float>(height);
            m_view_port.TopLeftX = 0;
            m_view_port.TopLeftY = 0;
            m_view_port.MinDepth = 0.0f;
            m_view_port.MaxDepth = 1.0f;
        }

        d3d11::itexture2d_ptr compose_image(  const gpu::texture_resource image )
        {
            ID3D11DeviceContext* device_context = static_cast<ID3D11DeviceContext*>(*m_shared_context );

            d3d11::vs_set_shader(device_context, m_shared_context->get_crop_shader_vs(get_mode(*this)));
            d3d11::ps_set_shader(device_context, m_shared_context->get_crop_shader_ps());

            d3d11::gs_set_shader(device_context, nullptr);
            d3d11::ds_set_shader(device_context, nullptr);
            d3d11::hs_set_shader(device_context, nullptr);

            //set render target as the back buffer, goes to the operating system
            d3d11::om_set_render_target(device_context, m_render_target);

            //set a view port for rendering
            device_context->RSSetViewports(1, &m_view_port);

            //clear the back buffer
            const float fraction = 0.0f;
            d3d11::clear_render_target_view(device_context, m_render_target, math::set(0, 0, fraction, 1.0f));

            d3d11::om_set_depth_state(device_context, *m_shared_context);
            d3d11::rs_set_state(device_context, *m_shared_context);
            d3d11::om_set_blend_state(device_context, *m_shared_context);

            d3d11::ia_set_index_buffer(device_context, nullptr);
            d3d11::ia_set_vertex_buffer(device_context, nullptr, 0);
            d3d11::ia_set_input_layout(device_context, nullptr);
            d3d11::ia_set_primitive_topology(device_context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            d3d11::ps_set_sampler_state(device_context, *m_shared_context);

            d3d11::ps_set_shader_resources( device_context, m_shared_context->get_model_texture(get_mode(*this)), image );

            device_context->Draw(3, 0);

            d3d11::copy_resource(device_context, m_read_back_texture, m_render_target_texture);

            return m_read_back_texture;
        }

        operator ID3D11DeviceContext* () const
        {
            return static_cast<ID3D11DeviceContext*> (*m_shared_context);
        }

        operator ID3D11Device* () const
        {
            return static_cast<ID3D11Device*> ( *m_shared_context );
        }

        private:

        std::shared_ptr< shared_compose_context > m_shared_context;

        //private
        D3D11_VIEWPORT                      m_view_port;

        //pool
        d3d11::id3d11rendertargetview_ptr   m_render_target;
        d3d11::itexture2d_ptr               m_render_target_texture;
        d3d11::itexture2d_ptr               m_read_back_texture;

        
        public:
       
        uint32_t width() const
        {
            return static_cast<uint32_t> (m_view_port.Width - m_view_port.TopLeftX);
        }

        uint32_t height() const
        {
            return static_cast<uint32_t> (m_view_port.Height - m_view_port.TopLeftY);
        }

        private:
    };

    struct compose_image_context
    {
        std::shared_ptr<compose_context>               m_ctx;
        gpu::texture_resource                          m_photo_texture;
    };

    
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

static void convert_texture( const std::shared_ptr<composer::shared_compose_context>& shared, const std::wstring& in, const std::wstring& out)
{
    auto l = composer::gpu::create_texture_resource( *shared, *shared, in.c_str() );

    auto t = l.get();

    uint32_t size[2] = { 2284, 1632 };
    uint32_t width;
    uint32_t height;

    if ( t.width() > t.height() )
    {
        width = size[0];
        height = size[1];
    }
    else
    {
        width = size[1];
        height = size[0];
    }

    auto ctx = std::make_shared< composer::compose_context>(shared, width, height);

    {
        auto t0 = ctx->compose_image(t);

        auto r = composer::gpu::copy_texture_to_cpu( static_cast<ID3D11DeviceContext*> ( *ctx ), t0);

        std::wstring w(out);

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
    auto url1 = fs::build_media_url(base, L"model/Adele & Anthony.tif");
    auto url2 = fs::build_media_url(base, L"model/Adele & Anthony VERT.tif");

    //read the png texture

    std::wcout << L"source directory: " << source_in.get_path_wstring() << std::endl;
    std::wcout << L"destination directory: " << source_out.get_path_wstring() << std::endl;

    std::wcout << L"horizontal model: " << url1.get_path_wstring() << std::endl;
    std::wcout << L"vertical   model: " << url2.get_path_wstring() << std::endl;

    auto shared = std::make_shared< composer::shared_compose_context >(d3d11::create_system_context(), url1.get_path_wstring(), url2.get_path_wstring());

    for (auto i = 0U; i < p.size(); ++i)
    {
        std::cout << "Picture : " << i << " of " << p.size() << std::endl;
        auto in = p[i];
        auto out = p2[i];

        convert_texture(shared, in, out);
    }

    return 0;
}


