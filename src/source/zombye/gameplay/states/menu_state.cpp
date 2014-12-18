#include <zombye/gameplay/states/menu_state.hpp>
#include <zombye/rendering/camera_component.hpp>
#include <zombye/rendering/staticmesh_component.hpp>
#include <zombye/physics/physics_component.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <btBulletDynamicsCommon.h>

using namespace std::string_literals;

zombye::menu_state::menu_state(zombye::state_machine *sm) : sm_(sm) {

}

void zombye::menu_state::enter() {
    zombye::log("enter menu state");
    auto& camera = sm_->get_game()->entity_manager().emplace(glm::vec3{1.f, 1.f, 2.f}, glm::normalize(glm::quat{}), glm::vec3{1.f});
    camera.emplace<camera_component>(glm::vec3{0.f}, glm::vec3{0.f, 1.f, 0.f});
    sm_->get_game()->rendering_system().activate_camera(camera.id());
    sm_->get_game()->entity_manager().emplace("suzanne", glm::vec3{0.f}, glm::normalize(glm::quat{}), glm::vec3{1.f});
}

void zombye::menu_state::leave() {
    zombye::log("leave menu state");
}

void zombye::menu_state::update(float delta_time) {

}
