#include "precompiled.h"

#include <cstdint>
#include <memory>
#include <ppl.h>
#include <iostream>
#include <filesystem>

#include <sys/sys_profile_timer.h>

#include <composer/composer.h>
#include <os/windows/com_initializer.h>

#include <fs/fs_media.h>

namespace composer
{
    static std::vector< std::wstring > file_paths(const wchar_t* file_path)
    {
        namespace fs = std::experimental::filesystem;

        fs::path someDir(file_path);
        fs::directory_iterator end_iter;

        typedef std::vector< std::wstring > result_set_t;
        result_set_t result_set;

        if (fs::exists(someDir) && fs::is_directory(someDir))
        {
            for (fs::directory_iterator dir_iter(someDir); dir_iter != end_iter; ++dir_iter)
            {
                if (fs::is_regular_file(dir_iter->status()))
                {
                    result_set.push_back(dir_iter->path());
                }
            }
        }

        return result_set;
    }

    static std::vector< std::wstring > file_paths2(const std::vector< std::wstring >& file_path, const wchar_t* out_path)
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

    uint32_t create_shared_compose_context()
    {
        sys::profile_timer  timer;
        using namespace     os::windows;
        com_initializer     com;

        fs::media_source base(L"./media/");
        fs::media_source source_in(L"./media/in/");
        fs::media_source source_out(L"./media/out/");

        auto p = file_paths(source_in.get_path());
        auto p2 = file_paths2(p, source_out.get_path());

        //read a texture
        auto url1 = fs::build_media_url(base, L"model/Adele & Anthony.tif");
        auto url2 = fs::build_media_url(base, L"model/Adele & Anthony VERT.tif");

        //read the png texture

        std::wcout << L"source directory: " << source_in.get_path_wstring() << std::endl;
        std::wcout << L"destination directory: " << source_out.get_path_wstring() << std::endl;

        std::wcout << L"horizontal model: " << url1.get_path_wstring() << std::endl;
        std::wcout << L"vertical   model: " << url2.get_path_wstring() << std::endl;

        auto shared_system = std::make_shared< composer::shared_system_context>(d3d11::create_system_context());
        auto shared = std::make_shared< composer::shared_compose_context >(shared_system, url1.get_path_wstring(), url2.get_path_wstring());

        std::cout << "Initialization " << timer.milliseconds() << " ms" << std::endl;
        timer.reset();

        concurrency::parallel_for(0U, static_cast<uint32_t>(p.size()), [&shared, &p, &p2](uint32_t i)
        {
            auto in = p[i];
            auto out = p2[i];
            convert_texture(shared, in, out);
        });

        std::cout << "Conversion " << timer.milliseconds() << " ms" << std::endl;
        timer.reset();
        return 0;
    }

    void    free_shared_compose_context(uint32_t handle)
    {

    }
}
