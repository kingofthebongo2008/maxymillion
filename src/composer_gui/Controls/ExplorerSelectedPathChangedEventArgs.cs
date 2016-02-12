using System;

namespace composer_gui.Controls
{
    /// <summary>
    /// Provides data for explorer navigation exception handling.
    /// </summary>
    public class ExplorerSelectedPathChangedEventArgs: EventArgs
    {
        /// <summary>
        /// Gets thrown exception.
        /// </summary>
        public string SelectedPath
        {
            get;
            private set;
        }

        public ExplorerSelectedPathChangedEventArgs(string path)
        {
            SelectedPath = path;
        }
    }
}