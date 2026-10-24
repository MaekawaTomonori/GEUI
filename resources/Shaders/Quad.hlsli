static const uint NO_TEXTURE = 0xFFFFFFFF;
static const uint KIND_QUAD = 0;
static const uint KIND_GLYPH = 1;

struct Instance {
    float2 center;
    float2 halfSize;
    float4 color;
    float4 uvRect;
    uint texIndex;
    uint kind;
    float weight;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    nointerpolation uint texIndex : TEXINDEX0;
    nointerpolation uint kind : KIND0;
    nointerpolation float weight : WEIGHT0;
};
