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
};

//get full screen triangle that works better than full screen quad, due to better rasterization utilization
//outputs vertices : (-1,-1,0), (-1, 3, 0 ), (3, -1, 0 ), vertex_id = 0, 1, 2
//see vertex-shader-tricks-bill-bilodeau
float4 triangle_screen_space( uint vertex_id )
{
    float4 r;
    r.x = (float) ( vertex_id >> 1 ) * 4.0f - 1.0f;
    r.y = (float) ( vertex_id & 0x1 ) * 4.0f - 1.0f;
    r.z = 0.0f;
    r.w = 1.0;
    return r;
}

//get full screen triangle that works better than full screen quad, due to better rasterization utilization
//outputs vertices : (0 , 1 ), ( 0, -1 ), ( 2 , 1 ) vertex_id = 0, 1, 2
//see vertex-shader-tricks-bill-bilodeau
float2 triangle_uv(uint vertex_id)
{
    float2 r;

    r.x = (float)( vertex_id  >> 1  ) * 2.0f ;
    r.y = 1.0f - (float)( vertex_id  & 0x1 ) * 2.0f;

    return r;
}

//vertex shader just passes through
vs_samples_output main( in uint vertex_id: SV_VertexID)
{
    vs_samples_output r;
    r.m_position = triangle_screen_space(vertex_id);
    
    return r;
}

