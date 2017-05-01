struct VSIn
{
    float4 pos : POSITION;
    float4 color : COLOR;
};

struct VSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

cbuffer Mats : register(b0)
{
    matrix viewProj;
}

VSOut vsMain(VSIn In)
{
    VSOut Out;
    Out.pos = mul(In.pos, viewProj);
    Out.color = In.color;
    return Out;
}

float4 psMain(VSOut In) : SV_TARGET0
{
    return In.color;
}
