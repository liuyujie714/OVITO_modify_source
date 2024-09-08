#version 440

#include "../../global_uniforms.glsl"
#include "../../shading.glsl"

// Inputs:
layout(location = 0) flat in vec4 color_fs;
layout(location = 1) flat in vec3 particle_view_pos_fs;
layout(location = 2) flat in float particle_radius_squared_fs;
layout(location = 3) noperspective in vec3 ray_origin;
layout(location = 4) noperspective in vec3 ray_dir;

// Outputs:
layout(location = 0) out vec4 fragColor;

void main()
{
	vec3 ray_dir_norm = normalize(ray_dir);
	vec3 sphere_dir = particle_view_pos_fs - ray_origin;

	// Perform ray-sphere intersection test.
	float b = dot(ray_dir_norm, sphere_dir);
	vec3 delta = ray_dir_norm * b - sphere_dir;
	float x = dot(delta, delta);
	float disc = particle_radius_squared_fs - x;

	// Only calculate the intersection closest to the viewer.
	if(disc < 0.0)
		discard; // Ray missed sphere entirely, discard fragment

	// Calculate closest intersection position.
	float tnear = b - sqrt(disc);

	// Discard intersections located behind the viewer.
	if(tnear < 0.0)
		discard;

	// Calculate intersection point in view coordinate system.
	vec3 view_intersection_pnt = ray_origin + tnear * ray_dir_norm;

	// Output the ray-sphere intersection point as the fragment depth
	// rather than the depth of the bounding box polygons.
	// The eye coordinate Z value must be transformed to normalized device
	// coordinates before being assigned as the final fragment depth.
	vec4 projected_intersection = GlobalUniforms.projection_matrix * vec4(view_intersection_pnt, 1.0);
	gl_FragDepth = projected_intersection.z / projected_intersection.w;

    // Calculate surface normal in view coordinate system.
    vec3 surface_normal = normalize(view_intersection_pnt - particle_view_pos_fs);
    fragColor = shadeSurfaceColor(surface_normal, color_fs);
}
