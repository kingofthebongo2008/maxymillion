#pragma once

#include <memory>
#include <mutex>

#include <d3d11/d3d11_system.h>

#include <gx/gx_render_resource.h>

#include "shaders/shader_crop_vertical_vs.h"
#include "shaders/shader_crop_horizontal_vs.h"
#include "shaders/shader_crop_ps.h"
#include "shaders/shader_rotate_270_cw_ps.h"
#include "shaders/shader_rotate_vs.h"

#include <ppl.h>

namespace composer
{
    enum photo_mode
    {
        horizontal,
        vertical

    };

    class shared_system_context
    {
    public:

        shared_system_context(const::std::shared_ptr<d3d11::system_context>& system) : m_system(system)
        {
            auto d = system->m_device.get();

            concurrency::task_group g;

            g.run([this, d]()
            {
                m_vs_crop_horizontal = create_shader_crop_horizontal_vs(d);
            });

            g.run([this, d]()
            {
                m_vs_crop_vertical = create_shader_crop_vertical_vs(d);
            });

            g.run([this, d]()
            {
                m_ps_crop = create_shader_crop_ps(d);
            });

            g.run([this, d]()
            {
                m_ps_rotate = create_shader_rotate_270_cw_ps(d);
            });

            g.run([this, d]()
            {
                m_vs_rotate = create_shader_rotate_vs(d);
            });

            g.run([this, d]()
            {
                m_blend_state = gx::create_opaque_blend_state(d);
            });


            g.run([this, d]()
            {
                m_sampler = gx::create_point_sampler_state(d);
            });

            g.run([this, d]()
            {
                m_rasterizer_state = gx::create_cull_none_rasterizer_state(d);
            });

            g.run([this, d]()
            {
                m_depth_state = gx::create_depth_test_disable_state(d);
            });

            g.wait();
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

        ID3D11VertexShader* get_rotate_shader_vs() const
        {
            return m_vs_rotate;
        }
        
        ID3D11PixelShader* get_rotate_shader_ps() const
        {
            return m_ps_rotate;
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

        void execute_command_list(ID3D11CommandList* list)
        {
            std::lock_guard < std::mutex > g(m_immediate_context_lock);
            m_system->m_immediate_context->ExecuteCommandList(list, FALSE);
        }

        template <typename r, typename f> r execute_command_list(ID3D11CommandList* list, f f)
        {
            std::lock_guard < std::mutex > g(m_immediate_context_lock);
            m_system->m_immediate_context->ExecuteCommandList(list, FALSE);
            return f();
        }

    private:

        std::shared_ptr<d3d11::system_context>              m_system;
        std::mutex                                          m_immediate_context_lock;

        //shared read only
        d3d11::iblendstate_ptr                              m_blend_state;
        d3d11::irasterizerstate_ptr                         m_rasterizer_state;
        d3d11::idepthstencilstate_ptr                       m_depth_state;
        d3d11::isamplerstate_ptr                            m_sampler;

        //shared read only
        shader_crop_vertical_vs                             m_vs_crop_vertical;
        shader_crop_horizontal_vs                           m_vs_crop_horizontal;
        shader_crop_ps                                      m_ps_crop;

        shader_rotate_270_cw_ps                             m_ps_rotate;
        shader_rotate_vs                                    m_vs_rotate;

    };
}
