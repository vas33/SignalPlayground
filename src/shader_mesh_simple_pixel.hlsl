//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************


struct PSInput
{
    float4 position    : SV_POSITION;
    float2 uv        : TEXCOORD0;
};

Texture2D        g_txDiffuse : register(t0);
SamplerState    g_sampler : register(s0);


float4 PSMain(PSInput input) : SV_TARGET
{
    //normalizred color antrazit
    float4 color= float4(0.21f, 0.24f, 0.25f, 0.f);
    // g_txDiffuse.Sample(g_sampler, input.uv);

    return color;
}
