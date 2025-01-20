#version 430 core

layout(location = 0) out vec4 fragColor;
layout(location = 1) out int id;

// fragment
in vec2 v_Pos;

// camera
uniform mat4 u_iV;
uniform bool u_ortho;
uniform float u_nearPlane;
uniform float u_farPlane;

#include "blinn_phong.glsl"
#include "sdf.glsl"
#external_definition SDF_CODE

// rendering
const vec3 u_Ambient = vec3(0.25,0.25,0.25);

uniform directional_light u_dirLight;
uniform point_light u_pointLight;

sdf_result map( vec3 pos )
{
    return SDF_CODE;
}

float raycast(vec3 ray_origin, vec3 ray_direction, out sdf_result hit)
{
    float tmin = u_nearPlane;
    float tmax = u_farPlane;

    float t = tmin;
    for( int i=0; i<256 && t<tmax; i++ )
    {
        vec3 pos = ray_origin + t*ray_direction;
        sdf_result res = map(pos);
        if( abs(res.dist)<(0.0001*t) )
        { 
            hit = res;
            return t;
        }
        t += res.dist;
    }
    
    return u_farPlane;
}

// https://iquilezles.org/articles/normalsSDF
vec3 calcNormal( in vec3 pos )
{
    // inspired by tdhooper and klems - a way to prevent the compiler from inlining map() 4 times
    vec3 n = vec3(0.0);
    for( int i=0; i<4; i++ )
    {
        vec3 e = 0.5773*(2.0*vec3((((i+3)>>1)&1),((i>>1)&1),(i&1))-1.0);
        n += e*map(pos+0.0005*e).dist;
      //if( n.x+n.y+n.z>100.0 ) break;
    }
    return normalize(n); 
}

float render( vec3 ray_origin, vec3 ray_direction )
{ 
    fragColor = vec4(0.0);
    id = -1;

    sdf_result result;
    float t = raycast(ray_origin, ray_direction, result);
    if( t>-0.5 && t < u_farPlane )
    {
        vec3 pos = ray_origin + t*ray_direction;
        vec3 nor = calcNormal( pos );
        material mat = result.mat;

        vec3 totalAmbient = u_Ambient + u_dirLight.ambient_impact * u_dirLight.color;
        vec3 light = mat.ka * totalAmbient
            + calc_dir_light(u_dirLight, mat, nor, -ray_direction)
            + calc_point_light(u_pointLight, mat, nor, -ray_direction, pos);
		fragColor = vec4(light * mat.albedo, 1.0);
        id = result.id;
    }
    return t;
}

void main() {
    vec4 ray_origin;    // vec4(ro, 1) as it needs to be translated
    vec4 ray_direction; // vec4(rd, 0) as it cannot be translated
    if (u_ortho) {
        ray_origin = vec4(v_Pos, 0, 1);
        ray_direction = vec4(0, 0, -1, 0);
    } else {
        ray_origin = vec4(0, 0, 0, 1); 
        ray_direction = normalize(vec4(v_Pos, -u_nearPlane, 0));
    }
    
    float t = render((u_iV * ray_origin).xyz, ((u_iV * ray_direction).xyz));

    // Depth
    float A = u_nearPlane * u_farPlane;
    float B = u_farPlane - u_nearPlane;
    if (u_ortho) {
        gl_FragDepth = (t - u_nearPlane) / B;
    } else {
        gl_FragDepth = (A/(t * ray_direction.z) + u_farPlane) / B;
    }
}
