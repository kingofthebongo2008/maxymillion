#include "precompiled.h"

#include "composer_bridge_shared_compose_context.h"

#include <memory>

#include <composer/composer.h>

namespace Composer
{
    namespace Bridge
    {
        static void convert_texture(std::shared_ptr<composer::shared_compose_context>& shared, const std::wstring& in, const std::wstring& out)
        {
            auto dc = d3d11::create_defered_context(*shared);
            auto t = composer::gpu::create_texture_resource(*shared, dc, in.c_str());

            uint32_t size[2] = { 2284, 1632 };
            uint32_t width;
            uint32_t height;

            if (t.width() > t.height())
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
                auto t0 = ctx->compose_image(t, dc);

                d3d11::icommandlist_ptr list = d3d11::finish_command_list(dc);
                ID3D11DeviceContext* immediate_context = static_cast<ID3D11DeviceContext*> (*ctx);

                auto r = shared->execute_command_list< imaging::cpu_texture>(list, [immediate_context, &t0]
                {
                    return composer::gpu::copy_texture_to_cpu(immediate_context, t0);
                }
                );

                std::wstring w(out);
                imaging::write_texture(r, w.c_str());
            }
        }


        class SharedComposeContextImpl : public util::noncopyable
        {
            public:
            std::shared_ptr< composer::shared_compose_context >    m_shared_compose_context;

            SharedComposeContextImpl(SharedSystemContext* system, const std::wstring& model0, const std::wstring& model1)
            {
                m_shared_compose_context = std::make_shared< composer::shared_compose_context>(system->Get(), model0, model1);
            }
        };

        SharedComposeContext::SharedComposeContext(SharedSystemContext* system, const std::wstring& model0, const std::wstring& model1): m_impl( new SharedComposeContextImpl( system, model0, model1) )
        {
            
        }

        SharedComposeContext::~SharedComposeContext()
        {
            delete m_impl;
        }


        void SharedComposeContext::ComposeImages(const std::vector< std::wstring > in, const std::vector< std::wstring > & out)
        {
            auto size = static_cast<uint32_t>(in.size());
            auto shared = m_impl->m_shared_compose_context;

            concurrency::parallel_for(0U, size, [&shared, &in, &out](uint32_t i)
            {
                auto in_texture  = in[i];
                auto out_texture = out[i];
                convert_texture(shared, in_texture, out_texture);
            });
            
        }
    }
}