#ifndef __ZOMBYE_RENDERING_SYSTEM_HPP__
#define __ZOMBYE_RENDERING_SYSTEM_HPP__

#include <memory>
#include <unordered_map>
#include <vector>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <zombye/rendering/buffer.hpp>
#include <zombye/rendering/mesh_manager.hpp>
#include <zombye/rendering/shader.hpp>
#include <zombye/rendering/shader_manager.hpp>
#include <zombye/rendering/skeleton_manager.hpp>
#include <zombye/rendering/skinned_mesh_manager.hpp>
#include <zombye/rendering/texture.hpp>
#include <zombye/rendering/texture_manager.hpp>
#include <zombye/rendering/vertex_array.hpp>
#include <zombye/rendering/vertex_layout.hpp>

namespace zombye {
    class game;
    class animation_component;
    class camera_component;
    class light_component;
    class directional_light_component;
    class framebuffer;
    class program;
    class screen_quad;
    class staticmesh_component;
    class shadow_component;
}

namespace zombye {
    struct light_attributes {
        glm::vec3 position;
        glm::vec3 color;
        float radius;
        float exponent;
    };

    class rendering_system {
        friend class animation_component;
        friend class camera_component;
        friend class directional_light_component;
        friend class light_component;
        friend class staticmesh_component;
        friend class shadow_component;

        game& game_;
        SDL_Window* window_;
        SDL_GLContext context_;
        float width_;
        float height_;

        std::vector<animation_component*> animation_components_;
        std::unordered_map<unsigned long, camera_component*> camera_components_;
        std::vector<light_component*> light_components_;
        std::vector<directional_light_component*> directional_light_components_;
        std::vector<staticmesh_component*> staticmesh_components_;
        std::vector<shadow_component*> shadow_components_;

        std::unique_ptr<program> animation_program_;
        std::unique_ptr<program> staticmesh_program_;
        vertex_layout skinnedmesh_layout_;
        vertex_layout staticmesh_layout_;
        zombye::mesh_manager mesh_manager_;
        zombye::texture_manager texture_manager_;
        zombye::shader_manager shader_manager_;
        zombye::skinned_mesh_manager skinned_mesh_manager_;
        zombye::skeleton_manager skeleton_manager_;
        unsigned long active_camera_;

        glm::mat4 ortho_projection_;

        std::unique_ptr<framebuffer> g_buffer_;
        std::unique_ptr<program> screen_quad_program_;
        std::vector<std::unique_ptr<screen_quad>> debug_screen_quads_;
        std::unique_ptr<screen_quad> screen_quad_;
        std::unique_ptr<program> composition_program_;

        int shadow_resolution_;
        std::unique_ptr<framebuffer> shadow_map_;
        glm::mat4 shadow_projection_;
        std::unique_ptr<program> shadow_staticmesh_program_;
        std::unique_ptr<program> shadow_animation_program_;

        std::unique_ptr<framebuffer> shadow_map_blured_;
        std::unique_ptr<program> shadow_blur_program_;

        std::unique_ptr<program> skybox_program_;
        std::shared_ptr<const mesh> skybox_mesh_;

        std::unique_ptr<program> light_cube_program_;

        vertex_layout light_volume_layout_;

        std::unique_ptr<program> point_light_program_;
        std::shared_ptr<const mesh> point_light_volume_;
        std::unique_ptr<vertex_buffer> point_light_instance_data_;

        std::unique_ptr<program> directional_light_program_;

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

        void activate_camera(unsigned long owner_id) {
            if (camera_components_.find(owner_id) != camera_components_.end()) {
                active_camera_ = owner_id;
            }
        }

        unsigned long active_camera_id() {
            return active_camera_;
        }

        camera_component* active_camera() {
            if (camera_components_.find(active_camera_) != camera_components_.end()) {
                return camera_components_[active_camera_];
            }
            return nullptr;
        }

        auto& mesh_manager() noexcept {
            return mesh_manager_;
        }

        auto& shader_manager() noexcept {
            return shader_manager_;
        }

        auto& skinnedmesh_layout() noexcept {
            return skinnedmesh_layout_;
        }

        auto& skinned_mesh_manager() noexcept {
            return skinned_mesh_manager_;
        }

        auto& skeleton_manager() noexcept {
            return skeleton_manager_;
        }

        auto& staticmesh_layout() noexcept {
            return staticmesh_layout_;
        }

        auto& texture_manager() noexcept {
            return texture_manager_;
        }

        auto active_point_lights() const {
            return light_components_.size();
        }

    private:
        void render_debug_screen_quads() const;
        void render_screen_quad();
        void render_shadowmap();
        void apply_gaussian_blur();
        void render_skybox() const;
        void render_lights() const;
        void render_directional_lights(const camera_component& camera) const;
        void render_point_lights(const camera_component& camera) const;
        float calculate_point_light_extend(const light_component& light) const;

        void register_at_script_engine();

        void register_component(animation_component* component);
        void unregister_component(animation_component* component);
        void register_component(camera_component* component);
        void unregister_component(camera_component* component);
        void register_component(directional_light_component* component);
        void unregister_component(directional_light_component* component);
        void register_component(light_component* component);
        void unregister_component(light_component* component);
        void register_component(staticmesh_component* component);
        void unregister_component(staticmesh_component* component);
        void register_component(shadow_component* component);
        void unregister_component(shadow_component* component);
    };
}

#endif
