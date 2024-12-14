#version 330 core

#include "sdf.glsl"
#external_definition SDF_CODE

layout(location = 0) out vec4 fragColor;

// fragment
in vec3 v_Pos;

// camera
uniform mat4 u_iV;
uniform bool u_ortho;
uniform vec2 u_resolution;
uniform float u_nearPlane;
uniform float u_farPlane;
uniform float u_camSize;

// rendering
const vec3 u_Ambient = vec3(0.25,0.25,0.25);
uniform directional_light u_dirLight;
uniform point_light u_pointLight;

// objects
uniform mat4 u_transforms[100];
uniform sdf_primitive u_sdf_primitives[100];

sdf_result map( vec3 pos3 )
{
    vec4 pos = vec4(pos3, 1.0);
    // return opSmoothUnion(sdCube(u_transforms[1]*pos, u_sdf_primitives[1]), sdSphere(u_transforms[0]*pos, u_sdf_primitives[0]));
    return SDF_CODE;
}

sdf_result raycast( vec3 ray_origin, vec3 ray_direction )
{
    sdf_result result;
    result.mat.ka = -1.0;

    float tmin = u_nearPlane;
    float tmax = u_farPlane;

    float t = tmin;
    for( int i=0; i<256 && t<tmax; i++ )
    {
        vec3 pos = ray_origin + t*ray_direction;
        sdf_result res = map(pos);
        if( abs(res.dist)<(0.0001*t) )
        { 
            result = res;
            res.dist = t;
            break;
        }
        t += res.dist;
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
        n += e*map(pos+0.0005*e).dist;
      //if( n.x+n.y+n.z>100.0 ) break;
    }
    return normalize(n); 
}

vec3 render( vec3 ray_origin, vec3 ray_direction )
{ 
    vec3 col = u_Ambient;

    sdf_result res = raycast(ray_origin, ray_direction);
    if( res.mat.ka>-0.5 )
    {
        vec3 pos = ray_origin + res.dist*ray_direction;
        vec3 nor = calcNormal( pos );
        material mat = res.mat; 

        vec3 totalAmbient = u_Ambient + u_dirLight.ambient_impact * u_dirLight.color;
        vec3 light = mat.ka * totalAmbient
            + calc_dir_light(u_dirLight, mat, nor, -ray_direction)
            + calc_point_light(u_pointLight, mat, nor, -ray_direction, pos);
		col = light * mat.albedo;
    }

	return col;
}

void main() {
    vec2 pos = (2.0*gl_FragCoord.xy-u_resolution)/u_resolution.y;

    vec4 ray_origin;    // vec4(ro, 1) as it needs to be translated
    vec4 ray_direction; // vec4(rd, 0) as it cannot be translated
    if (u_ortho) {
        ray_origin = vec4(vec3(u_camSize * pos, 0), 1); // apply desired scaling from fov
        ray_direction = vec4(0, 0, -1, 0);
    } else {
        ray_origin = vec4(0, 0, 0, 1); 
        ray_direction = vec4(normalize(vec3(pos, -u_nearPlane)), 0);
    }
    
    vec3 color = render((u_iV * ray_origin).xyz, ((u_iV * ray_direction).xyz));
    fragColor = vec4(color, 1.0 );
}
