using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using System;
using System.IO;

namespace ComposerBridge
{
    class ComposerJobDefinition
    {
        public ComposerJobDefinition()
        {

        }

        public ComposerJobDefinition(string inputDirectory, string outputDirectory, string framesDirectory) 
        {
            m_inputDirectory = inputDirectory;
            m_outputDirectory = outputDirectory;
            m_framesDirectory = framesDirectory;
        }

        public string InputDirectory {  get { return m_inputDirectory;  } }
        public string OutputDirectory { get { return m_outputDirectory; } }
        public string FramesDirectory { get { return m_framesDirectory; } }

        private string m_inputDirectory;
        private string m_outputDirectory;

        private string m_framesDirectory;
    };

    class ComposerJobDefinitionFactory
    {
        public static ComposerJobDefinition CreateFromStream( Stream s )
        {
            var r = new StreamReader(s);
            var d = new ComposerJobDefinition();

            var serializer = new JsonSerializer();
            serializer.Converters.Add(new JavaScriptDateTimeConverter());
            serializer.NullValueHandling = NullValueHandling.Ignore;

            using (var sr = new StreamReader(s))
            using (JsonReader reader = new JsonTextReader(sr))
            {
                return  serializer.Deserialize<ComposerJobDefinition>(reader);
            }
        }

        public static ComposerJobDefinition CreateFromFile ( string path )
        {
            var r = new ComposerJobDefinition();

            try
            {
                return CreateFromStream(new FileStream(path, FileMode.Open));
            }
            catch (Exception )
            {
                return r;
            }
        }
    }


}