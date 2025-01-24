#external_definition MAX_UBO_NODE_COUNT
#external_definition MAX_UBO_MATERIAL_COUNT

struct sdf_node {   
    mat4 transform; 
    vec3 size;      
    float scale;    
    int mat_id;     
};

struct sdf_result {
    material mat;
    float dist;
    int id;
};

const int kMaxNodeCount = MAX_UBO_NODE_COUNT;
layout (std140, binding = 0) uniform NodeData 
{
    sdf_node u_sdf_primitives[kMaxNodeCount];
};

const int kMaxMaterialCount = MAX_UBO_MATERIAL_COUNT;
layout (std140, binding = 1) uniform MaterialData 
{
    material u_sdf_materials[kMaxMaterialCount];
};

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

sdf_result sdSphere(vec3 pos, int node_id, int primitive_id)
{
    sdf_result res;

    prepare(res, pos, node_id, primitive_id);

    sdf_node prop = u_sdf_primitives[primitive_id];
    res.dist = prop.scale == 0 ? u_farPlane : prop.scale * (length(pos) - prop.size.x);
    return res;
}

sdf_result sdCube(vec3 pos, int node_id, int primitive_id)
{
    sdf_result res;

    prepare(res, pos, node_id, primitive_id);

    sdf_node prop = u_sdf_primitives[primitive_id];
    vec3 d = abs(pos) - 0.5*prop.size;
    res.dist = prop.scale == 0 ? u_farPlane : prop.scale * (min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0)));
    return res;
}

sdf_result sdTorus(vec3 pos, int node_id, int primitive_id)
{
    sdf_result res;

    prepare(res, pos, node_id, primitive_id);

    sdf_node prop = u_sdf_primitives[primitive_id];
    vec2 d = vec2(length(pos.xz) - prop.size.x, pos.y);
    res.dist = prop.scale == 0 ? u_farPlane : prop.scale * (length(d) - prop.size.y);
    return res;
}

sdf_result sdCapsule(vec3 pos, int node_id, int primitive_id)
{
    sdf_result res;

    prepare(res, pos, node_id, primitive_id);

    sdf_node prop = u_sdf_primitives[primitive_id];
    vec3 d = vec3(pos.x, pos.y - clamp(pos.y, -0.5*prop.size.x, 0.5*prop.size.x), pos.z);
    res.dist = prop.scale == 0 ? u_farPlane : prop.scale * (length(d) - prop.size.y);
    return res;
}

sdf_result sdLink(vec3 pos, int node_id, int primitive_id)
{
    sdf_result res;

    prepare(res, pos, node_id, primitive_id);

    sdf_node prop = u_sdf_primitives[primitive_id];
    vec3 d = vec3(pos.x, max(abs(pos.y) - 0.5*prop.size.x, 0.0), pos.z);
    res.dist = prop.scale == 0 ? u_farPlane : prop.scale * (length(vec2(length(d.xy)-prop.size.y,d.z)) - prop.size.z);
    return res;
}

sdf_result sdEllipsoid(vec3 pos, int node_id, int primitive_id)
{
    sdf_result res;

    prepare(res, pos, node_id, primitive_id);

    sdf_node prop = u_sdf_primitives[primitive_id];
    vec2 d = vec2(length(pos/prop.size), length(pos/(prop.size*prop.size)));
    res.dist = prop.scale == 0 ? u_farPlane : prop.scale * (d.x*(d.x-1.0)/d.y);
    return res;
}

//https://iquilezles.org/articles/distfunctions/
sdf_result sdPyramid(vec3 pos, int node_id, int primitive_id)
{
    sdf_result res;

    prepare(res, pos, node_id, primitive_id);

    sdf_node prop = u_sdf_primitives[primitive_id];
    float h = prop.size.x;
    float m2 = h*h + 0.25;
    pos.xz = abs(pos.xz);
    pos.xz = (pos.z>pos.x) ? pos.zx : pos.xz;
    pos.xz -= 0.5;
    vec3 q = vec3(pos.z, h*pos.y - 0.5*pos.x, h*pos.x + 0.5*pos.y);
    float s = max(-q.x,0.0);
    float t = clamp((q.y-0.5*pos.z)/(m2+0.25), 0.0, 1.0);
    float a = m2*(q.x+s)*(q.x+s) + q.y*q.y;
    float b = m2*(q.x+0.5*t)*(q.x+0.5*t) + (q.y-m2*t)*(q.y-m2*t);
    float d2 = min(q.y,-q.x*m2-q.y*0.5) > 0.0 ? 0.0 : min(a,b);
    res.dist = prop.scale == 0 ? u_farPlane : prop.scale * (sqrt( (d2+q.z*q.z)/m2 ) * sign(max(q.z,-pos.y)));
    return res;
}

