static const uint NO_TEXTURE = 0xFFFFFFFF;

struct Instance {
    float2 center;
    float2 halfSize;
    float4 color;
    uint texIndex;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    nointerpolation uint texIndex : TEXINDEX0;
};
