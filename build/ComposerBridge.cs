using System.Runtime.InteropServices;

namespace ComposerBridge
{
    public class Imports
    {
        [DllImport("composer_runtime.dll", CharSet = CharSet.Unicode)]
        public static extern void composer_runtime_initialize();

        [DllImport("composer_runtime.dll", CharSet = CharSet.Unicode)]
        public static extern void composer_runtime_shutdown();

        [DllImport("composer_runtime.dll", CharSet = CharSet.Unicode)]
        public static extern uint composer_runtime_create_shared_compose_context();

        [DllImport("composer_runtime.dll", CharSet = CharSet.Unicode)]
        public static extern void composer_runtime_free_shared_compose_context(uint handle);
    }
}
