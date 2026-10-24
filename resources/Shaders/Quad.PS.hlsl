#include "Quad.hlsli"

Texture2D<float4> textures[] : register(t0, space1);
SamplerState textureSampler : register(s0);

// Font.cpp の ON_EDGE と同じ値にする
static const float GLYPH_EDGE = 180.0f / 255.0f;

float4 main(VSOutput _input) : SV_TARGET {
    if (_input.texIndex == NO_TEXTURE) {
        return _input.color;
    }

    // 1回の描画の中でインスタンスごとに番号が変わる
    float4 texel = textures[NonUniformResourceIndex(_input.texIndex)].Sample(textureSampler, _input.uv);

    if (_input.kind == KIND_GLYPH) {
        // 境目を下げるほど、輪郭の外側まで塗られて太くなる
        float edge = GLYPH_EDGE - _input.weight;
        float aa = fwidth(texel.r) * 0.75f;
        float alpha = smoothstep(edge - aa, edge + aa, texel.r);
        return float4(_input.color.rgb, _input.color.a * alpha);
    }

    return texel * _input.color;
}
