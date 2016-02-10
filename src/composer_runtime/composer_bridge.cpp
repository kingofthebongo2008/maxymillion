#include "precompiled.h"

#include <cstdint>
#include <memory>

#include <composer/composer_shared_system_context.h>
#include <os/windows/com_initializer.h>

namespace composer
{
    struct initializer
    {
        //os::windows::com_initializer                      m_com;
        std::shared_ptr<composer::shared_system_context>  m_shared_system_context;

        initializer()
        {
            m_shared_system_context = std::make_shared< composer::shared_system_context>(d3d11::create_system_context());
        }

    };

    static std::shared_ptr< initializer > g_initializer;

    void initialize()
    {
        g_initializer = std::make_shared<initializer>();
    }

    void shutdown()
    {
        g_initializer.reset();
    }

    uint32_t create_shared_compose_context()
    {
        return 0;
    }

    void    free_shared_compose_context(uint32_t handle)
    {

    }
}
