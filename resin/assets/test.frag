#version 330 core

layout(location = 0) out vec4 color;
				
in vec3 v_Pos;
uniform mat4 u_iV;
uniform bool u_ortho;

uniform vec2 u_resolution;
uniform float u_nearPlane;
uniform float u_farPlane;

uniform mat4 u_iM;
uniform float u_scale;
uniform float u_camSize;

const float infinity = 1. / 0.;

float sdSphere( vec3 pos, float radius )
{
    return length(pos) - radius;
}

float sdCube( vec3 pos, float size )
{
    vec3 d = abs(pos) - size;
    return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

vec2 opUnion( vec2 d1, vec2 d2 )
{
	return (d1.x<d2.x) ? d1 : d2;
}

vec2 opSmoothUnion( vec2 d1, vec2 d2, float k )
{
    float h = clamp( 0.5 + 0.5*(d2.x-d1.x)/k, 0.0, 1.0 );
    return mix( d2, d1, h )- vec2(k*h*(1.0-h), 0);
}

vec2 map( vec3 pos )
{
    return opSmoothUnion(vec2(u_scale == 0 ? u_farPlane : u_scale * sdCube((u_iM*vec4(pos,1)).xyz, 0.5), 0), vec2(sdSphere(pos, 1), 1), 0.5);
}

vec2 raycast( vec3 ray_origin, vec3 ray_direction )
{
    vec2 result = vec2(-1.0,-1.0);

    float tmin = u_nearPlane;
    float tmax = u_farPlane;

    float t = tmin;
    for( int i=0; i<70 && t<tmax; i++ )
    {
        vec3 pos = (u_iV * vec4(ray_origin + t*ray_direction, 1)).xyz;
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
    vec3 col = vec3(0.2, 0.1, 0.0);

    vec2 res = raycast(ray_origin, ray_direction);
    float t = res.x;
	float m = res.y;
    if( m>-0.5 )
    {
        vec3 pos = (u_iV * vec4(ray_origin + t*ray_direction, 1)).xyz;
        vec3 nor = calcNormal( pos );

		col = mix(vec3(1,0,0), vec3(0,0,1), m) * (nor+1.0)/2.0;
    }

	return col;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 pos = (2.0*fragCoord-u_resolution)/u_resolution.y;
    if(u_ortho) {
        pos = u_camSize * pos;
    }
    vec3 ray_direction;
    vec3 ray_origin;

    if (u_ortho) {
        ray_origin = vec3(pos, 0);
        ray_direction = vec3(0,0,-1);
    } else {
        ray_origin = vec3(0,0,0); 
        ray_direction = normalize( vec3(pos, -u_nearPlane) );
    }
   
    fragColor = vec4( render( ray_origin, ray_direction), 1.0 );
}

void main() {
    mainImage(color, gl_FragCoord.xy);
}