#version 330 core

layout(location = 0) out vec4 color;
				
in vec3 v_Pos;
uniform float u_time;

const vec2 u_resolution = vec2(1280, 720);

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

vec2 raycast( vec3 ray_origin, vec3 ray_direction )
{
    vec2 result = vec2(-1.0,-1.0);

    float tmin = 1.0;
    float tmax = 20.0;

    float t = tmin;
    for( int i=0; i<70 && t<tmax; i++ )
    {
        vec3 pos = ray_origin + t*ray_direction;
        vec2 dist = opSmoothUnion(vec2(sdCube(pos-vec3(1,1,1), 0.5), 0), vec2(sdSphere(pos, 1), 1), 0.5);
        if( abs(dist.x)<(0.0001*t) )
        { 
            result = vec2(t,dist.y); 
            break;
        }
        t += dist.x;
    }
    
    return result;
}

vec3 render( vec3 ray_origin, vec3 ray_direction )
{ 
    vec3 col = vec3(0.2, 0.1, 0.0);
    
    vec2 res = raycast(ray_origin, ray_direction);
    float t = res.x;
	float m = res.y;
    if( m>-0.5 )
    {
        vec3 pos = ray_origin + t*ray_direction;
        
		col = mix(vec3(1,0,0), vec3(0,0,1), m);
    }

	return col;
}

mat3 setCamera( in vec3 ro, float cr )
{
	vec3 cw = normalize(-ro);
	vec3 cp = vec3(sin(cr), cos(cr),0.0);
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv =          ( cross(cu,cw) );
    return mat3( cu, cv, cw );
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec3 ray_origin = vec3(5*cos(u_time), 2.5, 5*sin(u_time));
    mat3 camera = setCamera( ray_origin, 0.0 );

    vec2 pos = (2.0*fragCoord-u_resolution)/u_resolution.y;
    const float focal_length = 2.5;
    vec3 ray_direction = camera * normalize( vec3(pos, focal_length) );
    fragColor = vec4( render( ray_origin, ray_direction), 1.0 );
}

void main() {
    mainImage(color, gl_FragCoord.xy);
}