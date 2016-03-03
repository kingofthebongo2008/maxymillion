struct vs_samples_output
{
    float4 m_position : SV_Position;
    float2 m_uv0      : texcoord0;
};

Texture2D		photo_model: register(t0);
SamplerState	photo_sampler : register(s0);


#define orientation_horizontal 1                         // 1 = Horizontal(normal)
#define orientation_mirror_horizontal 2                  // 2 = Mirror horizontal
#define orientation_rotate_180  3                        // 3 = Rotate 180
#define orientation_mirror_vertical 4                    // 4 = Mirror vertical
#define orientation_mirror_horizontal_rotate_270_cw 5    // 5 = Mirror horizontal and rotate 270 CW
#define orientation_rotate_90_cw 6                       // 6 = Rotate 90 CW
#define orientation_miror_horizontal_rotate_90_cw  7     // 7 = Mirror horizontal and rotate 90 CW
#define orientation_rotate_270_cw 8                      // 8 = Rotate 270 CW


float2x2 horizontal_matrix()
{
    return float2x2(
        float2 (1.0f, 0.0f),
        float2 (0.0f, 1.0f));
}

float2x2 mirror_horizontal_matrix()
{
    return float2x2(
        float2 (-1.0f, 0.0f),
        float2 (0.0f, 1.0f));
}

/*
cos  sin
-sin cos
*/

float2x2 rotate_180_matrix()
{
    return float2x2(
        float2 (-1.0f, 0.0f),
        float2 (0.0f, -1.0f));
}

float2x2 mirror_vertical_matrix()
{
    return float2x2(
        float2 (1.0f, 0.0f),
        float2 (0.0f, -1.0f));
}

float2x2 rotate_270_cw_matrix()
{
    return float2x2(
        float2 (0.0f, 1.0f),
        float2 (-1.0f, 0.0f));
}

float2x2 mirror_horizontal_rotate_270_cw_matrix()
{
    return mirror_horizontal_matrix() * rotate_270_cw_matrix();
}

float2x2 rotate_90_cw_matrix()
{
    return float2x2(
        float2 (0.0f, -1.0f),
        float2 (1.0f, 0.0f));
}

float2x2 miror_horizontal_rotate_90_cw_matrix()
{
    return mirror_horizontal_matrix() * rotate_90_cw_matrix();
}

float2x2 exif_rotation_matrix(uint orientation)
{
    switch (orientation)
    {
        case orientation_horizontal: return horizontal_matrix();
        case orientation_mirror_horizontal: return mirror_horizontal_matrix();
        case orientation_rotate_180: return rotate_180_matrix();
        case orientation_mirror_vertical: return mirror_vertical_matrix();
        case orientation_mirror_horizontal_rotate_270_cw: return mirror_horizontal_rotate_270_cw_matrix();
        case orientation_rotate_90_cw: return rotate_90_cw_matrix();
        case orientation_miror_horizontal_rotate_90_cw: return miror_horizontal_rotate_90_cw_matrix();
        case orientation_rotate_270_cw: return rotate_270_cw_matrix();
    }
}

float4 main(in vs_samples_output i) : sv_target0
{
    
    float2 h   = float2(0.5f, 0.5f);
    float2 uv  = i.m_uv0 - h;
    float2 uv2 = mul(uv, exif_rotation_matrix( orientation_rotate_270_cw ) );
    float2 uv3 = uv2 + h;
    
    //float2 uv3 = float2 (1.0f - i.m_uv0.y, i.m_uv0.x);

    float3 f0 = photo_model.Sample(photo_sampler, uv3).xyz;
    return float4( f0, 1.0f);
}

