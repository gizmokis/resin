#include "blinn_phong.glsl"

struct sdf_primitive {
    float size;
    material mat;
};

struct sdf_result {
    float dist;
    material mat;
};

sdf_result sdSphere( vec4 pos, sdf_primitive primitive )
{
    sdf_result res;
    res.dist = length(pos.xyz) - primitive.size;
    res.mat = primitive.mat;
    return res;
}

sdf_result sdCube( vec4 pos, sdf_primitive primitive )
{
    vec3 d = abs(pos.xyz) - primitive.size;
    sdf_result res;
    res.dist = min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
    res.mat = primitive.mat;
    return res;
}

sdf_result opUnion( sdf_result res1, sdf_result res2 )
{
	return (res1.dist<res2.dist) ? res1 : res2;
}

sdf_result opDiff( sdf_result res1, sdf_result res2 )
{
	return (-res2.dist<res1.dist) ? res1 : res2;
}

sdf_result opSmoothUnion( sdf_result res1, sdf_result res2 )
{
    const float k = 0.5;
    float h = clamp( 0.5 + 0.5*(res2.dist-res1.dist)/k, 0.0, 1.0 );
    vec2 d = mix( vec2(res2.dist, 1), vec2(res1.dist, 0), h )- vec2(k*h*(1.0-h), 0);
    sdf_result res;
    res.dist = d.x;
    res.mat = material_mix(res1.mat, res2.mat, d.y);
    return res;
}
