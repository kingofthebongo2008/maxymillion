using System.Collections.Generic;

namespace ComposerBridge
{
    class ComposerModel
    {
        public

        ComposerModel ( string filePath)
        {
            m_filePath = filePath;
        }

        public string FilePath
        {
            get { return m_filePath; }
        }

        private string m_filePath;
    };

    class ComposerJob
    {
        public ComposerJob (IEnumerable<string> inputFiles, IEnumerable<string> outputFiles, ComposerModel model0, ComposerModel model1 )
        {
            m_InputFiles = inputFiles;
            m_OutputFiles = outputFiles;

            m_model0 = model0;
            m_model1 = model1;
        }

        private IEnumerable<string> m_InputFiles;
        private IEnumerable<string> m_OutputFiles;

        ComposerModel m_model0;
        ComposerModel m_model1;
    };


}