#include "Particle.hlsli"
struct ParticleForGPU
{
    float32_t4x4 WVP;
    float32_t4x4 World;
    float32_t4 color;
};
StructuredBuffer<ParticleForGPU> gParticle : register(t0);

struct VertexShaderInput
{
    float32_t4 posision : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t4 color : COLOR0;
};

VertexShaderOutput main(VertexShaderInput input,uint32_t instanceId:SV_InstanceID)
{
    VertexShaderOutput output;
    output.posision = mul(input.posision, gParticle[instanceId].WVP);
    output.texcoord = input.texcoord;
    output.color = gParticle[instanceId].color;
    
    return output;
}