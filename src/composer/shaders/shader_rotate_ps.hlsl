cbuffer transform_info : register(b0)
{
    float m_center_x;
    float m_center_y;

    float m_image_width;
    float m_image_height;
};

struct vs_samples_output
{
    float4 m_position : SV_Position;
    float2 m_uv0      : texcoord0;
    float2 m_uv1      : texcoord1;
};

Texture2D		photo_model: register(t0);

SamplerState	photo_sampler : register(s0);

float4 main(in vs_samples_output i) : sv_target0
{
    float2 uv2 = float2 (1.0f - i.m_uv0.y, i.m_uv0.x);
    float3 f0 = photo_model.Sample(photo_sampler, uv2).xyz;
    return float4( f0, 1.0f);
}

