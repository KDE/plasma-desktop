/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    float qt_Opacity;
};
layout(binding = 1) uniform sampler2D source;
layout(binding = 2) uniform sampler2D mask;
void main() {
    fragColor = texture(source, qt_TexCoord0.st) * (1.0 - (texture(mask, qt_TexCoord0.st).a)) * qt_Opacity;
}
