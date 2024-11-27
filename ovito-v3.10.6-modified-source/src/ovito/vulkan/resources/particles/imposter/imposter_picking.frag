#version 440

#include "../../global_uniforms.glsl"

// Inputs:
layout(location = 0) flat in vec4 color_fs;
layout(location = 1) in vec2 uv_fs;
layout(location = 2) flat in vec2 radius_and_eyez_fs;

// Outputs:
layout(location = 0) out vec4 fragColor;

void main()
{
	// Test if fragment is within the unit circle.
	float rsq = dot(uv_fs, uv_fs);
	if(rsq >= 1.0) discard;

    // Use flat shading in picking mode.
    fragColor = color_fs;

	// Vary the depth value across the imposter to obtain proper intersections between particles.
	float ze = radius_and_eyez_fs.y + sqrt(1.0 - rsq) * radius_and_eyez_fs.x;
	gl_FragDepth = (GlobalUniforms.projection_matrix[2][2] * ze + GlobalUniforms.projection_matrix[3][2]) 
					/ (GlobalUniforms.projection_matrix[2][3] * ze + GlobalUniforms.projection_matrix[3][3]);
}
