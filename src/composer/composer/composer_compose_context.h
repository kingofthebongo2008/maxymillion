#pragma once

#include "composer_shared_compose_context.h"

namespace composer
{
    template <typename t> inline photo_mode get_mode(const t& v)
    {
        return v.width() > v.height() ? photo_mode::horizontal : photo_mode::vertical;
    }

    struct compose_context
    {
        public:

            compose_context(const std::shared_ptr< shared_compose_context>& shared_context, uint32_t width, uint32_t height) : m_shared_context(shared_context)
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

            d3d11::itexture2d_ptr compose_image(const gpu::texture_resource image, ID3D11DeviceContext* device_context)
            {
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

                d3d11::ps_set_shader_resources(device_context, m_shared_context->get_model_texture(get_mode(*this)), image);

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
                return static_cast<ID3D11Device*> (*m_shared_context);
            }

        private:

            std::shared_ptr< shared_compose_context > m_shared_context;

            //private
            D3D11_VIEWPORT                      m_view_port;

            //pool
            d3d11::irendertargetview_ptr        m_render_target;
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
}
