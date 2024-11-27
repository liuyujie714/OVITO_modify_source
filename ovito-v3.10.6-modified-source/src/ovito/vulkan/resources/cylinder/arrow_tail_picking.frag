#version 440

#include "../global_uniforms.glsl"

// Inputs:
layout(location = 0) flat in vec4 color_fs;
layout(location = 1) flat in vec3 cylinder_view_base;		// Transformed cylinder position in view coordinates
layout(location = 2) flat in vec3 cylinder_view_axis;		// Transformed cylinder axis in view coordinates
layout(location = 3) flat in float cylinder_radius_sq_fs;	// The squared radius of the cylinder
layout(location = 4) flat in float cylinder_length;			// The length of the cylinder
layout(location = 5) noperspective in vec3 ray_origin;
layout(location = 6) noperspective in vec3 ray_dir;

// Outputs:
layout(location = 0) out vec4 fragColor;

void main()
{
	vec3 ray_dir_norm = normalize(ray_dir);

	// Perform ray-cylinder intersection test.
	vec3 n = cross(ray_dir_norm, cylinder_view_axis);
	float ln = length(n);
	vec3 RC = ray_origin - cylinder_view_base;

	vec3 view_intersection_pnt = ray_origin;

	bool skip = false;

	if(ln < 1e-7 * cylinder_length) {
		// Handle case where view ray is parallel to cylinder axis:

		float t = dot(RC, ray_dir_norm);
		float v = dot(RC, RC);
		if(v-t*t > cylinder_radius_sq_fs) {
			discard;
		}
		else {
			view_intersection_pnt -= t * ray_dir_norm;
			float tfar = dot(cylinder_view_axis, ray_dir_norm);
			if(tfar < 0.0) {
				view_intersection_pnt += tfar * ray_dir_norm;
			}
		}
	}
	else {

		n /= ln;
		float d = dot(RC,n);
		d *= d;

		// Test if ray missed the cylinder.
		if(d > cylinder_radius_sq_fs) {
			discard;
		}
		else {

			// Calculate closest intersection position.
			float t = dot(cross(cylinder_view_axis, RC), n) / ln;
			float s = abs(sqrt(cylinder_radius_sq_fs - d) / dot(cross(n, cylinder_view_axis), ray_dir_norm) * cylinder_length);
			float tnear = t - s;

			// Calculate intersection point in view coordinate system.
			view_intersection_pnt += tnear * ray_dir_norm;

			// Find intersection position along cylinder axis.
			float anear = dot(view_intersection_pnt - cylinder_view_base, cylinder_view_axis) / (cylinder_length*cylinder_length);
			if(anear >= 0.0 && anear <= 1.0) {
			}
			else {
				// Calculate second intersection point.
				float tfar = t + s;
				vec3 far_view_intersection_pnt = ray_origin + tfar * ray_dir_norm;
				float afar = dot(far_view_intersection_pnt - cylinder_view_base, cylinder_view_axis) / (cylinder_length*cylinder_length);

				// Compute intersection with cylinder caps.
				if(anear < 0.0 && afar > 0.0) {
					view_intersection_pnt += (anear / (anear - afar) * 2.0 * s + 1e-6 * ln) * ray_dir_norm;
				}
				else if(anear > 1.0 && afar < 1.0) {
					view_intersection_pnt += ((anear - 1.0) / (anear - afar) * 2.0 * s + 1e-6 * ln) * ray_dir_norm;
				}
				else {
					discard;
				}
			}
		}
	}

	// Output the ray-cylinder intersection point as the fragment depth
	// rather than the depth of the bounding box polygons.
	// The eye coordinate Z value must be transformed to normalized device
	// coordinates before being assigned as the final fragment depth.
	vec4 projected_intersection = GlobalUniforms.projection_matrix * vec4(view_intersection_pnt, 1.0);
	gl_FragDepth = projected_intersection.z / projected_intersection.w;

    // Flat shading:
    fragColor = color_fs;
}
