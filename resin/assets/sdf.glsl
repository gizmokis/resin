struct sdf_node {
    mat4 transform;
    vec3 size;
    float scale;
};

struct sdf_result {
    material mat;
    float dist;
    int id;
};

const int kMaxNodeCount = MAX_UBO_NODE_COUNT;
layout (std140) uniform Data 
{
    sdf_node u_sdf_primitives[kMaxNodeCount];
};

void prepare(inout sdf_result res, inout vec3 pos, int node_id, int material_id) {
    pos = (u_sdf_primitives[node_id].transform * vec4(pos,1)).xyz;
    res.mat = u_sdf_materials[material_id];
    res.id = node_id;
}

sdf_result sdSphere(vec3 pos, int node_id)
{
    sdf_result res;

    prepare(res, pos, node_id, 0);

    sdf_node prop = u_sdf_primitives[node_id];
    res.dist = prop.scale == 0 ? u_farPlane : prop.scale * (length(pos) - prop.size.x);
    return res;
}

sdf_result sdCube(vec3 pos, int node_id)
{
    sdf_result res;

    prepare(res, pos, node_id, 1);

    sdf_node prop = u_sdf_primitives[node_id];
    vec3 d = abs(pos) - prop.size;
    res.dist = prop.scale == 0 ? u_farPlane : prop.scale * (min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0)));
    return res;
}

sdf_result opUnion(sdf_result d1, sdf_result d2, float k) // FIXME(SDF-117)
{
	return (d1.dist<d2.dist) ? d1 : d2;
}

sdf_result opSmoothUnion(sdf_result d1, sdf_result d2, float k)
{
    k *= 6.0/4.0;
    float h = max(k - abs(d1.dist-d2.dist), 0.0)/k;
    sdf_result result;
    result.mat = material_mix(d1.mat, d2.mat, clamp( 0.5+0.5*(d1.dist-d2.dist)/(4*k/6), 0.0, 1.0 ));
    result.dist = min(d1.dist,d2.dist) - h*h*h*k*(1.0/6.0); 
    result.id = d1.dist < d2.dist ? d1.id : d2.id;
    return result;

    // float h = clamp( 0.5 + 0.5*(d2.dist-d1.dist)/k, 0.0, 1.0 );
    // sdf_result result;
    // result.mat = material_mix(d2.mat, d1.mat, h);
    // result.dist = mix( d2.dist, d1.dist, h ) - k*h*(1.0-h);
    // result.id = (d1.dist<d2.dist) ? d1.id : d2.id;
    // return result;
}