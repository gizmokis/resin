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