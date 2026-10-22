#include "Quad.hlsli"

cbuffer Screen : register(b0) {
    float2 screenSize;
};

StructuredBuffer<Instance> instances : register(t0);

static const float2 CORNERS[6] = {
    float2(-1.0f, -1.0f), float2(1.0f, -1.0f), float2(-1.0f, 1.0f),
    float2(-1.0f, 1.0f), float2(1.0f, -1.0f), float2(1.0f, 1.0f),
};

VSOutput main(uint _vertexId : SV_VertexID, uint _instanceId : SV_InstanceID) {
    Instance instance = instances[_instanceId];
    float2 pixel = instance.center + CORNERS[_vertexId] * instance.halfSize;

    // 入力は左上原点で y が下向きなので反転する
    float2 ndc = pixel / screenSize * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);

    VSOutput output;
    output.position = float4(ndc, 0.0f, 1.0f);
    output.color = instance.color;
    return output;
}
