#version 440

#include "../../picking.glsl"

// Push constants:
layout(push_constant) uniform constants {
    mat4 mvp;
    int pickingBaseId;
} PushConstants;

// Inputs:
layout(location = 0) in vec3 position;
layout(location = 1) in float radius;

// Outputs:
layout(location = 0) out vec4 color_fs;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	// Const array of vertex positions for the cube triangle strip.
	const vec3 cube[14] = vec3[14](
        vec3( 1.0,  1.0,  1.0),
        vec3( 1.0, -1.0,  1.0),
        vec3( 1.0,  1.0, -1.0),
        vec3( 1.0, -1.0, -1.0),
        vec3(-1.0, -1.0, -1.0),
        vec3( 1.0, -1.0,  1.0),
        vec3(-1.0, -1.0,  1.0),
        vec3( 1.0,  1.0,  1.0),
        vec3(-1.0,  1.0,  1.0),
        vec3( 1.0,  1.0, -1.0),
        vec3(-1.0,  1.0, -1.0),
        vec3(-1.0, -1.0, -1.0),
        vec3(-1.0,  1.0,  1.0),
        vec3(-1.0, -1.0,  1.0)
	);

    // The index of the cube corner.
    int corner = gl_VertexIndex;

	// Apply model-view-projection matrix to particle position displaced by the cube vertex position.
    gl_Position = PushConstants.mvp * vec4(position + cube[corner] * radius, 1.0);

    // Compute color from object ID.
    color_fs = pickingModeColor(PushConstants.pickingBaseId, gl_InstanceIndex);
}
