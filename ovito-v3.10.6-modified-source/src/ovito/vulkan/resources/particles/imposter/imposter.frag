#version 440

#include "../../global_uniforms.glsl"
#include "../../shading.glsl"

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

	// Calculate surface normal in view coordinate system.
	vec3 surface_normal = vec3(uv_fs, sqrt(1.0 - rsq));

	// Compute local surface color.
	fragColor = shadeSurfaceColor(surface_normal, color_fs);

	// Vary the depth value across the imposter to obtain proper intersections between particles.
	float ze = radius_and_eyez_fs.y + surface_normal.z * radius_and_eyez_fs.x;
	gl_FragDepth = (GlobalUniforms.projection_matrix[2][2] * ze + GlobalUniforms.projection_matrix[3][2]) 
					/ (GlobalUniforms.projection_matrix[2][3] * ze + GlobalUniforms.projection_matrix[3][3]);
}
