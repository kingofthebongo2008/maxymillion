#ifndef __shader_full_screen_h__
#define __shader_full_screen_h__

 
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

#endif