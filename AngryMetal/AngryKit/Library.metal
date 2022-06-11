//
//  Library.metal
//  AngryKit
//
//  Created by  Ivan Ushakov on 22.05.2021.
//

#include <metal_stdlib>

#include "shader_common.h"

using namespace metal;

struct DepthOnlyInputVertex
{
    float3 position [[attribute(0)]];
};

struct DepthOnlyOutputVertex
{
    float4 position [[position]];
};

vertex DepthOnlyOutputVertex depth_only_vertex_shader(DepthOnlyInputVertex input_vertex [[stage_in]],
                                                      constant DepthOnlyUniforms& uniforms [[buffer(1)]])
{
    DepthOnlyOutputVertex output;
    output.position = uniforms.light_space_matrix * uniforms.model_matrix * float4(input_vertex.position, 1.0f);
    return output;
}

fragment void dummy_fragment_shader()
{
    return;
}

struct BasicInputVertex
{
    float3 position [[attribute(0)]];
    float2 uv [[attribute(1)]];
};

struct BasicOutputVertex
{
    float4 position [[position]];
    float2 uv;
    float4 light_space_position;
    float3 world_position;
};

vertex BasicOutputVertex basic_vertex_shader(BasicInputVertex input_vertex [[stage_in]],
                                             constant BasicUniforms& uniforms [[buffer(2)]])
{
    BasicOutputVertex output;
    float4 world_position = uniforms.model_matrix * float4(input_vertex.position, 1.0f);
    output.position = uniforms.pv * world_position;
    output.uv = input_vertex.uv;
    output.world_position = world_position.xyz;
    output.light_space_position = uniforms.light_space_matrix * world_position;
    return output;
}

constexpr sampler basic_sampler(address::repeat, filter::linear);
constexpr sampler shadow_map_sampler(address::clamp_to_border, filter::nearest);

float shadow_calculation_1(float bias, float4 frag_pos_light_space, float2 offset, depth2d<float, access::sample> shadow_map)
{
    float3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords = proj_coords * 0.5f + 0.5f;
    float closest_depth = shadow_map.sample(shadow_map_sampler, proj_coords.xy + offset);
    float current_depth = proj_coords.z;
    bias = 0.001f;
    float shadow = (current_depth - bias) > closest_depth ? 1.0f : 0.0f;
    return shadow;
}

fragment half4 floor_fragment_shader(BasicOutputVertex input [[stage_in]],
                                     constant CommonFragmentUniforms& uniforms [[buffer(0)]],
                                     texture2d<float> diffuse_texture [[texture(0)]],
                                     texture2d<float> normal_texture [[texture(1)]],
                                     texture2d<float> specular_texture [[texture(2)]],
                                     depth2d<float, access::sample> shadow_map [[texture(3)]])
{
    float4 color = diffuse_texture.sample(basic_sampler, input.uv);

    float4 result_color = color;
    if (uniforms.use_light)
    {
        float3 light_direction = normalize(-uniforms.direction_light.direction);
        float3 normal = normal_texture.sample(basic_sampler, input.uv).xyz;
        normal = normalize(normal * 2.0f - 1.0f);
        float diff = max(dot(normal, light_direction), 0.0f);
        float3 ambient = uniforms.ambient_color * color.xyz;

        float bias = max(0.05f * (1.0f - dot(normal, light_direction)), 0.005f);
        float shadow = 0.0f;
        float2 texel_size = 1.0f / shadow_map.get_width();
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                shadow += shadow_calculation_1(bias, input.light_space_position, float2(x, y) * texel_size, shadow_map);
            }
        }
        shadow /= 9.0f;
        shadow *= 0.7f;

        result_color = 0.7f * (1.0f - shadow) * float4(uniforms.direction_light.color, 1.0f) * result_color * diff + float4(ambient, 1.0f);

        if (uniforms.use_specular)
        {
            float3 normal(0.0f, 1.0f, 0.0f);
            float3 spec_light_dir = normalize(float3(-3.0f, 0.0f, -1.0f));
            float3 reflect_dir = reflect(spec_light_dir, normal);
            float3 view_dir = normalize(uniforms.view_position - input.world_position);
            float shininess = 0.7f;
            float str = 1.0f; // 0.88;
            float spec = pow(max(dot(view_dir, reflect_dir), 0.0f), shininess);
            result_color += str * spec * specular_texture.sample(basic_sampler, input.uv) * float4(uniforms.direction_light.color, 1.0f);
        }

        if (uniforms.use_point_light)
        {
            float3 light_dir = normalize(uniforms.point_light.world_position - input.world_position);
            float3 normal(0.0f, 1.0f, 0.0f);
            float diff = max(dot(normal, light_dir), 0.0f);
            float d = length(uniforms.point_light.world_position - input.world_position);
            float linear = 0.5f;
            float k = 0.0f;
            float quadratic = 3.0f;
            float attenuation = 1.0f / (k + linear * d + quadratic * (d * d));
            float3 diffuse = uniforms.point_light.color * diff * color.xyz;
            diffuse *= attenuation;
            result_color += float4(diffuse.xyz, 1.0f);
        }
    }

    return half4(result_color);
}

struct InputVertex
{
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float2 uv [[attribute(2)]];
};

struct OutputVertex
{
    float4 position [[position]];
    float3 normal;
    float2 uv;
    float4 light_space_position;
    float3 world_position;
};

vertex OutputVertex vertex_shader(InputVertex input_vertex [[stage_in]],
                                  constant Uniforms& uniforms [[buffer(3)]])
{
    OutputVertex output;
    float4 world_position = uniforms.model_matrix * float4(input_vertex.position, 1.0f);
    output.position = uniforms.pv * world_position;
    output.uv = input_vertex.uv;
    output.normal = (uniforms.aim_rotation * float4(input_vertex.normal, 1.0f)).xyz;
    output.world_position = world_position.xyz;
    output.light_space_position = uniforms.light_space_matrix * world_position;
    return output;
}

