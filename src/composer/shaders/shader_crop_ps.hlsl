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
    float2 m_uv       : texcoord;
};

Texture2D		photo_model: register(t0);
SamplerState	photo_sampler : register(s0);

float4 main(in vs_samples_output i) : sv_target0
{
    float3 f = photo_model.Sample(photo_sampler, i.m_uv).xyz;

    float3 mask = float3 ( 197 / 255.0f, 201 / 255.0f, 195 / 255.0f  );

    float3 d = abs(f - mask);
    float  r = dot(d, float3(1.0f, 1.0f, 1.0f));

    if ( r == 0.0f )
    {
        f = float3(1.0f, 0.0f, 0.0f);
    }

    return float4( f, 1.0f);
}

