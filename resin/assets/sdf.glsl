struct node {
    mat4 transform;
    vec3 size;
    float scale;
};

vec2 sdSphere(vec3 pos, node prop)
{
    pos = (prop.transform * vec4(pos,1)).xyz;
    return vec2(prop.scale == 0 ? u_farPlane : prop.scale * (length(pos) - prop.size.x),1);
}

vec2 sdCube(vec3 pos, node prop)
{
    pos = (prop.transform * vec4(pos,1)).xyz;
    vec3 d = abs(pos) - prop.size;
    return vec2(prop.scale == 0 ? u_farPlane : prop.scale * (min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0))),0);
}

vec2 opUnion(vec2 d1, vec2 d2, float k) // FIXME(SDF-117)
{
	return (d1.x<d2.x) ? d1 : d2;
}

vec2 opSmoothUnion(vec2 d1, vec2 d2, float k)
{
    float h = clamp( 0.5 + 0.5*(d2.x-d1.x)/k, 0.0, 1.0 );
    return mix( d2, d1, h )- vec2(k*h*(1.0-h), 0);
}
