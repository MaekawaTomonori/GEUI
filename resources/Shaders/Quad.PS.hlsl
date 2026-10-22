#include "Quad.hlsli"

float4 main(VSOutput _input) : SV_TARGET {
    return _input.color;
}
