#pragma once

#include <memory>
#include <util/util_noncopyable.h>

namespace composer
{
    class shared_system_context;
}

namespace Composer
{
    namespace Bridge
    {
        class SharedSystemContext : public util::noncopyable
        {
            public:

            SharedSystemContext();
            ~SharedSystemContext();

            std::shared_ptr< composer::shared_system_context >  Get();

            private:

            class SharedSystemContextImpl* m_impl;
        };
    }
}