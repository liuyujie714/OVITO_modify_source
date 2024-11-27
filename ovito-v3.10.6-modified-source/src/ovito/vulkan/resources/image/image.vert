#version 440

// Push constants:
layout(push_constant) uniform constants {
    vec2 minc;
    vec2 maxc;
} PushConstants;

// Outputs:
out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec2 texcoord_fs;

void main()
{
    // Draw a textured quad:
    if(gl_VertexIndex == 0) {
        gl_Position = vec4(PushConstants.maxc.x, PushConstants.minc.y, 0.0, 1.0);
        texcoord_fs = vec2(1.0, 0.0);
    }
    else if(gl_VertexIndex == 1) {
        gl_Position = vec4(PushConstants.minc.x, PushConstants.minc.y, 0.0, 1.0);
        texcoord_fs = vec2(0.0, 0.0);
    }
    else if(gl_VertexIndex == 2) {
        gl_Position = vec4(PushConstants.maxc.x, PushConstants.maxc.y, 0.0, 1.0);
        texcoord_fs = vec2(1.0, 1.0);
    }
    else if(gl_VertexIndex == 3) {
        gl_Position = vec4(PushConstants.minc.x, PushConstants.maxc.y, 0.0, 1.0);
        texcoord_fs = vec2(0.0, 1.0);
    }
}
