#pragma once

#include <d3d11/d3d11_helpers.h>

#include "gpu_texture_resource.h"
#include "gpu_constant_buffer.h"
#include "composer_shared_system_context.h"

namespace composer
{
    struct photo_models
    {
        gpu::texture_resource               m_photo_model_horizontal;
        gpu::texture_resource               m_photo_model_vertical;
        gpu::constant_buffer                m_photo_model_vertical_buffer;
        gpu::constant_buffer                m_photo_model_horizontal_buffer;
    };

    struct transparency_color
    {
        float m_r;
        float m_g;
        float m_b;
        float m_a;
    };

    inline transparency_color pack_color(const gpu::rgb r )
    {
        transparency_color c;

        c.m_r = r.b / 255.0f;
        c.m_g = r.g / 255.0f;
        c.m_b = r.r / 255.0f;
        c.m_a = 0.0f;

        transparency_color c2;
        c2.m_r = 197 / 255.0f;
        c2.m_g = 201 / 255.0f;
        c2.m_b = 195 / 255.0f;
        c2.m_a = 0.0f;

        return c;
    }

    class shared_compose_context
    {
    public:

        shared_compose_context(const::std::shared_ptr<shared_system_context>& system, const std::wstring& url_horizontal, const std::wstring& url_vertical) : m_system(system)
        {
            auto d = static_cast<ID3D11Device*> (*this);
            auto c = static_cast<ID3D11DeviceContext*> (*this);

            concurrency::task_group g;

            g.run([this, d, c, url_horizontal, system]()
            {
                auto dc = d3d11::create_defered_context(d);
                auto t  = gpu::create_texture_resource2(d, dc, url_horizontal.c_str());

                auto transparency = pack_color(std::get<1>(t));
                auto s = gpu::create_constant_buffer(d, sizeof(transparency), &transparency);

                m_photo_models.m_photo_model_horizontal = std::get<0>(t);
                m_photo_models.m_photo_model_horizontal_buffer = s;
                execute_command_list(d3d11::finish_command_list(dc));

            });

            g.run([this, d, c, url_vertical, system]()
            {
                auto dc = d3d11::create_defered_context(d);
                auto t = gpu::create_texture_resource2(d, dc, url_vertical.c_str());
                
                auto transparency = pack_color(std::get<1>(t));
                auto s = gpu::create_constant_buffer(d, sizeof(transparency), &transparency);

                m_photo_models.m_photo_model_vertical = std::get<0>(t);
                m_photo_models.m_photo_model_vertical_buffer = s;
                
                execute_command_list(d3d11::finish_command_list(dc));
            });

            g.wait();

            auto h = m_photo_models.m_photo_model_horizontal;
            auto v = m_photo_models.m_photo_model_vertical;

            //swap horizontal and vertcal frame if needed
            if (h.width() < v.width())
            {
                std::swap(m_photo_models.m_photo_model_horizontal, m_photo_models.m_photo_model_vertical);
                std::swap(m_photo_models.m_photo_model_horizontal_buffer, m_photo_models.m_photo_model_vertical_buffer);
            }
        }

