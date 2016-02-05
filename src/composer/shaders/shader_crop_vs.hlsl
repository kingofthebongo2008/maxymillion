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

//get full screen triangle that works better than full screen quad, due to better rasterization utilization
//outputs vertices : (-1,-1, 0 ), ( -1, 3, 0 ), ( 3, -1, 0 ), vertex_id = 0, 1, 2
//see vertex-shader-tricks-bill-bilodeau
float4 triangle_screen_space( uint vertex_id )
{
    float4 r;
    r.x = (float) ( vertex_id >> 1) * 4.0f - 1.0f;
    r.y = (float) ( vertex_id & 0x1 ) * 4.0f - 1.0f;
    r.z = 0.0f;
    r.w = 1.0;
    return r;
}

//get full screen triangle that works better than full screen quad, due to better rasterization utilization
//outputs vertices : ( 0 , 1 ), ( 0, -1 ), ( 2 , 1 ) vertex_id = 0, 1, 2
//see vertex-shader-tricks-bill-bilodeau
float2 triangle_uv(uint vertex_id)
{
    float2 r;

    r.x = (float)( vertex_id  >> 1   ) * 2.0f;
    r.y = 1.0f - (float)( vertex_id  &0x1 ) * 2.0f;

    return r;
}

float dot(float3 a, float3 b)
{
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

float3x3 translate(float2 t)
{
    return float3x3(float3(1.0, 0.0f, t.x), float3(0.0, 1.0f, t.y), float3(0.0, 0.0f, 1));
}

float3x3 scale(float2 t)
{
    return float3x3(float3(t.x, 0.0f, 0.0f), float3(0.0, t.y, 0.0f), float3(0.0, 0.0f, 1));
}

float3 mul(float3 v, float3x3 m)
{
    float3 r;

    r.x = dot(v, m[0]);
    r.y = dot(v, m[1]);
    r.z = dot(v, m[2]);

    return r;
}

//vertex shader just passes through
vs_samples_output main( in uint vertex_id: SV_VertexID)
{
    vs_samples_output r;
    r.m_position = triangle_screen_space(vertex_id);
    r.m_uv0 = triangle_uv(vertex_id);

    float3x3 t0 = translate(float2(-0.5, 0.0f));
    
#ifdef HORIZONTAL
    float3x3 t1 = scale(float2( 2285.0f/ 2464.0f, 1.0f));
#else
    float3x3 t1 = scale(float2(1.0f, 2285.0f / 2464.0f) );
#endif
    float3x3 t2 = translate(float2(0.5, 0.0f));

    r.m_uv0 = triangle_uv(vertex_id);

    //scale the uvs to crop the picture
    float3 uv1 = float3(r.m_uv0, 1.0f);

    uv1 = mul(uv1, t0);
    uv1 = mul(uv1, t1);
    uv1 = mul(uv1, t2);

    r.m_uv1 = uv1.xy;

    return r;
}

