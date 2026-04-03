#version 460

#extension GL_EXT_mesh_shader : require
#extension GL_EXT_scalar_block_layout : require

struct meshlets_MeshletData {
    uint vertex_offset;
    uint vertex_count;
    uint primitive_offset;
    uint primitive_count;
    vec4 bounds;
};

struct meshlets_TaskPayloadData {
    uint meshlet;
};

struct meshlets_ViewData {
    mat4 view_proj;
    vec4 frustum_planes[6];
    uint meshlet_count;
    uint task_group_width;
};

layout (location = 0) smooth out vec3 lout0[64];

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (max_vertices = 64, max_primitives = 126) out;
layout (triangles) out;

taskPayloadSharedEXT meshlets_TaskPayloadData task_payload;

layout (std430, push_constant) uniform PC {
    layout (offset = 0) meshlets_ViewData pc;
};

layout (std430, set = 0, binding = 0) readonly buffer Buffer0x0 {
    vec4 value[];
} r0b0;

layout (std430, set = 0, binding = 0) readonly buffer Buffer0x0 {
    meshlets_MeshletData value[];
} r0b0;

layout (std430, set = 0, binding = 0) readonly buffer Buffer0x0 {
    uint value[];
} r0b0;

layout (scalar, set = 0, binding = 0) readonly buffer Buffer0x0 {
    uvec3 value[];
} r0b0;

layout (scalar, set = 0, binding = 0) readonly buffer Buffer0x0 {
    vec3 value[];
} r0b0;

void main()
{
    uint lvar0;
    lvar0 = task_payload.meshlet;
    uint lvar1;
    lvar1 = pc.meshlet_count;
    uint lvar2;
    lvar2 = pc.task_group_width;
    uint lvar3;
    lvar3 = lvar0;
    uint lvar4;
    lvar4 = lvar3;
    uint lvar5;
    lvar5 = r0b0.value[lvar4].primitive_count;
    uint lvar6;
    lvar6 = r0b0.value[lvar4].vertex_count;
    SetMeshOutputsEXT(lvar6, lvar5);
    uint lvar7;
    lvar7 = 0;
    while (true) {
        uint lvar8;
        lvar8 = r0b0.value[lvar4].vertex_count;
        uint lvar9;
        lvar9 = lvar7;
        bool lvar10;
        lvar10 = (lvar9 < lvar8);
        bool lvar11;
        lvar11 = (!lvar10);
        bool lvar12;
        lvar12 = lvar11;
        bool lvar13;
        lvar13 = lvar12;
        if (lvar13) {
            break;
        }
        uint lvar14;
        lvar14 = (r0b0.value[lvar4].vertex_offset + lvar7);
        uint lvar15;
        lvar15 = lvar14;
        uint lvar16;
        lvar16 = r0b0.value[lvar15];
        vec4 lvar17;
        lvar17 = (pc.view_proj * r0b0.value[lvar16]);
        uint lvar18;
        lvar18 = lvar7;
        gl_MeshVerticesEXT[lvar18].gl_Position = lvar17;
        uint lvar19;
        lvar19 = lvar3;
        uint lvar20;
        lvar20 = lvar7;
        lout0[lvar20] = r0b0.value[lvar19];
        uint lvar21;
        lvar21 = lvar7;
        uint lvar22;
        lvar22 = 1;
        uint lvar23;
        lvar23 = (lvar7 + lvar22);
        lvar7 = lvar23;
    }
    uint lvar24;
    lvar24 = 0;
    while (true) {
        uint lvar25;
        lvar25 = r0b0.value[lvar4].primitive_count;
        uint lvar26;
        lvar26 = lvar24;
        bool lvar27;
        lvar27 = (lvar26 < lvar25);
        bool lvar28;
        lvar28 = (!lvar27);
        bool lvar29;
        lvar29 = lvar28;
        bool lvar30;
        lvar30 = lvar29;
        if (lvar30) {
            break;
        }
        uint lvar31;
        lvar31 = (r0b0.value[lvar4].primitive_offset + lvar24);
        uint lvar32;
        lvar32 = lvar31;
        uint lvar33;
        lvar33 = lvar24;
        gl_PrimitiveTriangleIndicesEXT[lvar33] = r0b0.value[lvar32];
        uint lvar34;
        lvar34 = lvar24;
        uint lvar35;
        lvar35 = 1;
        uint lvar36;
        lvar36 = (lvar24 + lvar35);
        lvar24 = lvar36;
    }
}
