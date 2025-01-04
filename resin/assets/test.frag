#version 330 core

layout(location = 0) out vec4 fragColor;

// fragment
in vec2 v_Pos;
in vec2 v_ndcPos;

// camera
uniform mat4 u_iV;
uniform bool u_ortho;
uniform float u_nearPlane;
uniform float u_farPlane;
uniform mat4 u_iP; //temp

#include "blinn_phong.glsl"
#include "sdf.glsl"
#external_definition SDF_CODE
#external_definition MAX_UBO_NODE_COUNT

// rendering
const vec3 u_Ambient = vec3(0.25,0.25,0.25);
const int kMaxNodeCount = MAX_UBO_NODE_COUNT;
layout (std140) uniform Data 
{
    node u_sdf_primitives[kMaxNodeCount];
};

uniform directional_light u_dirLight;
uniform point_light u_pointLight;

// objects
uniform mat4 u_iM;
uniform float u_scale;        
uniform material u_sphereMat;
uniform material u_cubeMat;

vec2 map( vec3 pos )
{
    return SDF_CODE;
}

vec2 raycast( vec3 ray_origin, vec3 ray_direction )
{
    vec2 result = vec2(-1.0,-1.0);

    float tmin = u_nearPlane;
    float tmax = u_farPlane;

    float t = tmin;
    for( int i=0; i<256 && t<tmax; i++ )
    {
        vec3 pos = ray_origin + t*ray_direction;
        vec2 dist = map(pos);
        if( abs(dist.x)<(0.0001*t) )
        { 
            result = vec2(t,dist.y); 
            break;
        }
        t += dist.x;
    }
    
    return result;
}

// https://iquilezles.org/articles/normalsSDF
vec3 calcNormal( in vec3 pos )
{
    // inspired by tdhooper and klems - a way to prevent the compiler from inlining map() 4 times
    vec3 n = vec3(0.0);
    for( int i=0; i<4; i++ )
    {
        vec3 e = 0.5773*(2.0*vec3((((i+3)>>1)&1),((i>>1)&1),(i&1))-1.0);
        n += e*map(pos+0.0005*e).x;
      //if( n.x+n.y+n.z>100.0 ) break;
    }
    return normalize(n); 
}

vec3 render( vec3 ray_origin, vec3 ray_direction )
{ 
    vec3 col = u_Ambient;

    vec2 res = raycast(ray_origin, ray_direction);
    float t = res.x;
	float m = res.y;
    if( m>-0.5 )
    {
        vec3 pos = ray_origin + t*ray_direction;
        vec3 nor = calcNormal( pos );
        material mat = material_mix(u_cubeMat, u_sphereMat, m);

        vec3 totalAmbient = u_Ambient + u_dirLight.ambient_impact * u_dirLight.color;
        vec3 light = mat.ka * totalAmbient
            + calc_dir_light(u_dirLight, mat, nor, -ray_direction)
            + calc_point_light(u_pointLight, mat, nor, -ray_direction, pos);
		col = light * mat.albedo;
    }

	return col;
}

void main() {

    vec4 ray_origin;    // vec4(ro, 1) as it needs to be translated
    vec4 ray_direction; // vec4(rd, 0) as it cannot be translated
    if (u_ortho) {

        vec2 pos = v_Pos;
        ray_origin = vec4(vec3(pos, 0), 1);
        ray_direction = vec4(0, 0, -1, 0);
    } else {
        
        vec2 pos = v_ndcPos;
        ray_origin = vec4(0, 0, 0, 1); 
        ray_direction = vec4(normalize(vec3(pos, -u_nearPlane)), 0);

        // begin: temp
        vec4 temp = u_iP * vec4(pos, 1.0, 1.0); 
        temp /= temp.w;
        ray_direction = vec4(normalize(temp.xyz), 0);
        // end: temp
    }
    
    vec3 color = render((u_iV * ray_origin).xyz, ((u_iV * ray_direction).xyz));
    fragColor = vec4(color, 1.0 );
}
