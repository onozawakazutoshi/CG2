struct VertexShaderOutput
{
    float32_t4 posision : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t3 worldPosition : POSITION0;
};
struct Camera
{
    float32_t3 worldPosition;
};