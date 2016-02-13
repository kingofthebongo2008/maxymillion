#include "precompiled.h"

#include "composer_bridge_shared_compose_context.h"

#include <memory>

#include <composer/composer.h>

namespace Composer
{
    namespace Bridge
    {
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
            
        }
    }
}