#include "precompiled.h"

#include <cstdint>
#include <iostream>

#include <fs/fs_media.h>
#include <imaging/imaging_utils.h>
#include <os/windows/com_initializer.h>

#include <d3d11/d3d11_system.h>

int32_t main( int32_t , char const* [] )
{
    using namespace     os::windows;
    com_initializer     com;

    fs::media_source source(L"./media/");

    auto url0 = fs::build_media_url(source, L"001JD31JAn16.JPG");

    //read the png texture
    auto texture = imaging::read_texture(url0.get_path());
    auto pixels = texture.get_pixels();
    auto width  = texture.get_width();
    auto height = texture.get_height();


    std::cout << "width: " << width << std::endl;
    std::cout << "height: " << height << std::endl;

    auto d = d3d11::create_system_context();


    return 0;
}


