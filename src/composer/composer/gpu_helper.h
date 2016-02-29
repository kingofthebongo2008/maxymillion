#pragma once

#include <imaging/imaging_utils.h>
#include <d3d11/d3d11_helpers.h>

namespace composer
{
    namespace gpu
    {
        struct rgb
        {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };

        namespace
        {
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
        }

        //todo: move this to the gpu
        inline void upload_to_gpu( D3D11_TEXTURE2D_DESC d, D3D11_MAPPED_SUBRESOURCE m, const imaging::cpu_texture& t )
        {
            //assume 24 bit source
            //assume 32 bit destination
          
            auto dst_row_pitch  = m.RowPitch;
            auto dst_row_width  = d.Width;

            auto src_row_pitch  = t.get_pitch();
           
            uint8_t* source         = reinterpret_cast<uint8_t*> (t.get_pixels().get_pixels_cpu() );
            uint8_t* destination    = reinterpret_cast<uint8_t*> (m.pData);
            

            for (auto i = 0U; i < d.Height; ++i)
            {
                //position on the beginning of the row
                uint8_t* d_row = destination + i * dst_row_pitch;
                uint8_t* s_row = source + i * src_row_pitch;

                //read the rgb values and skip the alpha
                for (auto j = 0U; j < dst_row_width; ++j)
                {
                    *d_row++ = *s_row++;
                    *d_row++ = *s_row++;
                    *d_row++ = *s_row++;
                    d_row++;
                }
            }
        }

        inline  d3d11::itexture2d_ptr upload_to_gpu(ID3D11DeviceContext* context, d3d11::itexture2d_ptr gpu_texture, const imaging::cpu_texture& t )
        {
            D3D11_MAPPED_SUBRESOURCE m = {};
            D3D11_TEXTURE2D_DESC     d;

            gpu_texture->GetDesc(&d);

            os::windows::throw_if_failed<d3d11::map_resource_exception >(context->Map(gpu_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &m));

            upload_to_gpu( d, m, t );

            context->Unmap(gpu_texture, 0);

            return gpu_texture;
        }

        inline d3d11::itexture2d_ptr upload_to_gpu(ID3D11Device* device, ID3D11DeviceContext* context, const imaging::cpu_texture& t)
        {
            D3D11_TEXTURE2D_DESC d = {};

            d.Format = translate_format(t.get_image_type());
            d.ArraySize = 1;
            d.MipLevels = 1;
            d.SampleDesc.Count = 1;
            d.Height = t.get_height();
            d.Width = t.get_width();
            d.Usage = D3D11_USAGE_DYNAMIC;
            d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            auto result = d3d11::create_texture_2d(device, &d, nullptr);

            upload_to_gpu(context, result, t);

            return result;
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



        inline imaging::cpu_texture copy_texture_to_cpu(D3D11_TEXTURE2D_DESC d, D3D11_MAPPED_SUBRESOURCE m)
        {
            //assume 24 bit

            auto bpp = 24;
            auto row_pitch = m.RowPitch;
            auto row_height = d.Height;
            auto row_width = d.Width;
            auto image_size = row_pitch * row_height;

            std::unique_ptr<uint8_t[]> temp(new (std::nothrow) uint8_t[image_size]);
            uint8_t*                   source = reinterpret_cast<uint8_t*> (m.pData);
            uint8_t*                   destination = reinterpret_cast<uint8_t*> (temp.get());

            for (auto i = 0U; i < d.Height; ++i)
            {
                //position on the beginning of the row
                uint8_t* d_row = destination + i * row_pitch;
                uint8_t* s_row = source + i * row_pitch;

                //read the rgb values and skip the alpha
                for (auto j = 0U; j < row_width; ++j)
                {
                    *d_row++ = *s_row++;
                    *d_row++ = *s_row++;
                    *d_row++ = *s_row++;

                    s_row++;
                }
            }

            return imaging::cpu_texture(d.Width, d.Height, bpp, static_cast<uint32_t>(image_size), static_cast<uint32_t>(row_pitch), imaging::image_type::rgb, temp.release());
        }

        inline  imaging::cpu_texture copy_texture_to_cpu(ID3D11DeviceContext* context, ID3D11Texture2D*  texture)
        {
            D3D11_MAPPED_SUBRESOURCE m = {};
            D3D11_TEXTURE2D_DESC     d;

            texture->GetDesc(&d);

            os::windows::throw_if_failed<d3d11::map_resource_exception >(context->Map(texture, 0, D3D11_MAP_READ, 0, &m));

            auto r = copy_texture_to_cpu(d, m);

            context->Unmap(texture, 0);

            return r;
        }

        inline rgb sample_texture(const imaging::cpu_texture& t, uint32_t x, uint32_t y)
        {
            auto row_pitch  = t.get_pitch();
            uint8_t*       source = t.get_pixels().get_pixels_cpu();
            auto  pixel = reinterpret_cast<rgb*>( source + y * row_pitch + x * 3 );
            return *pixel;
        }

        inline rgb sample_texture_center(const imaging::cpu_texture& t)
        {
            auto row_height = t.get_height();
            auto row_width  = t.get_width();
            auto x          = row_width / 2;

            auto y          = row_height / 2;
            return sample_texture(t, x, y);
        }
    }

}
