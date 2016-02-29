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
using Microsoft.WindowsAPICodePack.Dialogs;
using Microsoft.WindowsAPICodePack.Shell;

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

        enum SelectDirectoryOption
        {
            Folder,
            File
        };


        public MainWindow( )
        {
            App a = (composer_gui.App)App.Current;
            m_Runtime = a.Runtime;
            m_definition = a.Definition;

            InitializeComponent();

            m_InputFiles.ItemsSource = m_Files;
            m_FramesHorizontal.ItemsSource = m_FrameFiles;

            m_inputPath.Text = m_definition.InputDirectory;
            m_outputPath.Text = m_definition.OutputDirectory;
            m_framePath.Text = m_definition.FramesDirectory;
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
            var s = m_outputPath.Text;
            if ( s[s.Length - 1 ] != '\\' )
            {
                return s + '\\';
            }
            else
            {
                return s;
            }

        }

        private string getHorizontalFrame()
        {
            return m_FramesHorizontal.SelectedItems[0].ToString();
        }

        private string getVerticalFrame()
        {
            if (m_FramesHorizontal.SelectedItems.Count > 1 )
            {
                return m_FramesHorizontal.SelectedItems[1].ToString();
            }
            else
            {
                return string.Empty;
            }
            
        }

        private async void m_Process_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                var i = getInputFiles();
                var o = getOutputDirectory();
                var h = getHorizontalFrame();
                var v = getVerticalFrame();
                await Task.Run(() => {

                    m_Progress.Dispatcher.Invoke(
                        () =>
                        {
                            m_Progress.IsIndeterminate = true;
                        });

                    if (v != string.Empty)
                    {
                        var context = new Composer.Bridge.ComposeContext(m_Runtime, h, v);
                        context.ComposeImages(i, o);
                    }
                    else
                    {
                        var context = new Composer.Bridge.ComposeContext(m_Runtime, h);
                        context.ComposeImages(i, o);
                    }

                    m_Progress.Dispatcher.Invoke(
                        () =>
                        {
                            m_Progress.IsIndeterminate = false;
                        });

                    this.Dispatcher.Invoke(
                       new Action(() =>
                       {
                           MessageBox.Show(this, "Processing completed", "Complete", MessageBoxButton.OK, MessageBoxImage.Information);
                       }));
                });
            }

            catch ( Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        private void m_SelectAll_Click(object sender, RoutedEventArgs e)
        {
            m_InputFiles.SelectAll();
        }

        private void handle_inputPath_TextChanged(string sourceDirectory)
        {
            m_Files.Clear();

            try
            {
                
                var txtFiles = Directory.EnumerateFiles(sourceDirectory, "*.jpg", SearchOption.TopDirectoryOnly);

                m_Files.AddRange(txtFiles);
            }
            catch (Exception)
            {

            }

            this.m_InputFiles.Items.Refresh();
        }


        private void m_inputPath_TextChanged(object sender, TextChangedEventArgs e)
        {
            var sourceDirectory = (sender as TextBox).Text;
            handle_inputPath_TextChanged(sourceDirectory);
        }

        private void handle_framePath_TextChanged( string sourceDirectory )
        {
            m_FrameFiles.Clear();

            try
            {
                
                var txtFiles = Directory.EnumerateFiles(sourceDirectory, "*.tif", SearchOption.TopDirectoryOnly);

                m_FrameFiles.AddRange(txtFiles);
            }

            catch (Exception)
            {

            }

            m_FramesHorizontal.Items.Refresh();

        }

        private void m_framePath_TextChanged(object sender, TextChangedEventArgs e)
        {
            var sourceDirectory = (sender as TextBox).Text;
            handle_framePath_TextChanged(sourceDirectory);
        }

        private void SelectFolder( string name, SelectDirectoryOption option, Action<string> a )
        {
            // Display a CommonOpenFileDialog to select only folders 
            CommonOpenFileDialog cfd = new CommonOpenFileDialog(name);
            cfd.EnsureReadOnly = true;
            cfd.IsFolderPicker = option == SelectDirectoryOption.Folder ? true : false;
            cfd.AllowNonFileSystemItems = true;
            cfd.InitialDirectory = name;


            if (cfd.ShowDialog(this) == CommonFileDialogResult.Ok)
            {
                if ( option == SelectDirectoryOption.Folder )
                {
                    var selectedSO = cfd.FileAsShellObject as ShellContainer;
                    a(selectedSO.ParsingName);
                }
                else
                {
                    var p = System.IO.Path.GetDirectoryName(cfd.FileName);
                    a(p);
                }
            }
        }

        private void m_InputDirectorySelect_Click(object sender, RoutedEventArgs e)
        {
            SelectInputPath();
        }

        private void SelectOutputPath()
        {
            SelectFolder(m_outputPath.Text, SelectDirectoryOption.Folder, (string s) =>
            {
                // Set the path in our filename textbox
                m_outputPath.Text = s;
            });
        }

        private void m_outputDirectorySelect_Click(object sender, RoutedEventArgs e)
        {
            SelectOutputPath();
        }

        void SelectFramePath()
        {
            SelectFolder(m_framePath.Text, SelectDirectoryOption.Folder, (string s) =>
            {
                var text = m_framePath.Text;
                // Set the path in our filename textbox
                m_framePath.Text = s;

                if (s.Equals(text))
                {
                    handle_framePath_TextChanged(s);
                }
            });
        }

        private void m_frameDirectorySelect_Click(object sender, RoutedEventArgs e)
        {
            SelectFramePath();
        }

        private void SelectInputPath()
        {
            SelectFolder(m_inputPath.Text, SelectDirectoryOption.File, (string s) =>
            {
                var text = m_inputPath.Text;
                // Set the path in our filename textbox

                // Set the path in our filename textbox
                m_inputPath.Text = s;

                if (s.Equals(text))
                {
                    handle_inputPath_TextChanged(s);
                }
            });
        }

        private void m_inputPath_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            SelectInputPath();
        }

        private void m_outputPath_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            SelectOutputPath();
        }

        private void m_framePath_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            SelectFramePath();
        }
    }
}
