//
//  player_animation_system.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 25.05.2021.
//

#include "player_animation_system.hpp"

#include <simd/simd.h>

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "animation_component.hpp"
#include "health_component.hpp"
#include "mesh_component.hpp"
#include "movement_component.hpp"
#include "scene.hpp"

using namespace angry;

aiMatrix4x4 zero_ai_mat()
{
    aiMatrix4x4 m;
    for (int i = 0; i < 4; i++)
    {
        m[i][i] = 0.0f;
    }
    return m;
}

void scaled_add(aiMatrix4x4& m1, const float scale, const aiMatrix4x4& m2)
{
    m1.a1 += scale * m2.a1;
    m1.a2 += scale * m2.a2;
    m1.a3 += scale * m2.a3;
    m1.a4 += scale * m2.a4;
    m1.b1 += scale * m2.b1;
    m1.b2 += scale * m2.b2;
    m1.b3 += scale * m2.b3;
    m1.b4 += scale * m2.b4;
    m1.c1 += scale * m2.c1;
    m1.c2 += scale * m2.c2;
    m1.c3 += scale * m2.c3;
    m1.c4 += scale * m2.c4;
    m1.d1 += scale * m2.d1;
    m1.d2 += scale * m2.d2;
    m1.d3 += scale * m2.d3;
    m1.d4 += scale * m2.d4;
}

using TransformMap = std::unordered_map<std::string, aiMatrix4x4>;

struct Processor
{
    BufferManagerInterface& buffer_manager;
    Scene& scene;
    AnimationComponent& animation_component;

    Processor(BufferManagerInterface& buffer_manager, Scene& scene, AnimationComponent& animation_component)
        : buffer_manager(buffer_manager), scene(scene), animation_component(animation_component)
    {
    }

    void set_mesh_vertex_buffer(MeshComponent& mesh_component, TransformMap& node_transform_map)
    {
        auto source = mesh_component.source_mesh;

        std::vector<aiMatrix4x4> bone_anim_transform(source->mNumVertices, zero_ai_mat());
        for (unsigned bone_index = 0; bone_index < source->mNumBones; bone_index++)
        {
            aiBone* bone = source->mBones[bone_index];
            const std::string bone_name = bone->mName.C_Str();
            const aiMatrix4x4 node_transform = animation_component.global_inv * node_transform_map[bone_name] * bone->mOffsetMatrix;
            for (unsigned weight_index = 0; weight_index < bone->mNumWeights; weight_index++)
            {
                aiVertexWeight w = bone->mWeights[weight_index];
                scaled_add(bone_anim_transform[w.mVertexId], w.mWeight, node_transform);
            }
        }

        aiMatrix4x4 node_anim_transform;
        if (source->mNumBones == 0)
        {
            std::vector<aiNode*> mesh_nodes;
            std::function<void(aiNode*)> r;
            r = [source, this, &mesh_nodes, &r](aiNode* n)
            {
                for (unsigned xx = 0; xx < n->mNumMeshes; xx++)
                {
                    aiMesh* tmp_mesh = get_mesh(n->mMeshes[xx]).source_mesh;
                    if (tmp_mesh == source)
                    {
                        mesh_nodes.push_back(n);
                    }
                }

                for (unsigned xx = 0; xx < n->mNumChildren; xx++)
                {
                    r(n->mChildren[xx]);
                }
            };

            r(animation_component.root_node);
            for (auto* mesh_node : mesh_nodes)
            {
                node_anim_transform *= node_transform_map[std::string(mesh_node->mName.data)];
            }
        }

        const auto position_index = mesh_component.mesh.vertex_buffer[VertexAttribute::position];
        auto position = buffer_manager.get_buffer_view<float>(position_index).data;

        for (unsigned int i = 0; i < source->mNumVertices; i++)
        {
            aiVector3D v = (source->mNumBones > 0 ? bone_anim_transform[i] : node_anim_transform) * source->mVertices[i];
            position[3 * i] = v.x;
            position[3 * i + 1] = v.y;
            position[3 * i + 2] = v.z;
        }
    }

