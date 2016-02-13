using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using composer_gui.Controls;
using Microsoft.Win32;
using System.IO;
using ComposerBridge;

namespace composer_gui
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        Composer.Bridge.ComposerRuntime m_Runtime;

        List<string> m_Files        = new List<string>();
        List<string> m_FrameFiles   = new List<string>();

        ComposerJobDefinition   m_definition;

        public MainWindow()
        {
            App a = (composer_gui.App)App.Current;
            m_Runtime = a.Runtime;

            InitializeComponent();

            m_InputFiles.ItemsSource = m_Files;
            m_FramesHorizontal.ItemsSource = m_FrameFiles;
            m_FramesVertical.ItemsSource = m_FrameFiles;

            m_definition = ComposerJobDefinitionFactory.CreateFromFile("composer_gui.json");
        }

        IEnumerable<string> Files {
            get { return m_Files; } }

        IEnumerable<string> FrameFiles
        {
            get { return m_FrameFiles; }
        }

        private void explorer_ExplorerError(object sender, ExplorerErrorEventArgs e)
        {
            MessageBox.Show(e.Exception.Message);
        }

        private void m_Process_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("Not implemented");
        }

        private void m_SelectAll_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("Not implemented");
        }

        private void inputDirectory_SelectedPathChanged(object sender, ExplorerSelectedPathChangedEventArgs e)
        {
            m_Files.Clear();

            try
            {
                var sourceDirectory = e.SelectedPath;
                var txtFiles = Directory.EnumerateFiles(sourceDirectory, "*.jpg", SearchOption.TopDirectoryOnly);

                m_Files.AddRange(txtFiles);
            }
            catch (Exception )
            {

            }

            this.m_InputFiles.Items.Refresh();
        }

        private void inputDirectoryLoaded(object sender, RoutedEventArgs e)
        {

            //todo: check if path exists
            m_inputDirectory.SelectedPath   = m_definition.InputDirectory;
            m_outputDirectory.SelectedPath  = m_definition.OutputDirectory;
            m_frameDirectory.SelectedPath   = m_definition.FramesDirectory;


        }
    }
}
