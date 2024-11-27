#version 440

#include "../../global_uniforms.glsl"
#include "../../shading.glsl"

// Inputs:
layout(location = 0) flat in vec4 color_fs;
layout(location = 1) flat in mat3 view_to_sphere_fs;
layout(location = 4) flat in vec3 particle_view_pos_fs;
layout(location = 5) noperspective in vec3 ray_origin;
layout(location = 6) noperspective in vec3 ray_dir;

// Outputs:
layout(location = 0) out vec4 fragColor;

void main()
{
	vec3 sphere_dir = view_to_sphere_fs * (particle_view_pos_fs - ray_origin);

	// Ray direction in sphere coordinate system.
	vec3 ray_dir2 = normalize(view_to_sphere_fs * ray_dir);

	// Perform ray-sphere intersection test.
	float b = dot(ray_dir2, sphere_dir);
	vec3 delta = ray_dir2 * b - sphere_dir;
	float x = dot(delta, delta);
	float disc = 1.0 - x;

	// Only calculate the intersection closest to the viewer.
	if(disc < 0.0)
		discard; // Ray missed sphere entirely, discard fragment

	// Calculate closest intersection position.
	float tnear = b - sqrt(disc);

	// Discard intersections located behind the viewer.
	if(tnear < 0.0)
		discard;

	// Calculate intersection point in sphere coordinate system.
	vec3 sphere_intersection_pnt = tnear * ray_dir2 - sphere_dir;

	// Calculate intersection point in view coordinate system.
    vec3 view_intersection_pnt = inverse(view_to_sphere_fs) * sphere_intersection_pnt + particle_view_pos_fs;

	// Output the ray-sphere intersection point as the fragment depth
	// rather than the depth of the bounding box polygons.
	// The eye coordinate Z value must be transformed to normalized device
	// coordinates before being assigned as the final fragment depth.
	vec4 projected_intersection = GlobalUniforms.projection_matrix * vec4(view_intersection_pnt, 1.0);
	gl_FragDepth = projected_intersection.z / projected_intersection.w;

    // Calculate surface normal in view coordinate system.
    vec3 surface_normal = normalize(sphere_intersection_pnt * view_to_sphere_fs);
    fragColor = shadeSurfaceColor(surface_normal, color_fs);
}
