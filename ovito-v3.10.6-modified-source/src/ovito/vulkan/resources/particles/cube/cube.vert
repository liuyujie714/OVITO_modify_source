#version 440

// Push constants:
layout(push_constant) uniform constants {
    mat4 mvp;
    mat4 normal_tm;
} PushConstants;

// Inputs:
layout(location = 0) in vec3 position;
layout(location = 1) in float radius;
layout(location = 2) in vec4 color;

// Outputs:
layout(location = 0) out vec4 color_fs;
layout(location = 1) out vec3 normal_fs;
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

	// Const array of vertex normals for the cube triangle strip.
    // Note the difference between Vulkan and OpenGL.
	const vec3 normals[14] = vec3[14](
        vec3( 1.0,  0.0,  0.0),
        vec3( 1.0,  0.0,  0.0),
        vec3( 0.0,  0.0, -1.0),
        vec3( 0.0, -1.0,  0.0),
        vec3( 0.0, -1.0,  0.0),
        vec3( 0.0,  0.0,  1.0),
        vec3( 0.0,  0.0,  1.0),
        vec3( 0.0,  1.0,  0.0),
        vec3( 0.0,  1.0,  0.0),
        vec3( 0.0,  0.0, -1.0),
        vec3(-1.0,  0.0,  0.0),
        vec3(-1.0,  0.0,  0.0),
        vec3( 1.0,  0.0,  0.0),
        vec3( 1.0,  0.0,  0.0)
    );

    // The index of the cube corner.
    int corner = gl_VertexIndex;

	// Apply model-view-projection matrix to particle position displaced by the cube vertex position.
    gl_Position = PushConstants.mvp * vec4(position + cube[corner] * radius, 1.0);

    // Forward particle color to fragment shader.
    color_fs = color;

    // Transform local vertex normal.
    normal_fs = vec3(PushConstants.normal_tm * vec4(normals[corner], 0.0));
}
