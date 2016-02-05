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

float4 main(in vs_samples_output i) : sv_target0
{
    return float4(0.0f, 1.0f, 0.0f, 1.0f);
}

