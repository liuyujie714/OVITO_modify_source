#version 440

#include "../global_uniforms.glsl"

// Inputs:
layout(location = 0) flat in vec4 color_fs;
layout(location = 1) flat in vec3 center;	// Transformed cone vertex in view coordinates
layout(location = 2) flat in vec3 axis;		// Transformed cone axis in view coordinates
layout(location = 3) flat in float cone_radius;	// The radius of the cone
layout(location = 4) noperspective in vec3 ray_origin;
layout(location = 5) noperspective in vec3 ray_dir;

// Outputs:
layout(location = 0) out vec4 fragColor;

void main()
{
	vec3 ray_dir_norm = normalize(ray_dir);

	float zmin;
	vec3 ray_origin_shifted = ray_origin;
	// This is to improve numeric precision of intersection calculation:
	if(!is_perspective()) {
		zmin = dot(ray_dir_norm, ray_origin - center);
		ray_origin_shifted.z = center.z;
	}
	else {
		zmin = 0.0;
	}

	float height = length(axis);
	float angle = atan(cone_radius / height);
	float cos_angle = cos(angle);
	float sin_angle = sin(angle);
	float cosSqr = cos_angle * cos_angle;
	float AdD = dot(axis, ray_dir_norm) / height;
	vec3 E = ray_origin_shifted - center;
	float AdE = dot(axis, E) / height;
	float DdE = dot(ray_dir_norm, E);
	float EdE = dot(E, E);
	float c2 = AdD*AdD - cosSqr;
	float c1 = AdD*AdE - cosSqr*DdE;
	float c0 = AdE*AdE - cosSqr*EdE;

	// Solve the quadratic. Keep only those X for which dot(A,X-V) >= 0.
	float ray_t = zmin;

	const float epsilon = 1e-9 * cone_radius * cone_radius;
	if(abs(c2) >= epsilon) {
		float discr = c1*c1 - c0*c2;
		if(discr < -epsilon) {
			// Q(t) = 0 has no real-valued roots. The ray does not
			// intersect the double-sided cone.
			discard;
		}
		else if(discr > epsilon) {
			// Q(t) = 0 has two distinct real-valued roots.  However, one or
			// both of them might intersect the portion of the double-sided
			// cone "behind" the vertex.  We are interested only in those
			// intersections "in front" of the vertex.
			float root = sqrt(discr);
			float t = (-c1 - root) / c2;
			E = ray_origin_shifted + t * ray_dir_norm - center;
			float ddot = dot(E, axis);
			if(ddot > 0.0 && ddot < height*height && t > zmin) {
				ray_t = t;
			}
			t = (-c1 + root) / c2;
			vec3 E2 = ray_origin_shifted + t * ray_dir_norm - center;
			ddot = dot(E2, axis);
			if(ddot > 0.0 && ddot < height*height && t > zmin) {
				ray_t = t;
				E = E2;
			}
		}
		else {
			// One repeated real root (line is tangent to the cone).
			float t = -(c1/c2);
			E = ray_origin_shifted + t * ray_dir_norm - center;
			if(dot(E, axis) > 0.0) {
				ray_t = t;
			}
		}
	}
	else if(abs(c1) >= epsilon) {
		// c2 = 0, c1 != 0 (D is a direction vector on the cone boundary)
		float t = -(0.5*c0/c1);
		E = ray_origin_shifted + t * ray_dir_norm - center;
		if(dot(E, axis) > 0.0) {
			ray_t = t;
		}
	}
	else if(abs(c0) >= epsilon) {
		// c2 = c1 = 0, c0 != 0
		discard;
	}
	else if(DdE > 0.0) {
		// c2 = c1 = c0 = 0, cone contains ray V+t*D where V is cone vertex
		// and D is the line direction.
		ray_t = DdE;
	}
	if(ray_t <= zmin)
		discard;

	// Intersection point with cone:
	vec3 view_intersection_pnt = E + center;

	// Compute intersection with disc.
	vec3 disc_center = center + axis;
	vec3 normal = axis;
	const float d = -dot(disc_center, normal);
	float t = -(d + dot(normal, ray_origin_shifted));
	const float td = dot(normal, ray_dir_norm);
	t /= td;
	if(t > zmin && t < ray_t) {
		vec3 hitpnt = ray_origin_shifted + t * ray_dir_norm - disc_center;
		if(dot(hitpnt,hitpnt) < cone_radius*cone_radius) {
			view_intersection_pnt = ray_origin_shifted + t * ray_dir_norm;
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
