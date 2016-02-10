#pragma once

#include <d3d11/d3d11_helpers.h>

#include "gpu_texture_resource.h"
#include "composer_shared_system_context.h"

namespace composer
{
    struct photo_models
    {
        gpu::texture_resource               m_photo_model_horizontal;
        gpu::texture_resource               m_photo_model_vertical;
    };


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
                m_photo_models.m_photo_model_horizontal = gpu::create_texture_resource(d, dc, url_horizontal.c_str());
                execute_command_list(d3d11::finish_command_list(dc));
            });

            g.run([this, d, c, url_vertical, system]()
            {
                auto dc = d3d11::create_defered_context(d);
                m_photo_models.m_photo_model_vertical = gpu::create_texture_resource(d, dc, url_vertical.c_str());
                execute_command_list(d3d11::finish_command_list(dc));
            });

            g.wait();

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
    };

}
