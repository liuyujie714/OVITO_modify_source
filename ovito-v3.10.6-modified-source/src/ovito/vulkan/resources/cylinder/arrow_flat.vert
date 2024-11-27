#version 440

#include "../global_uniforms.glsl"

// Push constants:
layout(push_constant) uniform constants {
    mat4 mvp;
    vec4 view_dir_eye_pos; // Either camera viewing direction (parallel) or camera position (perspective) in object space coordinates.
} PushConstants;

// Inputs:
layout(location = 0) in vec3 base;
layout(location = 1) in vec3 head;
layout(location = 2) in float radius;
layout(location = 3) in vec4 color1;
layout(location = 4) in vec4 color2;

// Outputs:
layout(location = 0) out vec4 color_fs;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
    // Vector pointing from camera to cylinder base in object space:
	vec3 view_dir;
	if(!is_perspective())
		view_dir = PushConstants.view_dir_eye_pos.xyz;
	else
		view_dir = PushConstants.view_dir_eye_pos.xyz - base;

	// Build local coordinate system in object space.
    mat2x3 uv_tm;
	uv_tm[0] = head - base;
    uv_tm[1] = normalize(cross(view_dir, uv_tm[0])) * radius;

    vec2 vpos;
    float arrowHeadRadius = 2.5;
    float arrowHeadLength = (radius * arrowHeadRadius * 1.8) / length(uv_tm[0]);
    if(arrowHeadLength < 1.0) {
        switch(gl_VertexIndex) {
            case 0: vpos = vec2(1.0, 0.0); break;
            case 1: vpos = vec2(1.0 - arrowHeadLength, arrowHeadRadius); break;
            case 2: vpos = vec2(1.0 - arrowHeadLength, 1.0); break;
            case 3: vpos = vec2(0.0, 1.0); break;
            case 4: vpos = vec2(0.0,-1.0); break;
            case 5: vpos = vec2(1.0 - arrowHeadLength,-1.0); break;
            case 6: vpos = vec2(1.0 - arrowHeadLength,-arrowHeadRadius); break;
        }
    }
    else {
        switch(gl_VertexIndex) {
            case 0: vpos = vec2(1.0, 0.0); break;
            case 1: vpos = vec2(0.0, arrowHeadRadius / arrowHeadLength); break;
            case 6: vpos = vec2(0.0,-arrowHeadRadius / arrowHeadLength); break;
            default: vpos = vec2(0.0, 0.0); break;
        }
    }

	// Project corner vertex.
    gl_Position = PushConstants.mvp * vec4(base + uv_tm * vpos, 1.0);

    // Forward primitive color to fragment shader.
    color_fs = color1;
}
