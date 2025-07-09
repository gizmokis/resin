#external_definition SDFS_IMPLEMENTATION
#external_definition MAX_UBO_NODE_COUNT
#external_definition MAX_UBO_MATERIAL_COUNT

struct sdf_node {   
    mat4 transform;
    vec3 size;
    int mat_id;
};

struct node_attributes {   
    float scale;
    float factor;
};

struct sdf_result {
    material mat;
    float dist;
    int id;
};

const int kMaxNodeCount = MAX_UBO_NODE_COUNT;
layout (std140, binding = 0) uniform PrimitiveNodeData 
{
    sdf_node u_sdf_primitives[kMaxNodeCount];
};

layout (std140, binding = 1) uniform NodeAttributesData 
{
    node_attributes u_node_attributes[kMaxNodeCount];
};

const int kMaxMaterialCount = MAX_UBO_MATERIAL_COUNT;
layout (std140, binding = 2) uniform MaterialData 
{
    material u_sdf_materials[kMaxMaterialCount];
};

sdf_result opScale(sdf_result res, int node_id) {
    float scale = u_node_attributes[node_id].scale;
    res.dist = scale == 0 ? u_farPlane : scale * res.dist;
    return res;
}

void prepare(inout sdf_result res, inout vec3 pos, int node_id, int primitive_id) {
    pos = (u_sdf_primitives[primitive_id].transform * vec4(pos,1)).xyz;
    res.mat = u_sdf_materials[u_sdf_primitives[primitive_id].mat_id];
    res.id = node_id;
}

sdf_result sdEmpty()
{
    sdf_result res;
    res.dist = u_farPlane;
    return res;
}

sdf_result opUnion(sdf_result d1, sdf_result d2) // FIXME(SDF-117)
{
	return (d1.dist<d2.dist) ? d1 : d2;
}

sdf_result opDiff(sdf_result d1, sdf_result d2) // FIXME(SDF-117)
{
    d2.dist -= 0.001; // prevent rendering ultra-thin regions
	return (d1.dist>-d2.dist) ? d1 : sdf_result(d2.mat, -d2.dist, d2.id);
} 

sdf_result opInter(sdf_result d1, sdf_result d2) // FIXME(SDF-117)
{
	return (d1.dist>d2.dist) ? d1 : d2;
}

sdf_result opXor(sdf_result d1, sdf_result d2) // FIXME(SDF-117)
{
    sdf_result mn = (d1.dist < d2.dist) ? d1 : d2;
    sdf_result mx = (d1.dist < d2.dist) ? d2 : d1;
    mx.dist = -mx.dist + 0.001; // prevent rendering ultra-thin regions
	return (mn.dist>mx.dist) ? mn : mx; 
}

float smooth_min(float a, float b, float k) 
{
    float h = max(k - abs(a-b), 0.0)/k;
    return min(a, b) - 0.1666*h*h*h*k;
}

float smooth_max(float a, float b, float k) 
{
    float h = max(k - abs(a-b), 0.0)/k;
    return max(a, b) + 0.1666*h*h*h*k;
}

sdf_result opSmoothUnion(sdf_result d1, sdf_result d2, int node_id)
{
    float k = max(0.01, u_node_attributes[node_id].factor);
    sdf_result result;
    result.mat = material_mix(d1.mat, d2.mat, clamp(0.5+0.75*(d1.dist-d2.dist)/k, 0.0, 1.0)); // TODO(SDF-157): optimize math?
    result.dist = smooth_min(d1.dist,d2.dist,k); 
    result.id = d1.dist < d2.dist ? d1.id : d2.id;
    return result;
}

sdf_result opSmoothDiff(sdf_result d1, sdf_result d2, int node_id)
{
    float k = max(0.01, u_node_attributes[node_id].factor);
    sdf_result result;
    result.mat = material_mix(d1.mat, d2.mat, clamp(0.5-0.75*(d1.dist+d2.dist)/k, 0.0, 1.0)); // TODO(SDF-157): optimize math?
    result.dist = smooth_max(d1.dist,-d2.dist,k); 
    result.id = d1.dist > -d2.dist ? d1.id : d2.id;
    return result;
}

sdf_result opSmoothInter(sdf_result d1, sdf_result d2, int node_id)
{
    float k = max(0.01, u_node_attributes[node_id].factor);
    sdf_result result;
    result.mat = material_mix(d1.mat, d2.mat, clamp(0.5-0.75*(d1.dist-d2.dist)/k, 0.0, 1.0)); // TODO(SDF-157): optimize math?
    result.dist = smooth_max(d1.dist,d2.dist,k); 
    result.id = d1.dist > d2.dist ? d1.id : d2.id;
    return result;
}

sdf_result opSmoothXor(sdf_result d1, sdf_result d2, int node_id)
{
    return opDiff(opSmoothUnion(d1, d2, node_id), opSmoothInter(d1, d2, node_id)); // TODO(SDF-157): optimize math?
}

SDFS_IMPLEMENTATION
