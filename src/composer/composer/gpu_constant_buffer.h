#pragma once

#include <d3d11/d3d11_helpers.h>

namespace composer
{
    namespace gpu
    {
        struct constant_buffer
        {
            public:

            constant_buffer()
            {

            }

            constant_buffer(d3d11::ibuffer_ptr buffer) : m_buffer(buffer)
            {


            }

            operator ID3D11Buffer* () const
            {
                return m_buffer.get();
            }

            d3d11::ibuffer_ptr                  m_buffer;
        };

        inline constant_buffer create_constant_buffer(ID3D11Device* d, size_t size, const void* initial_data)
        {
            return constant_buffer(d3d11::create_gpu_constant_buffer( d, size, initial_data) );
        }

    }
}
