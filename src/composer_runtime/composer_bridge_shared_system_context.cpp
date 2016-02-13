#include "precompiled.h"

#include "composer_bridge_shared_system_context.h"

#include <memory>

#include <composer/composer.h>

namespace Composer
{
    namespace Bridge
    {
        class SharedSystemContextImpl : public util::noncopyable
        {
            public:

            std::shared_ptr< composer::shared_system_context >    m_shared_system_context;

            SharedSystemContextImpl()
            {
                m_shared_system_context = std::make_shared< composer::shared_system_context>(d3d11::create_system_context());
            }


        };

        SharedSystemContext::SharedSystemContext() : m_impl( new SharedSystemContextImpl() )
        {
            
        }

        SharedSystemContext::~SharedSystemContext()
        {
            delete m_impl;
        }

        std::shared_ptr< composer::shared_system_context >  SharedSystemContext::Get()
        {
            return m_impl->m_shared_system_context;
        }
    }
}