    void process_mesh(MeshComponent& mesh_component, TransformMap& node_transform_map)
    {
        auto source = mesh_component.source_mesh;

        if (mesh_component.mesh.vertex_buffer.empty())
        {
            const auto vertex_count = source->mNumVertices;
            {
                const auto index = buffer_manager.create_buffer(vertex_count * 3 * sizeof(float));
                mesh_component.mesh.vertex_buffer[VertexAttribute::position] = index;
            }

            const auto normal_index = buffer_manager.create_buffer(vertex_count * 3 * sizeof(float));
            mesh_component.mesh.vertex_buffer[VertexAttribute::normal] = normal_index;

            const auto uv_index = buffer_manager.create_buffer(vertex_count * 2 * sizeof(float));
            mesh_component.mesh.vertex_buffer[VertexAttribute::uv] = uv_index;

            auto normal = buffer_manager.get_buffer_view<float>(normal_index).data;
            auto uv = buffer_manager.get_buffer_view<float>(uv_index).data;
            for (unsigned int i = 0; i < source->mNumVertices; i++)
            {
                normal[3 * i] = source->mNormals[i].x;
                normal[3 * i + 1] = source->mNormals[i].y;
                normal[3 * i + 2] = source->mNormals[i].z;

                if (source->mTextureCoords[0])
                {
                    uv[2 * i] = source->mTextureCoords[0][i].x;
                    uv[2 * i + 1] = source->mTextureCoords[0][i].y;
                }
                else
                {
                    uv[2 * i] = 0;
                    uv[2 * i + 1] = 0;
                }
            }

            mesh_component.mesh.vertex_count = vertex_count;
        }

        if (!mesh_component.mesh.index_buffer)
        {
            const auto index_count = 3 * source->mNumFaces;
            const auto index = buffer_manager.create_buffer(index_count * sizeof(uint32_t));
            mesh_component.mesh.index_buffer = index;
            mesh_component.mesh.index_count = index_count;

            auto view = buffer_manager.get_buffer_view<uint32_t>(index);
            size_t p = 0;
            for (unsigned int i = 0; i < source->mNumFaces; i++)
            {
                aiFace& face = source->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                {
                    const unsigned int vertex_index = face.mIndices[j];
                    view.data[p++] = vertex_index;
                }
            }
        }

        set_mesh_vertex_buffer(mesh_component, node_transform_map);
    }

    MeshComponent& get_mesh(unsigned int mesh_index)
    {
        switch (mesh_index)
        {
            case 0:
            {
                return scene.get_registry().get<MeshComponent>(scene.get_player());
            }

            case 1:
            {
                return scene.get_registry().get<MeshComponent>(scene.get_gun());
            }

            default:
            {
                throw std::runtime_error("");
            }
        }
    }

    void process_node(aiNode* node, TransformMap& transform_map)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            process_mesh(get_mesh(node->mMeshes[i]), transform_map);
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            aiNode* child_node = node->mChildren[i];
            process_node(child_node, transform_map);
        }
    }
};

void append_node_points(float target_anim_ticks, aiAnimation* anim, aiMatrix4x4 transform, aiNode* node, TransformMap& node_transform_map)
{
    aiMatrix4x4 anim_transform;
    bool anim_found = false;
    for (unsigned channel_index = 0; channel_index < anim->mNumChannels; channel_index++)
    {
        const aiNodeAnim* node_anim = anim->mChannels[channel_index];
        if (node_anim->mNodeName != node->mName)
        {
            continue;
        }

        anim_found = true;

        // rotation animation
        for (unsigned x = 0; x < node_anim->mNumRotationKeys; ++x)
        {
            aiQuatKey k = node_anim->mRotationKeys[x];
            // TODO see b8bf1eac041f0bbb406019a28f310509dad51b86 in https://github.com/assimp/assimp
            const float t = k.mTime / 1000.0f * anim->mTicksPerSecond;
            if (t >= target_anim_ticks)
            {
                anim_transform = aiMatrix4x4(k.mValue.GetMatrix()) * anim_transform;
                break;
            }
        }

        // position animation
        for (unsigned x = 0; x < node_anim->mNumPositionKeys; ++x)
        {
            aiVectorKey k = node_anim->mPositionKeys[x];
            // TODO see b8bf1eac041f0bbb406019a28f310509dad51b86 in https://github.com/assimp/assimp
            const float t = k.mTime / 1000.0f * anim->mTicksPerSecond;
            if (t >= target_anim_ticks)
            {
                if (std::abs(k.mTime - target_anim_ticks) > 2.0f)
                {
                    // TODO why block is empty?
                }

                anim_transform.a4 += k.mValue.x;
                anim_transform.b4 += k.mValue.y;
                anim_transform.c4 += k.mValue.z;
                break;
            }
        }
    }

    transform *= anim_found ? anim_transform : node->mTransformation;
    node_transform_map.emplace(node->mName.C_Str(), transform);

    for (unsigned i = 0; i < node->mNumChildren; ++i)
    {
        append_node_points(target_anim_ticks, anim, transform, node->mChildren[i], node_transform_map);
    }
}

PlayerAnimationSystem::PlayerAnimationSystem(BufferManagerInterface& buffer_manager) : _buffer_manager(buffer_manager)
{
}

