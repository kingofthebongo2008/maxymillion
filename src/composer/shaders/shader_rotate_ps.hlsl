struct vs_samples_output
{
    float4 m_position : SV_Position;
    float2 m_uv0      : texcoord0;
};

Texture2D		photo_model: register(t0);

SamplerState	photo_sampler : register(s0);

/*
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
*/

float2x2 orientation_horizontal()
{
    return float2x2(
        float2 (1.0f, 0.0f),
        float2 (0.0f, 1.0f));
}

float2x2 mirror_horizontal()
{
    return float2x2(
        float2 (-1.0f, 0.0f),
        float2 (0.0f, 1.0f));
}

/*
cos  sin
-sin cos
*/

float2x2 rotate_180()
{
    return float2x2(
        float2 (-1.0f, 0.0f),
        float2 (0.0f, -1.0f));
}

float2x2 mirror_vertical()
{
    return float2x2(
        float2 (1.0f, 0.0f),
        float2 (0.0f, -1.0f));
}

float2x2 rotate_270_cw()
{
    return float2x2(
        float2 (0.0f, -1.0f),
        float2 (1.0f, 0.0f));
}

float2x2 mirror_horizontal_rotate_270_cw()
{
    return mirror_horizontal() * rotate_270_cw();
}

float2x2 rotate_90_cw()
{
    return float2x2(
        float2 (0.0f, 1.0f),
        float2 (-1.0f, 0.0f));
}

float2x2 miror_horizontal_rotate_90_cw()
{
    return mirror_horizontal() * rotate_90_cw();
}

float4 main(in vs_samples_output i) : sv_target0
{
    float2 uv2 = float2 (1.0f - i.m_uv0.y, i.m_uv0.x);
    float3 f0 = photo_model.Sample(photo_sampler, uv2).xyz;
    return float4( f0, 1.0f);
}

