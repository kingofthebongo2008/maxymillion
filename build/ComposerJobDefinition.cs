using System.IO;

namespace ComposerBridge
{
    class ComposerJobDefinition
    {
        public ComposerJobDefinition(string inputDirectory, string outputDirectory) 
        {
            m_inputDirectory = inputDirectory;
            m_outputDirectory = outputDirectory;
        }

        public string InputDirectory {  get { return m_inputDirectory;  } }
        public string OutputDirectory { get { return m_outputDirectory; } }

        private string m_inputDirectory;
        private string m_outputDirectory;
    };

    class ComposerJobFactory
    {
        public static ComposerJob Create ( ComposerJobDefinition definition, ComposerModel model0, ComposerModel model1 )
        {
            var f0 = Directory.EnumerateFiles(definition.InputDirectory, "*.jpg", SearchOption.AllDirectories);
            var f1 = Directory.EnumerateFiles(definition.OutputDirectory, "*.jpg", SearchOption.AllDirectories);

            return new ComposerJob(f0, f1, model0, model1);
        }
    }

}