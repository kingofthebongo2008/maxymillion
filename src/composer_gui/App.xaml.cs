using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;

namespace composer_gui
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        Composer.Bridge.ComposerRuntime m_Runtime;

        App()
        {
            Startup += new StartupEventHandler(this.ApplicationStartupHandler);
            Exit += new ExitEventHandler(this.ApplicationExitHandler);
        }

        void ApplicationStartupHandler( object sender, StartupEventArgs e)
        {
            m_Runtime = new Composer.Bridge.ComposerRuntime();
        }

        void ApplicationExitHandler(object sender, ExitEventArgs e)
        {
            m_Runtime.Dispose();
            m_Runtime = null;
        }

        public Composer.Bridge.ComposerRuntime Runtime {  get { return m_Runtime;  } }

    }
}
