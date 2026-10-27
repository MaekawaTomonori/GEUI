#include "Quad.hlsli"

Texture2D<float4> textures[] : register(t0, space1);
SamplerState textureSampler : register(s0);

cbuffer Output : register(b1) {
    uint linearize;
};

// Font.cpp の ON_EDGE と同じ値にする
static const float GLYPH_EDGE = 180.0f / 255.0f;

// 透明度と文字の SDF は割合なので変換しない。2.2 は sRGB の曲線の一般的な近似
float4 ToOutput(float4 _color) {
    return linearize != 0 ? float4(pow(_color.rgb, 2.2f), _color.a) : _color;
}

float4 main(VSOutput _input) : SV_TARGET {
    if (_input.texIndex == NO_TEXTURE) {
        return ToOutput(_input.color);
    }

    // 1回の描画の中でインスタンスごとに番号が変わる
    float4 texel = textures[NonUniformResourceIndex(_input.texIndex)].Sample(textureSampler, _input.uv);

    if (_input.kind == KIND_GLYPH) {
        // 境目を下げるほど、輪郭の外側まで塗られて太くなる
        float edge = GLYPH_EDGE - _input.weight;
        float aa = fwidth(texel.r) * 0.75f;
        float alpha = smoothstep(edge - aa, edge + aa, texel.r);
        return ToOutput(float4(_input.color.rgb, _input.color.a * alpha));
    }

    return ToOutput(texel * _input.color);
}
