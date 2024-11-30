struct material {
    vec3 albedo;

	float metallic;
	float roughness;
	float ao;

    vec3 F0;
};

material material_mix(material m1, material m2, float k) {
    return material(mix(m1.albedo, m2.albedo, k), mix(m1.metallic, m2.metallic, k), mix(m1.roughness, m2.roughness, k), mix(m1.ao, m2.ao, k), vec3(0));
}

struct attenuation {
	float constant;
	float linear;
	float quadratic;
};

struct directional_light {
	vec3 color;
	vec3 dir;
}; 

struct point_light {
	vec3 color;
	vec3 pos;

	attenuation atten;
}; 

float calc_attenuation(attenuation atten, float dist) {
	return 1.0 / (atten.constant + dist * (atten.linear + dist * atten.quadratic));
}

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 calc_light(material mat, vec3 to_light, vec3 normal, vec3 view_dir) {
	vec3 halfway_dir = normalize(to_light + view_dir);

    // cook-torrance brdf
    float NDF = DistributionGGX(normal, halfway_dir, mat.roughness);       
    float G   = GeometrySmith(normal, view_dir, to_light, mat.roughness);       
    vec3 F    = fresnelSchlick(max(dot(halfway_dir, view_dir), 0.0), mat.F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - mat.metallic;

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, view_dir), 0.0) * max(dot(normal, to_light), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;  
        
    float NdotL = max(dot(normal, to_light), 0.0);                
    return (kD * mat.albedo / PI + specular) * NdotL; 
}

vec3 calc_point_light(point_light light, material mat, vec3 normal, vec3 view_dir, vec3 frag_pos) {
	vec3 pbr = calc_light(mat, normalize(light.pos - frag_pos), normal, view_dir);

	return calc_attenuation(light.atten, length(light.pos - frag_pos)) * pbr * light.color;
}

vec3 calc_dir_light(directional_light light, material mat, vec3 normal, vec3 view_dir) {
	vec3 pbr = calc_light(mat, -light.dir, normal, view_dir);

	return pbr * light.color;
}