sdf_result sdCylinder(vec3 pos, int node_id, int primitive_id)
{
    sdf_result res;

    prepare(res, pos, node_id, primitive_id);

    sdf_node prop = u_sdf_primitives[primitive_id];
    vec2 d = abs(vec2(pos.y,length(pos.xz))) - vec2(prop.size.x/2, prop.size.y);
    res.dist = prop.scale == 0 ? u_farPlane : prop.scale * (min(max(d.y,d.x),0.0) + length(max(d,0.0)));
    return res;
}

//https://iquilezles.org/articles/distfunctions/
sdf_result sdPrism(vec3 pos, int node_id, int primitive_id)
{
    sdf_result res;

    prepare(res, pos, node_id, primitive_id);

    sdf_node prop = u_sdf_primitives[primitive_id];
    vec3 d = abs(pos);
    res.dist = prop.scale == 0 ? u_farPlane : prop.scale * (max(d.y-prop.size.x*0.5, max(d.x*0.866025+pos.z*0.5,-pos.z)-prop.size.y*0.5));
    return res;
}

sdf_result opUnion(sdf_result d1, sdf_result d2, float k) // FIXME(SDF-117)
{
	return (d1.dist<d2.dist) ? d1 : d2;
}

sdf_result opDiff(sdf_result d1, sdf_result d2, float k) // FIXME(SDF-117)
{
	return (d1.dist>-d2.dist) ? d1 : sdf_result(d2.mat, -d2.dist, d2.id);
} 

sdf_result opInter(sdf_result d1, sdf_result d2, float k) // FIXME(SDF-117)
{
	return (d1.dist>d2.dist) ? d1 : d2;
}

sdf_result opXor(sdf_result d1, sdf_result d2, float k) // FIXME(SDF-117)
{
    sdf_result mn = (d1.dist < d2.dist) ? d1 : d2;
    sdf_result mx = (d1.dist < d2.dist) ? d2 : d1;
    mx.dist = -mx.dist;
	return (mn.dist > mx.dist) ? mn : mx;
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

sdf_result opSmoothUnion(sdf_result d1, sdf_result d2, float k)
{
    sdf_result result;
    result.mat = material_mix(d1.mat, d2.mat, clamp(0.5+0.75*(d1.dist-d2.dist)/k, 0.0, 1.0)); // TODO(SDF-157): optimize math?
    result.dist = smooth_min(d1.dist,d2.dist,k); 
    result.id = d1.dist < d2.dist ? d1.id : d2.id;
    return result;
}

sdf_result opSmoothDiff(sdf_result d1, sdf_result d2, float k)
{
    sdf_result result;
    result.mat = material_mix(d1.mat, d2.mat, clamp(0.5-0.75*(d1.dist+d2.dist)/k, 0.0, 1.0)); // TODO(SDF-157): optimize math?
    result.dist = smooth_max(d1.dist,-d2.dist,k); 
    result.id = d1.dist > -d2.dist ? d1.id : d2.id;
    return result;
}

sdf_result opSmoothInter(sdf_result d1, sdf_result d2, float k)
{
    sdf_result result;
    result.mat = material_mix(d1.mat, d2.mat, clamp(0.5-0.75*(d1.dist-d2.dist)/k, 0.0, 1.0)); // TODO(SDF-157): optimize math?
    result.dist = smooth_max(d1.dist,d2.dist,k); 
    result.id = d1.dist > d2.dist ? d1.id : d2.id;
    return result;
}

sdf_result opSmoothXor(sdf_result d1, sdf_result d2, float k)
{
    return opDiff(opSmoothUnion(d1, d2, k), opSmoothInter(d1, d2, k), k); // TODO(SDF-157): optimize math?
}
