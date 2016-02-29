cbuffer transparency_color : register(b0)
{
    float4 m_color;
};

float3 unpack_transparency2(float4 v )
{
    return float3(v.xyz);
}

struct vs_samples_output
{
    float4 m_position : SV_Position;
    float2 m_uv0      : texcoord0;
    float2 m_uv1      : texcoord1;
};

Texture2D		photo_model: register(t0);
Texture2D		photo : register(t1);
SamplerState	photo_sampler : register(s0);

float4 main(in vs_samples_output i) : sv_target0
{
    float3 f0 = photo_model.Sample(photo_sampler, i.m_uv0).xyz;
    float3 f1 = photo.Sample(photo_sampler, i.m_uv1).xyz;

    float3 mask = float3 ( 197 / 255.0f, 201 / 255.0f, 195 / 255.0f  );

    float3 transparency = unpack_transparency2(m_color);
    float3 mask2 = transparency;
    float3 d = abs(f0 - mask2);
    float  r = dot(d, float3(1.0f, 1.0f, 1.0f));

    float3 f = f0;
    if ( r == 0.0f )
    {
        f = f1;
    }

    return float4(f, 1.0f);
}

