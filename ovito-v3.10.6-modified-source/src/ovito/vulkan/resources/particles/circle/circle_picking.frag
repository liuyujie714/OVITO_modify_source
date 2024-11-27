#version 440

// Inputs:
layout(location = 0) flat in vec4 color_fs;
layout(location = 1) in vec2 uv_fs;

// Outputs:
layout(location = 0) out vec4 fragColor;

void main()
{
	// Test if fragment is within the unit circle.
	float rsq = dot(uv_fs, uv_fs);
	if(rsq >= 1.0) discard;

    // Use flat shading in picking mode.
    fragColor = color_fs;
}
