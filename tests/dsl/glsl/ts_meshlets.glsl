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

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

taskPayloadSharedEXT meshlets_TaskPayloadData task_payload;

layout (std430, push_constant) uniform PC {
    layout (offset = 0) meshlets_ViewData pc;
};

layout (std430, set = 0, binding = 0) readonly buffer Buffer0x0 {
    meshlets_MeshletData value[];
} r0b0;

void main()
{
    uint lvar0;
    lvar0 = pc.meshlet_count;
    uint lvar1;
    lvar1 = pc.task_group_width;
    uvec3 lvar2;
    lvar2 = gl_WorkGroupID;
    uint lvar3;
    lvar3 = lvar2.x;
    uint lvar4;
    lvar4 = lvar2.y;
    uint lvar5;
    lvar5 = (lvar4 * lvar1);
    uint lvar6;
    lvar6 = (lvar5 + lvar3);
    uint lvar7;
    lvar7 = lvar0;
    uint lvar8;
    lvar8 = lvar6;
    bool lvar9;
    lvar9 = (lvar8 < lvar7);
    bool lvar10;
    lvar10 = lvar9;
    bool lvar11;
    lvar11 = lvar10;
    if (lvar11) {
        uint lvar12;
        lvar12 = lvar6;
        vec4 lvar13;
        lvar13 = vec4(r0b0.value[lvar12].bounds);
        vec3 lvar14;
        lvar14 = vec3(lvar13);
        float lvar15;
        lvar15 = r0b0.value[lvar12].bounds.w;
        bool lvar16;
        lvar16 = true;
        uint lvar17;
        lvar17 = 0;
        while (true) {
            uint lvar18;
            lvar18 = 6;
            uint lvar19;
            lvar19 = lvar17;
            bool lvar20;
            lvar20 = (lvar19 < lvar18);
            bool lvar21;
            lvar21 = (!lvar20);
            bool lvar22;
            lvar22 = lvar21;
            bool lvar23;
            lvar23 = lvar22;
            if (lvar23) {
                break;
            }
            uint lvar24;
            lvar24 = lvar17;
            vec4 lvar25;
            lvar25 = vec4(pc.frustum_planes[lvar24]);
            vec3 lvar26;
            lvar26 = vec3(lvar25);
            float lvar27;
            lvar27 = pc.frustum_planes[lvar24].w;
            float lvar28;
            lvar28 = dot(lvar26, lvar14);
            float lvar29;
            lvar29 = (lvar28 + lvar27);
            float lvar30;
            lvar30 = -1;
            float lvar31;
            lvar31 = (lvar30 * lvar15);
            float lvar32;
            lvar32 = lvar31;
            float lvar33;
            lvar33 = lvar29;
            bool lvar34;
            lvar34 = (lvar33 < lvar32);
            bool lvar35;
            lvar35 = lvar34;
            bool lvar36;
            lvar36 = lvar35;
            if (lvar36) {
                bool lvar37;
                lvar37 = false;
                lvar16 = lvar37;
            }
            uint lvar38;
            lvar38 = lvar17;
            uint lvar39;
            lvar39 = 1;
            uint lvar40;
            lvar40 = (lvar17 + lvar39);
            lvar17 = lvar40;
        }
        bool lvar41;
        lvar41 = lvar16;
        bool lvar42;
        lvar42 = lvar41;
        bool lvar43;
        lvar43 = lvar42;
        if (lvar43) {
            task_payload.meshlet = lvar6;
            uint lvar44;
            lvar44 = 1;
            uint lvar45;
            lvar45 = 1;
            uint lvar46;
            lvar46 = 1;
            EmitMeshTasksEXT(lvar46, lvar45, lvar44);
        }
    }
}
