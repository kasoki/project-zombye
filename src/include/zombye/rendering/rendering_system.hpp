#ifndef __ZOMBYE_RENDERING_SYSTEM_HPP__
#define __ZOMBYE_RENDERING_SYSTEM_HPP__

#include <memory>
#include <vector>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <zombye/rendering/buffer.hpp>
#include <zombye/rendering/mesh_manager.hpp>
#include <zombye/rendering/program.hpp>
#include <zombye/rendering/shader.hpp>
#include <zombye/rendering/shader_manager.hpp>
#include <zombye/rendering/staticmesh_component.hpp>
#include <zombye/rendering/texture.hpp>
#include <zombye/rendering/texture_manager.hpp>
#include <zombye/rendering/vertex_array.hpp>
#include <zombye/rendering/vertex_layout.hpp>

namespace zombye {
    class game;
}

namespace zombye {
    class rendering_system {
        friend class staticmesh_component;

        game& game_;
        SDL_Window* window_;
        SDL_GLContext context_;

        std::vector<staticmesh_component*> staticmesh_components_;

        shader_ptr vertex_shader_;
        shader_ptr fragment_shader_;
        std::unique_ptr<program> program_;
        vertex_layout staticmesh_layout_;
        zombye::mesh_manager mesh_manager_;
        zombye::texture_manager texture_manager_;
        zombye::shader_manager shader_manager_;

        glm::mat4 projection_;
        glm::mat4 view_;

    public:
        rendering_system(game& game, SDL_Window* window);
        rendering_system(const rendering_system& other) = delete;
        rendering_system(rendering_system&& other) = delete;
        ~rendering_system();
        rendering_system& operator=(const rendering_system& other) = delete;
        rendering_system& operator=(rendering_system&& other) = delete;

        void begin_scene();
        void end_scene();
        void update(float delta_time);
        void clear_color(float red, float green, float blue, float alpha);

        auto& mesh_manager() noexcept {
            return mesh_manager_;
        }

        auto& shader_manager() noexcept {
            return shader_manager_;
        }

        auto& staticmesh_layout() noexcept {
            return staticmesh_layout_;
        }

        auto& texture_manager() noexcept {
            return texture_manager_;
        }

    private:
        void register_component(staticmesh_component* component);
        void unregister_component(staticmesh_component* component);
    };
}

#endif