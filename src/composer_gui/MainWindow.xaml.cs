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

namespace composer_gui
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        Composer.Bridge.ComposerRuntime m_Runtime;

        public MainWindow()
        {
            App a = (composer_gui.App)App.Current;
            m_Runtime = a.Runtime;

            InitializeComponent();
        }

        private void explorer_ExplorerError(object sender, ExplorerErrorEventArgs e)
        {
            MessageBox.Show(e.Exception.Message);
        }

        private void btnUpdate_Click(object sender, RoutedEventArgs e)
        {
            inputDirectory.SelectedPath = txtPathInput.Text;
        }

    }
}
