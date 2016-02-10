// composer_runtime.cpp : Defines the exported functions for the DLL application.
//

#include "precompiled.h"
#include <cstdint>

namespace composer
{
    void        initialize();
    void        shutdown();
    uint32_t    create_shared_compose_context();
    void        free_shared_compose_context(uint32_t handle);
}

namespace bridge
{
    extern "C" __declspec(dllexport) void composer_runtime_initialize()
    {
        composer::initialize();
    }

    extern "C" __declspec(dllexport) void composer_runtime_shutdown()
    {
        composer::shutdown();
    }

    extern "C" __declspec(dllexport) uint32_t composer_runtime_create_shared_compose_context()
    {
        return 0;
    }

    extern "C" __declspec(dllexport) void    composer_runtime_free_shared_compose_context(uint32_t handle)
    {

    }
}


