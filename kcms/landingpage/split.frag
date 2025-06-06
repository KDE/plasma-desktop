/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#version 440

layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec2 tilt;
} ubuf;

layout(binding = 1) uniform sampler2D uSource1;
layout(binding = 2) uniform sampler2D uSource2;

void main()
{
    float f = dot(ubuf.tilt, vTexCoord - vec2(0.5, 0.5));
    float df = length(vec2(dFdx(f), dFdy(f)));
    float blending = clamp(0.5 + f / df, 0.0, 1.0);

    vec4 p1 = texture(uSource1, vTexCoord);
    vec4 p2 = texture(uSource2, vTexCoord);

    fragColor = mix(p1, p2, blending) * ubuf.qt_Opacity;
}
