#ifndef __ZOMBYE_SKINNED_MESH_HPP__
#define __ZOMBYE_SKINNED_MESH_HPP__

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include <zombye/rendering/buffer.hpp>
#include <zombye/rendering/mesh.hpp>
#include <zombye/rendering/vertex_array.hpp>

namespace zombye {
    class rendering_system;
}

namespace zombye {
    struct skinned_vertex {
        glm::vec3 pos;
        glm::vec2 tex;
        glm::vec3 nor;
        glm::vec3 tan;
        glm::ivec4 index;
        glm::vec4 weight;
    };

    class skinned_mesh {
        std::vector<submesh> submeshes_;
        vertex_array vao_;
        vertex_buffer vbo_;
        index_buffer ibo_;
        bool parallax_mapping_;
    public:
        skinned_mesh(rendering_system& rendering_system, const std::vector<char>& source, const std::string& file_name) noexcept;
        skinned_mesh(const skinned_mesh& other) = delete;
        skinned_mesh(skinned_mesh&& other) noexcept = default;
        ~skinned_mesh() = default;
        skinned_mesh& operator=(const skinned_mesh other) = delete;
        skinned_mesh& operator=(skinned_mesh&& other) noexcept = default;

        void draw() const noexcept;

        auto& vao() const noexcept {
            return vao_;
        }

        auto parallax_mapping() const {
            return parallax_mapping_;
        }
    };
}

#endif
