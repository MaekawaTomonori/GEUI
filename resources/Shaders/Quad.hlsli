struct Instance {
    float2 center;
    float2 halfSize;
    float4 color;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};
