#include "Quad.hlsli"

Texture2D<float4> textures[] : register(t0, space1);
SamplerState textureSampler : register(s0);

float4 main(VSOutput _input) : SV_TARGET {
    if (_input.texIndex == NO_TEXTURE) {
        return _input.color;
    }

    // 1回の描画の中でインスタンスごとに番号が変わる
    return textures[NonUniformResourceIndex(_input.texIndex)].Sample(textureSampler, _input.uv) * _input.color;
}
