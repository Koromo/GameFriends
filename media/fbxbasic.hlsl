struct VSIn
{
    float4 pos : POSITION;
};

struct VSOut
{
    float4 pos : SV_POSITION;
};

cbuffer Mats : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
}

cbuffer Color : register(b0)
{
    float4 diffuse = float4(0, 0.5, 0.5, 1);
}

VSOut vsMain(VSIn In)
{
    VSOut Out;
    Out.pos = mul(In.pos, world);
    Out.pos = mul(Out.pos, view);
    Out.pos = mul(Out.pos, proj);
    return Out;
}

float4 psMain_0(VSOut In) : SV_TARGET0
{
    return diffuse;
}