        shared_compose_context(const::std::shared_ptr<shared_system_context>& system, const std::wstring& url_model) : m_system(system)
        {
            auto d = static_cast<ID3D11Device*> (*this);
            auto c = static_cast<ID3D11DeviceContext*> (*this);

            concurrency::task_group g;

            g.run([this, d, c, url_model, system]()
            {
                auto dc = d3d11::create_defered_context(d);
                auto t = gpu::create_texture_resource2(d, dc, url_model.c_str());
                
                auto transparency = pack_color(std::get<1>(t));
                auto s = gpu::create_constant_buffer(d, sizeof(transparency), &transparency);

                m_photo_models.m_photo_model_horizontal         = std::get<0>(t);
                m_photo_models.m_photo_model_horizontal_buffer  = s;
                execute_command_list(d3d11::finish_command_list(dc));
            });

            g.wait();

            auto h = m_photo_models.m_photo_model_horizontal;

            //swap horizontal and vertcal frame if needed
            if (h.width() < h.height())
            {
                m_photo_models.m_photo_model_vertical          = h;
                m_photo_models.m_photo_model_vertical_buffer   = m_photo_models.m_photo_model_horizontal_buffer;

                m_photo_models.m_photo_model_horizontal        = gpu::texture_resource(d, rotate_image(h, c) );
                //assume on rotation the center pixel is the same, otherwise we must sample on the gpu
                m_photo_models.m_photo_model_horizontal_buffer = m_photo_models.m_photo_model_vertical_buffer;
            }
            else
            {
                m_photo_models.m_photo_model_vertical        = gpu::texture_resource(d, rotate_image(h, c));
                m_photo_models.m_photo_model_vertical_buffer = m_photo_models.m_photo_model_horizontal_buffer;
            }
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

        ID3D11Buffer* get_model_buffer(photo_mode mode) const
        {
            if (mode == photo_mode::horizontal)
            {
                return this->m_photo_models.m_photo_model_horizontal_buffer.m_buffer.get();
            }
            else
            {
                return this->m_photo_models.m_photo_model_vertical_buffer.m_buffer.get();
            }
        }

        operator ID3D11Device* () const
        {
            return static_cast<ID3D11Device*> (*m_system);
        }

        operator ID3D11DeviceContext* () const
        {
            return static_cast<ID3D11DeviceContext*> (*m_system);
        }

        ID3D11VertexShader* get_crop_shader_vs(photo_mode mode) const
        {
            return m_system->get_crop_shader_vs(mode);
        }

        ID3D11PixelShader* get_crop_shader_ps() const
        {
            return m_system->get_crop_shader_ps();
        }

        operator ID3D11BlendState*()  const
        {
            return static_cast<ID3D11BlendState*> (*m_system);
        }

        operator ID3D11RasterizerState*() const
        {
            return static_cast<ID3D11RasterizerState*> (*m_system);
        }

        operator ID3D11DepthStencilState*() const
        {
            return static_cast<ID3D11DepthStencilState*> (*m_system);
        }

        operator ID3D11SamplerState*() const
        {
            return static_cast<ID3D11SamplerState*> (*m_system);
        }

        void execute_command_list(ID3D11CommandList* list)
        {
            m_system->execute_command_list(list);
        }

        template <typename r, typename f> r execute_command_list(ID3D11CommandList* list, f f1)
        {
            return m_system->execute_command_list<r, f>(list, f1);
        }

        //shared
    private:

        std::shared_ptr<shared_system_context>              m_system;
        //shared read only
        photo_models                                        m_photo_models;

        d3d11::itexture2d_ptr rotate_image(gpu::texture_resource image, ID3D11DeviceContext* device_context)
        {
            auto d = static_cast<ID3D11Device*>(*this);
            auto desc = image.get_desc();
            auto width = desc.Height;  //swap
            auto height = desc.Width;

            auto resource = gx::create_render_target_resource(d, width, height, desc.Format);

            d3d11::vs_set_shader(device_context, m_system->get_rotate_shader_vs());
            d3d11::ps_set_shader(device_context, m_system->get_rotate_shader_ps());

            d3d11::gs_set_shader(device_context, nullptr);
            d3d11::ds_set_shader(device_context, nullptr);
            d3d11::hs_set_shader(device_context, nullptr);

            //set render target as the back buffer, goes to the operating system
            d3d11::om_set_render_target(device_context, resource);

            D3D11_VIEWPORT v = {};

            v.Height = static_cast<float>(height);
            v.Width  = static_cast<float>(width);

            v.MinDepth = 0.0f;
            v.MaxDepth = 1.0f;

            //set a view port for rendering
            device_context->RSSetViewports(1, &v);

            //clear the back buffer
            const float fraction = 0.0f;
            d3d11::clear_render_target_view(device_context, resource, math::set(0, 0, fraction, 1.0f));

            d3d11::om_set_depth_state(device_context, *this);
            d3d11::rs_set_state(device_context, *this);
            d3d11::om_set_blend_state(device_context, *this);

            d3d11::ia_set_index_buffer(device_context, nullptr);
            d3d11::ia_set_vertex_buffer(device_context, nullptr, 0);
            d3d11::ia_set_input_layout(device_context, nullptr);
            d3d11::ia_set_primitive_topology(device_context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            d3d11::ps_set_sampler_state(device_context, *this);

            d3d11::ps_set_shader_resource(device_context, image);

            device_context->Draw(3, 0);

            return resource.m_resource;
        }

    };

}
