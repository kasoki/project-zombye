#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <zombye/core/game.hpp>
#include <zombye/rendering/mesh.hpp>
#include <zombye/rendering/rendering_system.hpp>
#include <zombye/utils/component_helper.hpp>
#include <zombye/utils/logger.hpp>

namespace zombye {
    rendering_system::rendering_system(game& game, SDL_Window* window)
    : game_{game}, window_{window}, mesh_manager_{game_}, shader_manager_{game_}, texture_manager_{game_},
    active_camera_{0} {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        context_ = SDL_GL_CreateContext(window_);
        auto error = std::string{SDL_GetError()};
        if (error != "") {
            log(LOG_FATAL, "could not create OpenGL context with version 3.1: " + error);
        }
        SDL_ClearError();

        SDL_GL_SetSwapInterval(1);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            log(LOG_FATAL, "could not initialize glew");
        }

        auto version = std::string{reinterpret_cast<const char*>(glGetString(GL_VERSION))};
        log("OpenGL version " + version);

        glEnable(GL_DEPTH_TEST);
        clear_color(0.4, 0.5, 0.9, 1.0);

        vertex_shader_ = shader_manager_.load("shader/staticmesh.vs", GL_VERTEX_SHADER);
        fragment_shader_ = shader_manager_.load("shader/staticmesh.fs", GL_FRAGMENT_SHADER);

        program_ = std::make_unique<program>();
        program_->attach_shader(vertex_shader_);
        program_->attach_shader(fragment_shader_);

        staticmesh_layout_.emplace_back("position", 3, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
        staticmesh_layout_.emplace_back("nomal", 3, GL_FLOAT, GL_FALSE, sizeof(vertex), sizeof(glm::vec3));
        staticmesh_layout_.emplace_back("texcoord", 2, GL_FLOAT, GL_FALSE, sizeof(vertex), 2 * sizeof(glm::vec3));

        staticmesh_layout_.setup_program(*program_, "fragcolor");
        program_->link();

        float fovy = 90.f * 3.1415f / 180.f;
        float aspect = static_cast<float>(game_.width()) / static_cast<float>(game_.height());
        float near = 0.01f;
        float far = 1000.f;
        projection_ = glm::perspective(fovy, aspect, near, far);
    }

    rendering_system::~rendering_system() {
        window_ = nullptr;
        SDL_GL_DeleteContext(context_);
    }

    void rendering_system::begin_scene() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void rendering_system::end_scene() {
        SDL_GL_SwapWindow(window_);
    }

    void rendering_system::update(float delta_time) {
        auto camera = camera_components_.find(active_camera_);
        auto view = glm::mat4{1.f};
        auto camera_position = glm::vec3{0.f};
        if (camera != camera_components_.end()) {
            view = camera->second->transform();
            camera_position = camera->second->owner().position();
        }

        std::vector<glm::vec3> light_positions;
        std::vector<glm::vec3> light_colors;
        auto i = 0;
        for (auto& l : light_components_) {
            if (i > 15) {
                break;
            }
            light_positions.emplace_back(l->owner().position());
            light_colors.emplace_back(l->color());
        }
        int light_count = light_positions.size();

        program_->use();
        program_->uniform("diffuse_sampler", 0);
        program_->uniform("specular_sampler", 1);
        program_->uniform("light_count", light_count);
        program_->uniform("light_position", light_count, light_positions);
        program_->uniform("light_color", light_count, light_colors);
        program_->uniform("view", camera_position);
        for (auto& s : staticmesh_components_) {
            auto model = s->owner().transform();
            auto model_it = glm::inverse(glm::transpose(model));
            program_->uniform("m", false, model);
            program_->uniform("mit", false, model_it);
            program_->uniform("mvp", false, projection_ * view * model);
            s->draw();
        }
    }

    void rendering_system::clear_color(float red, float green, float blue, float alpha) {
        glClearColor(red, green, blue, alpha);
    }

    void rendering_system::register_component(camera_component* component) {
        camera_components_.insert(std::make_pair(component->owner().id(), component));
    }

    void rendering_system::unregister_component(camera_component* component) {
        auto it = camera_components_.find(component->owner().id());
        if (it != camera_components_.end()) {
            camera_components_.erase(it);
        }
    }

    void rendering_system::register_component(light_component* component) {
        light_components_.emplace_back(component);
    }

    void rendering_system::unregister_component(light_component* component) {
        remove(light_components_, component);
    }

    void rendering_system::register_component(staticmesh_component* component) {
        staticmesh_components_.emplace_back(component);
    }

    void rendering_system::unregister_component(staticmesh_component* component) {
        remove(staticmesh_components_, component);
    }
}