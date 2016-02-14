#pragma once

#include <string>
#include <vector>
#include <util/util_noncopyable.h>

#include "composer_bridge_shared_system_context.h"

namespace Composer
{
    namespace Bridge
    {
        class SharedComposeContext : public util::noncopyable
        {
            public:

            SharedComposeContext( SharedSystemContext* system, const std::wstring& model0, const std::wstring& model1 );
            SharedComposeContext( SharedSystemContext* system, const std::wstring& model0);
            ~SharedComposeContext();

            void ComposeImages(const std::vector< std::wstring > in, const std::vector< std::wstring > & out);

            private:
            class SharedComposeContextImpl* m_impl;
        };
    }
}