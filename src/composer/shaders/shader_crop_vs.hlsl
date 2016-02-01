cbuffer transform_info : register(b0)
{
    float m_center_x;
    float m_center_y;

    float m_image_width;
    float m_image_height;
};

struct vs_samples_input
{
    float2  m_position  : samples_position;
};

struct vs_samples_output
{
    float4 m_position : SV_Position;
};

//vertex shader just passes through
vs_samples_output main(in vs_samples_input i, in uint vertex_id: VertexID)
{
    vs_samples_output r;
    r.m_position = float4(1.0f, 1.0f, 0.0f, 1.0f);
    return r;
}

