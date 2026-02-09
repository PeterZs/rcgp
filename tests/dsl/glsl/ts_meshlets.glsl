#version 460

#extension GL_EXT_mesh_shader : require

struct meshletsxMeshletData {
    uint vertex_offset;
    uint vertex_count;
    uint primitive_offset;
    uint primitive_count;
    vec4 bounds;
};

struct meshletsxTaskPayloadData {
    uint meshlet;
};

struct meshletsxViewData {
    mat4 view_proj;
    vec4 frustum_planes[6];
    uint meshlet_count;
    uint task_group_width;
};

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

taskPayloadSharedEXT meshletsxTaskPayloadData task_payload;

layout (std430, push_constant) uniform PC {
    layout (offset = 0) meshletsxViewData pc;
};

layout (std430, set = 0, binding = 0) readonly buffer Buffer0x0 {
    meshletsxMeshletData value[];
} r0b0;

void main()
{
    uint lvar0;
    uint lvar1;
    mat4 lvar2;
    uint lvar3;
    uvec3 lvar4;
    lvar4 = gl_WorkGroupID;
    uint lvar5;
    lvar5 = lvar4.x;
    uint lvar6;
    lvar6 = lvar4.y;
    uint lvar7;
    lvar7 = (lvar6 * pc.task_group_width);
    uint lvar8;
    lvar8 = (lvar7 + lvar5);
    bool lvar9;
    lvar9 = (lvar8 < pc.meshlet_count);
    if (lvar9) {
        uint lvar10;
        uint lvar11;
        uint lvar12;
        uint lvar13;
        vec4 lvar14;
        vec3 lvar15;
        lvar15 = vec3(r0b0.value[lvar8].bounds);
        float lvar16;
        lvar16 = r0b0.value[lvar8].bounds.w;
        bool lvar17;
        lvar17 = true;
        uint lvar18;
        lvar18 = 0;
        while (true) {
            uint lvar19;
            lvar19 = 6;
            bool lvar20;
            lvar20 = (lvar18 < lvar19);
            bool lvar21;
            lvar21 = (!lvar20);
            if (lvar21) {
                break;
            }
            vec4 lvar22;
            vec3 lvar23;
            lvar23 = vec3(pc.frustum_planes[lvar18]);
            float lvar24;
            lvar24 = pc.frustum_planes[lvar18].w;
            dot(lvar23, lvar15);
            float lvar25;
            lvar25 = dot(lvar23, lvar15);
            float lvar26;
            lvar26 = (lvar25 + lvar24);
            float lvar27;
            lvar27 = -1;
            float lvar28;
            lvar28 = (lvar27 * lvar16);
            bool lvar29;
            lvar29 = (lvar26 < lvar28);
            if (lvar29) {
                bool lvar30;
                lvar30 = false;
                lvar17 = lvar30;
            }
            uint lvar31;
            lvar31 = 1;
            uint lvar32;
            lvar32 = (lvar18 + lvar31);
            lvar18 = lvar32;
        }
        if (lvar17) {
            task_payload.meshlet = lvar8;
            uint lvar33;
            lvar33 = 1;
            uint lvar34;
            lvar34 = 1;
            uint lvar35;
            lvar35 = 1;
            EmitMeshTasksEXT(lvar35, lvar34, lvar33);
        }
    }
}
