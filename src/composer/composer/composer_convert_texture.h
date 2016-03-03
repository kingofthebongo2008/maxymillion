#include <memory>
#include <tuple>
#include <string>

#include "composer_compose_context.h"

namespace composer
{
    static inline gpu::texture_resource create_texture_resource(const std::shared_ptr<shared_compose_context>& shared, ID3D11DeviceContext*c, const wchar_t* file_name)
    {
        std::wstring url(file_name);
        auto d = static_cast<ID3D11Device*>(*shared);

        auto cpu_texture = imaging::read_texture2(url.c_str());
        auto orientation = std::get<1>(cpu_texture);

        if (orientation != imaging::orientation::horizontal)
        {
            auto gpu_texture = gpu::upload_to_gpu(d, c, std::get<0>(cpu_texture));
            return gpu::texture_resource(d, shared->transform_image(gpu::texture_resource(d, gpu_texture), c, orientation));
        }
        else
        {
            auto gpu_texture = gpu::upload_to_gpu(d, c, std::get<0>(cpu_texture));
            return gpu::texture_resource(d, gpu_texture);
        }
    }

    static void convert_texture(std::shared_ptr<shared_compose_context>& shared, const std::wstring& in, const std::wstring& out)
    {
        auto dc = d3d11::create_defered_context(*shared);
        auto t = create_texture_resource(shared, dc, in.c_str());

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

        auto ctx = std::make_shared< compose_context >(shared, width, height);

        {
            auto t0 = ctx->compose_image(t, dc);

            d3d11::icommandlist_ptr list = d3d11::finish_command_list(dc);
            ID3D11DeviceContext* immediate_context = static_cast<ID3D11DeviceContext*> (*ctx);

            auto r = shared->execute_command_list< imaging::cpu_texture>(list, [immediate_context, &t0]
            {
                return gpu::copy_texture_to_cpu(immediate_context, t0);
            }
            );

            std::wstring w(out);
            imaging::write_texture(r, w.c_str());
        }
    }
}