float shadow_calculation_2(float bias, float4 frag_pos_light_space, depth2d<float, access::sample> shadow_map)
{
    float3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords = proj_coords * 0.5f + 0.5f;
    float closest_depth = shadow_map.sample(shadow_map_sampler, proj_coords.xy);
    float current_depth = proj_coords.z;
    bias = 0.001f;
    float shadow = (current_depth - bias) > closest_depth ? 1.0f : 0.0f;
    return shadow;
}

fragment half4 player_fragment_shader(OutputVertex input [[stage_in]],
                                      constant CommonFragmentUniforms& uniforms [[buffer(0)]],
                                      texture2d<float> diffuse_texture [[texture(0)]],
                                      texture2d<float> specular_texture [[texture(1)]],
                                      depth2d<float, access::sample> shadow_map [[texture(2)]])
{
    float4 color = diffuse_texture.sample(basic_sampler, input.uv);

    float4 result_color = color;
    if (uniforms.use_light)
    {
        float3 normal = normalize(input.normal);
        float shadow = 0.0f;
        {
            // direction light
            float3 light_dir = normalize(-uniforms.direction_light.direction);
            // TODO use normal texture as well
            float diff = max(dot(normal, light_dir), 0.0f);
            float3 amb = uniforms.ambient_color * color.xyz;
            float bias = max(0.05f * (1.0f - dot(normal, light_dir)), 0.005f);
            shadow = shadow_calculation_2(bias, input.light_space_position, shadow_map);
            result_color = (1.0f - shadow) * float4(uniforms.direction_light.color, 1.0f) * result_color * diff + float4(amb, 1.0f);
        }

        if (uniforms.use_point_light)
        {
            float3 light_dir = normalize(uniforms.point_light.world_position - input.world_position);
            float diff = max(dot(normal, light_dir), 0.0f);
            float3 diffuse  = 0.7f * uniforms.point_light.color  * diff * color.xyz;
            result_color += float4(diffuse, 1.0f);
        }

        if (shadow < 0.1f)
        {
            float3 reflect_dir = reflect(-uniforms.direction_light.direction, normal);
            float3 view_dir = normalize(uniforms.view_position - input.world_position);
            float shininess = 24.0f;
            float strength = 1.0f; // 0.88f
            float spec = pow(max(dot(view_dir, reflect_dir), 0.0f), shininess);
            result_color += strength * spec * specular_texture.sample(basic_sampler, input.uv) * float4(uniforms.direction_light.color, 1.0f);
            result_color += spec * 0.1f * float4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    return half4(result_color);
}

constant float wiggle_magnitude = 3.0f;
constant float wiggle_dist_modifier = 0.12f;
constant float wiggle_time_modifier = 9.4f;

vertex OutputVertex enemy_vertex_shader(InputVertex input_vertex [[stage_in]],
                                        constant EnemyUniforms& uniforms [[buffer(3)]],
                                        constant float4x4* model_matrix [[buffer(4)]],
                                        constant float4x4* aim_rotation [[buffer(5)]],
                                        ushort iid [[instance_id]])
{
    OutputVertex output;
    float4 world_position = model_matrix[iid] * float4(input_vertex.position, 1.0f);

    float d = wiggle_dist_modifier * distance(uniforms.nose_position, input_vertex.position);
    float x_offset = sin(wiggle_time_modifier * uniforms.time + d) * wiggle_magnitude;
    float4 p(input_vertex.position.x + x_offset, input_vertex.position.y, input_vertex.position.z, 1.0f);
    output.position = uniforms.pv * model_matrix[iid] * p;

    output.uv = input_vertex.uv;
    output.normal = (aim_rotation[iid] * float4(input_vertex.normal, 1.0f)).xyz;
    output.world_position = world_position.xyz;
    output.light_space_position = uniforms.light_space_matrix * world_position;
    return output;
}

fragment half4 enemy_fragment_shader(OutputVertex input [[stage_in]],
                                     constant CommonFragmentUniforms& uniforms [[buffer(0)]],
                                     texture2d<float> diffuse_texture [[texture(0)]])
{
    float4 color = diffuse_texture.sample(basic_sampler, input.uv);

    float4 result_color = color;
    if (uniforms.use_light)
    {
        float3 normal = normalize(input.normal);
        float shadow = 0.0f;
        {
            // direction light
            float3 light_dir = normalize(-uniforms.direction_light.direction);
            // TODO use normal texture as well
            float diff = max(dot(normal, light_dir), 0.0f);
            float3 amb = uniforms.ambient_color * color.xyz;
            result_color = (1.0f - shadow) * float4(uniforms.direction_light.color, 1.0f) * result_color * diff + float4(amb, 1.0f);
        }
    }

    return half4(result_color);
}

vertex BasicOutputVertex bullet_vertex_shader(BasicInputVertex input_vertex [[stage_in]],
                                              constant BulletUniforms& uniforms [[buffer(2)]],
                                              constant float4x4* model_matrix [[buffer(3)]],
                                              ushort iid [[instance_id]])
{
    BasicOutputVertex output;
    output.position = uniforms.pv * model_matrix[iid] * float4(input_vertex.position, 1.0f);;
    output.uv = input_vertex.uv;
    return output;
}

fragment half4 bullet_fragment_shader(BasicOutputVertex input [[stage_in]],
                                      texture2d<float> diffuse_texture [[texture(0)]])
{
    float4 color = diffuse_texture.sample(basic_sampler, input.uv);
    return half4(color);
}
