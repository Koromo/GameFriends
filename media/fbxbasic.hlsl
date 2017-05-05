struct VSIn
{
    float4 pos : POSITION;
};

struct VSOut
{
    float4 pos : SV_POSITION;
};

cbuffer Constants : register(b0)
{
    matrix _World;
    matrix _View;
    matrix _Proj;
}

cbuffer Color : register(b0)
{
    float4 diffuse = float4(0, 0.5, 0.5, 1);
}

VSOut vsMain(VSIn In)
{
    VSOut Out;
    Out.pos = mul(In.pos, _World);
    Out.pos = mul(Out.pos, _View);
    Out.pos = mul(Out.pos, _Proj);
    return Out;
}

float4 psMain_0(VSOut In) : SV_TARGET0
{
    return diffuse;
}