void PlayerAnimationSystem::update(Scene& scene, float time)
{
    auto player_entity = scene.get_player();
    auto& animation_component = scene.get_registry().get<AnimationComponent>(player_entity);
    auto& movement_component = scene.get_registry().get<MovementComponent>(player_entity);

    auto& health_component = scene.get_registry().get<HealthComponent>(player_entity);
    if (health_component.health == 0 && animation_component.death_time < 0)
    {
        animation_component.death_time = time;
    }

    aiAnimation* animation = animation_component.animation;
    aiNode* root_node = animation_component.root_node;
    TransformMap merged_node_transform_map;

    struct AnimationData
    {
        float weight;
        float min_ticks;
        float max_ticks;
        float tick_offset;
        float* opt_anim_start = nullptr;
    };
    const auto process_anim = [&merged_node_transform_map, root_node, animation, time](const AnimationData& data)
    {
        if (data.weight == 0.0f)
        {
            return;
        }

        const float tick_range = data.max_ticks - data.min_ticks;
        float target_anim_ticks = data.opt_anim_start ? std::min((float)((time - *data.opt_anim_start) * animation->mTicksPerSecond + data.tick_offset), tick_range)
            : fmod(time * animation->mTicksPerSecond + data.tick_offset, tick_range);
        target_anim_ticks += data.min_ticks;
        if (target_anim_ticks < (data.min_ticks - 0.01f) || target_anim_ticks > (data.max_ticks + 0.01f))
        {
            throw std::runtime_error("PlayerAnimationSystem::animate()");
        }

        TransformMap local_node_transform_map;
        append_node_points(target_anim_ticks, animation, aiMatrix4x4(), root_node, local_node_transform_map);
        for (const auto& t : local_node_transform_map)
        {
            if (merged_node_transform_map.find(t.first) == merged_node_transform_map.end())
            {
                merged_node_transform_map[t.first] = zero_ai_mat();
            }

            // TODO += ?
            merged_node_transform_map[t.first] = merged_node_transform_map[t.first] + t.second * data.weight;
        }
    };

    const bool is_player_moving = simd_length(movement_component.direction) > 0.1f;
    float theta_delta = 0;
    const simd_float2 movement_anim{sin(theta_delta), cos(theta_delta)};

    const float delta_time = time - animation_component.last_anim_time;
    animation_component.last_anim_time = time;

    animation_component.prev_idle_weight = std::max(0.0f, animation_component.prev_idle_weight - delta_time / animation_component.transition_time);
    animation_component.prev_right_weight = std::max(0.0f, animation_component.prev_right_weight - delta_time / animation_component.transition_time);
    animation_component.prev_left_weight = std::max(0.0f, animation_component.prev_left_weight - delta_time / animation_component.transition_time);
    animation_component.prev_forward_weight = std::max(0.0f, animation_component.prev_forward_weight - delta_time / animation_component.transition_time);
    animation_component.prev_back_weight = std::max(0.0f, animation_component.prev_back_weight - delta_time / animation_component.transition_time);

    const bool is_dead = animation_component.death_time >= 0.0f;
    float death_weight = is_dead ? 1.0f : 0.0f;
    float idle_weight = animation_component.prev_idle_weight + ((is_dead || is_player_moving) ? 0.0f : 1.0f);
    float right_weight = animation_component.prev_right_weight + (is_player_moving ? std::max(0.0f, -movement_anim.x) : 0.0f);
    float forward_weight = animation_component.prev_forward_weight + (is_player_moving ? std::max(0.0f, movement_anim.y) : 0.0f);
    float back_weight = animation_component.prev_back_weight + (is_player_moving ? std::max(0.0f, -movement_anim.y) : 0.0f);
    float left_weight = animation_component.prev_left_weight + (is_player_moving ? std::max(0.0f, movement_anim.x) : 0.0f);

    const float weight_sum = death_weight + idle_weight + right_weight + forward_weight + back_weight + left_weight;
    death_weight /= weight_sum;
    idle_weight /= weight_sum;
    right_weight /= weight_sum;
    forward_weight /= weight_sum;
    back_weight /= weight_sum;
    left_weight /= weight_sum;

    animation_component.prev_idle_weight = std::max(animation_component.prev_idle_weight, idle_weight);
    animation_component.prev_right_weight = std::max(animation_component.prev_right_weight, right_weight);
    animation_component.prev_left_weight = std::max(animation_component.prev_left_weight, left_weight);
    animation_component.prev_forward_weight = std::max(animation_component.prev_forward_weight, forward_weight);
    animation_component.prev_back_weight = std::max(animation_component.prev_back_weight, back_weight);

    if (abs(right_weight + forward_weight + back_weight + left_weight + idle_weight + death_weight - 1.0f) > 0.001f)
    {
        throw std::runtime_error("PlayerAnimationSystem::animate()");
    }

    process_anim({death_weight, 234.0f, 293.0f, 0.0f, &animation_component.death_time});
    process_anim({idle_weight, 55.0f, 130.0f, 0.0f});

    const float movement_anim_dur = 20.0f;
    process_anim({forward_weight, 134.0f, 134.0f + movement_anim_dur, 0.0f});
    process_anim({right_weight, 184.0f, 184.0f + movement_anim_dur, 10.0f});
    process_anim({back_weight, 159.0f, 159.0f + movement_anim_dur, 10.0f});
    process_anim({left_weight, 209.0f, 209.0f + movement_anim_dur, 0.0f});

    Processor processor(_buffer_manager, scene, animation_component);
    processor.process_node(root_node, merged_node_transform_map);
}
