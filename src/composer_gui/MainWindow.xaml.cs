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

        List<string> m_Files = new List<string>();
        List<string> m_FrameFiles = new List<string>();

        ComposerJobDefinition m_definition;

        public MainWindow( )
        {
            App a = (composer_gui.App)App.Current;
            m_Runtime = a.Runtime;
            m_definition = a.Definition;

            InitializeComponent();

            m_InputFiles.ItemsSource = m_Files;
            m_FramesHorizontal.ItemsSource = m_FrameFiles;
            m_FramesVertical.ItemsSource = m_FrameFiles;

        }

        IEnumerable<string> Files
        {
            get { return m_Files; }
        }

        IEnumerable<string> FrameFiles
        {
            get { return m_FrameFiles; }
        }

        private void explorer_ExplorerError(object sender, ExplorerErrorEventArgs e)
        {
            MessageBox.Show(e.Exception.Message);
        }

        private List<string> getInputFiles()
        {
            var r = new List<string>();
            var s = m_InputFiles.SelectedItems;
            r.AddRange(s.OfType<string>());
            return r;
        }

        private string getOutputDirectory()
        {
            return m_outputPath.Text;
        }

        private string getHorizontalFrame()
        {
            return m_FramesHorizontal.SelectedItem.ToString();
        }

        private string getVerticalFrame()
        {
            return m_FramesVertical.SelectedItem.ToString();
        }

        private async void m_Process_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                var i = getInputFiles();
                var o = getOutputDirectory();
                var h = getHorizontalFrame();
                var v = getVerticalFrame();
                await Task.Run(() => { MessageBox.Show("Not implemented"); });
            }

            catch ( Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
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
            catch (Exception)
            {

            }

            this.m_InputFiles.Items.Refresh();
        }

        private void frameDirectory_SelectedPathChanged(object sender, ExplorerSelectedPathChangedEventArgs e)
        {
            m_FrameFiles.Clear();

            try
            {
                var sourceDirectory = e.SelectedPath;
                var txtFiles = Directory.EnumerateFiles(sourceDirectory, "*.tif", SearchOption.TopDirectoryOnly);

                m_FrameFiles.AddRange(txtFiles);
            }

            catch (Exception)
            {

            }

            this.m_FramesHorizontal.Items.Refresh();
            this.m_FramesVertical.Items.Refresh();
        }

        private void inputDirectoryLoaded(object sender, RoutedEventArgs e)
        {
            //todo: check if path exists
            m_inputDirectory.SelectedPath = m_definition.InputDirectory;

        }

        private void outputDirectoryLoaded(object sender, RoutedEventArgs e)
        {
            m_outputDirectory.SelectedPath = m_definition.OutputDirectory;
        }

        private void frameDirectoryLoaded(object sender, RoutedEventArgs e)
        {
            m_frameDirectory.SelectedPath = m_definition.FramesDirectory;
        }
    }
}
