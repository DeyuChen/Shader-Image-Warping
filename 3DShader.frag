#version 130
// It was expressed that some drivers required this next line to function properly
precision highp float;

in  vec3 ex_Color;
out vec4 fragColor;

void main(void) {
    // Pass through our original color with full opacity.
    fragColor = vec4(ex_Color, 1.0);
}
