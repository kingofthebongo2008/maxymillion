using ComposerBridge;
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
        Composer.Bridge.ComposerRuntime         m_Runtime;
        ComposerBridge.ComposerJobDefinition    m_Definitiion;

        Task[] taskArray = new Task[2];


        App()
        {
            taskArray[0] = Task.Run( 
                () =>
                {
                    m_Runtime = new Composer.Bridge.ComposerRuntime();
                });

            taskArray[1] = Task.Run(() =>
            {
                m_Definitiion = ComposerJobDefinitionFactory.CreateFromFile("composer_gui.json");

                //m_definition = ComposerJobDefinitionFactory.CreateFromFile("composer_gui.json");
                //var d = new ComposerJobDefinition(@"c:\", @"d:\", @"e:\");
                //ComposerJobDefinitionFactory.WriteToFile("composer_gui.json", d);
            }

            );

            Startup += new StartupEventHandler(this.ApplicationStartupHandler);
            Exit += new ExitEventHandler(this.ApplicationExitHandler);
        }

        void ApplicationStartupHandler( object sender, StartupEventArgs e)
        {
            Task.WaitAll(taskArray);
            taskArray = null;
        }

        void ApplicationExitHandler(object sender, ExitEventArgs e)
        {
            m_Runtime.Dispose();
            m_Runtime = null;
        }

        public Composer.Bridge.ComposerRuntime Runtime {  get { return m_Runtime;  } }
        public ComposerBridge.ComposerJobDefinition Definition { get { return m_Definitiion; } }

    }
}
