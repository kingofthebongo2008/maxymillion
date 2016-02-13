// composer_runtime.cpp : Defines the exported functions for the DLL application.
//

#include "precompiled.h"

#include <cstdint>
#include <string>
#include <vector>
#include <filesystem>

#include <msclr/all.h>
#include <msclr/marshal_cppstd.h>

#include "composer_embedded.h"
#include "composer_bridge_shared_system_context.h"
#include "composer_bridge_shared_compose_context.h"


namespace Composer
{
    namespace Bridge
    {
        public ref class ComposerRuntime : public System::IDisposable
        {
            bool m_isDisposed;

            embedded<SharedSystemContext> m_shared_system_context;

            public:

            ComposerRuntime() : m_isDisposed(false)
            {

            }

            ~ComposerRuntime() {
                if (m_isDisposed)
                    return;

                this->!ComposerRuntime();
                m_isDisposed = true;
            }

            !ComposerRuntime()
            {

            }

            SharedSystemContext* Get()
            {
                return m_shared_system_context.get();
            }
        };

        inline std::vector<std::wstring> marshal_ienumerable_as( System::Collections::Generic::IEnumerable<System::String^>^  inputFiles)
        {
            std::vector<std::wstring> result;

            auto b = inputFiles->GetEnumerator();

            while (b->MoveNext())
            {
                System::String^ s = b->Current;

                result.emplace_back(msclr::interop::marshal_as<std::wstring>( s ));

            }

            return result;
        }

        inline std::vector< std::wstring > create_out_file_names(const std::vector< std::wstring >& file_path, const std::wstring& out_path)
        {
            namespace fs = std::experimental::filesystem;

            typedef std::vector< std::wstring > result_set_t;
            result_set_t result_set;

            result_set.reserve(file_path.size());

            std::wstring dir(out_path);

            for (auto& v : file_path)
            {
                fs::path p(v);
                result_set.push_back(dir + p.filename().generic_wstring());
            }

            return result_set;
        }

        public ref class ComposeContext
        {
            private:
            ComposerRuntime^                m_runtime;
            System::String^                 m_model0;
            System::String^                 m_model1;

            public:

            ComposeContext( ComposerRuntime^ runtime, System::String^ model0, System::String^ model1 ): 
                m_runtime(runtime)
                , m_model0(model0)
                , m_model1(model1)
            {


            }

            void ComposeImages( System::Collections::Generic::IEnumerable<System::String^> ^  inputFiles, System::String^ ouputDirectory ) 
            {
                using namespace msclr::interop;

                std::vector<std::wstring> in = marshal_ienumerable_as(inputFiles);
                std::vector<std::wstring> out = create_out_file_names(in,  marshal_as<std::wstring>(ouputDirectory) );
                
                System::String^ m0(m_model0);
                System::String^ m1(m_model1);

                std::auto_ptr<SharedComposeContext> ctx(new SharedComposeContext(m_runtime->Get(), marshal_as<std::wstring>(m0), marshal_as<std::wstring>(m1)));

                ctx->ComposeImages(in, out);
            }
        };
    }
}


