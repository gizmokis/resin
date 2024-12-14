struct material {
    vec3 albedo;

	float ka;
	float kd;
	float ks;
	float m;
};

material material_mix(material m1, material m2, float k) {
    return material(mix(m1.albedo, m2.albedo, k), mix(m1.ka, m2.ka, k), mix(m1.kd, m2.kd, k), mix(m1.ks, m2.ks, k), mix(m1.m, m2.m, k));
}

struct attenuation {
	float constant;
	float linear;
	float quadratic;
};

struct directional_light {
	vec3 color;
	vec3 dir;
	float ambient_impact;
}; 

struct point_light {
	vec3 color;
	vec3 pos;

	attenuation atten;
}; 

float calc_attenuation(attenuation atten, float dist) {
	return 1.0 / (atten.constant + dist * (atten.linear + dist * atten.quadratic));
}

vec2 calc_light(vec3 to_light, vec3 normal, vec3 view_dir) {
	float diff = max(dot(normal, to_light), 0.0);

	vec3 halfway_dir = normalize(to_light + view_dir);
	float spec = max(dot(normal, halfway_dir), 0.0);
	
	return vec2(diff, spec);
}

vec3 calc_point_light(point_light light, material mat, vec3 normal, vec3 view_dir, vec3 frag_pos) {
	vec2 factors = calc_light(normalize(light.pos - frag_pos), normal, view_dir);

	return calc_attenuation(light.atten, length(light.pos - frag_pos)) * (mat.kd * factors.x + mat.ks * pow(factors.y, mat.m)) * light.color;
}

vec3 calc_dir_light(directional_light light, material mat, vec3 normal, vec3 view_dir) {
	vec2 factors = calc_light(-light.dir, normal, view_dir);

	return (mat.kd * factors.x + mat.ks * pow(factors.y, mat.m)) * light.color;
}
