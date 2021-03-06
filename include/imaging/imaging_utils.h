#pragma once

#include <cstdint>
#include <memory>
#include <tuple>

#include <imaging/imaging.h>
#include <imaging/imaging_utils_base.h>
#include <imaging/imaging_utils_cpu.h>

namespace imaging
{
    typedef texture < cpu_texture_storage >     cpu_texture;

    //jfif tags for rotation, that come out from cameras
    enum orientation : uint32_t
    {
        horizontal = 1,                      // 1 = Horizontal(normal)
        mirror_horizontal = 2,               // 2 = Mirror horizontal
        rotate_180 = 3,                      // 3 = Rotate 180
        mirror_vertical = 4,                 // 4 = Mirror vertical
        mirror_horizontal_rotate_270_cw = 5, // 5 = Mirror horizontal and rotate 270 CW
        rotate_90_cw = 6,                    // 6 = Rotate 90 CW
        miror_horizontal_rotate_90_cw = 7,   // 7 = Mirror horizontal and rotate 90 CW
        rotate_270_cw = 8                    // 8 = Rotate 270 CW
    };

    inline std::tuple<cpu_texture, orientation > read_texture2(const wchar_t* url_path)
    {
        auto factory = imaging::create_factory();
        auto stream0 = imaging::create_stream_reading(factory, url_path);
        auto decoder0 = imaging::create_decoder_reading(factory, stream0);
        auto frame0 = imaging::create_decode_frame(decoder0);
        auto reader = imaging::create_metadata_query_reader(frame0);

        orientation o = orientation::horizontal;
        
        PROPVARIANT v;
        PropVariantInit(&v);

        auto res = reader->GetMetadataByName(L"/app1/ifd/{ushort=274}", &v);

        if (res == S_OK)
        {
            o = static_cast<orientation>(v.uiVal);

            if (o > orientation::rotate_270_cw)
            {
                o = orientation::horizontal;
            }
        }


        imaging::bitmap_source bitmap(frame0);
        auto size = bitmap.get_size();

        auto bpp = imaging::wic_bits_per_pixel(factory, GUID_WICPixelFormat24bppBGR);
        auto row_pitch = (bpp * std::get<0>(size) + 7) / 8;
        auto row_height = std::get<1>(size);
        auto image_size = row_pitch * row_height;

        std::unique_ptr<uint8_t[]> temp(new (std::nothrow) uint8_t[image_size]);

        bitmap.copy_pixels(nullptr, static_cast<uint32_t>(row_pitch), static_cast<uint32_t>(image_size), temp.get());
        return std::make_tuple( cpu_texture(std::get<0>(size), std::get<1>(size), bpp, static_cast<uint32_t>(image_size), static_cast<uint32_t>(row_pitch), image_type::rgb, temp.release() ) , o );
    }

    inline cpu_texture read_texture(const wchar_t* url_path)
    {
        auto factory    = imaging::create_factory();
        auto stream0    = imaging::create_stream_reading(factory, url_path);
        auto decoder0   = imaging::create_decoder_reading(factory, stream0);
        auto frame0     = imaging::create_decode_frame(decoder0);

        imaging::bitmap_source bitmap(frame0);

        auto size = bitmap.get_size();

        auto bpp = imaging::wic_bits_per_pixel(factory, GUID_WICPixelFormat24bppBGR);
        auto row_pitch = (bpp * std::get<0>(size) +7) / 8;
        auto row_height = std::get<1>(size);
        auto image_size = row_pitch * row_height;

        std::unique_ptr<uint8_t[]> temp(new (std::nothrow) uint8_t[image_size]);

        bitmap.copy_pixels(nullptr, static_cast<uint32_t>(row_pitch), static_cast<uint32_t>(image_size), temp.get());
        return cpu_texture(std::get<0>(size), std::get<1>(size), bpp, static_cast<uint32_t>(image_size), static_cast<uint32_t>(row_pitch), image_type::rgb, temp.release());
    }

    template <typename texture > inline void write_texture(const texture& t, const wchar_t* url_path)
    {
        using namespace os::windows;

        auto factory = imaging::create_factory();
        auto stream0 = imaging::create_stream_writing(factory, url_path);
        auto encoder0 = imaging::create_encoder_writing(factory, stream0);
        auto frame0 = imaging::create_encode_frame(encoder0);

        throw_if_failed<com_exception>(frame0->SetSize(t.get_width(), t.get_height()));

        WICPixelFormatGUID formatGUID;
        WICPixelFormatGUID formatGUID_required;


        switch (t.get_image_type())
        {
        case rgb:
        {
            formatGUID = formatGUID_required = GUID_WICPixelFormat24bppBGR;
        }
        break;

        case grayscale:
        {
            formatGUID = formatGUID_required = GUID_WICPixelFormat8bppGray;
        }
        break;
        case float32:
        {
            formatGUID = formatGUID_required = GUID_WICPixelFormat32bppGrayFloat;
        }
        break;
        }


        throw_if_failed<com_exception>(frame0->SetPixelFormat(&formatGUID));
        throw_if_failed<com_exception>(IsEqualGUID(formatGUID, formatGUID_required));

        auto proxy = t.get_pixels();


        throw_if_failed<com_exception>(frame0->WritePixels(t.get_height(), t.get_pitch(), t.get_size(), proxy.get_pixels_cpu()));
        throw_if_failed<com_exception>(frame0->Commit());
        throw_if_failed<com_exception>(encoder0->Commit());
    }

}
