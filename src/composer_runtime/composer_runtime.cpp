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

namespace Composer
{
    namespace Bridge
    {
        public ref class ComposerRuntime
        {
            public:

            ComposerRuntime()
            {
                composer::initialize();
            }

            ~ComposerRuntime()
            {
                composer::shutdown();
            }
        };

        public ref class ComposeContext
        {
            private:

            ComposerRuntime^ m_runtime;

            public:

            ComposeContext(ComposerRuntime^ runtime, System::String^ model0, System::String^ model1 ): m_runtime(runtime)
            {

            }

            void ComposeImages(System::String^ s)
            {

            }
        };
    }
    
    /*
    extern "C" __declspec(dllexport) void composer_runtime_initialize()
    {
        
    }

    extern "C" __declspec(dllexport) void composer_runtime_shutdown()
    {

    }

    extern "C" __declspec(dllexport) uint32_t composer_runtime_create_shared_compose_context()
    {
        return 0;
    }

    extern "C" __declspec(dllexport) void    composer_runtime_free_shared_compose_context(uint32_t handle)
    {

    }
    */
}


