#include "entity.as"

class player : entity {
    player(const glm::vec3& in position, const glm::quat& in rotation, const glm::vec3& in scale,
        const string& in mesh, const string& in skeleton) {
        super(position, rotation, scale);
        impl_.add_animation_component(mesh, skeleton);
        impl_.add_character_physics_component(box_shape(glm::vec3(0.6, 1, 0.3)), 10.f, 2.f);
        auto@ sc = impl_.add_state_component();
        sc.emplace("stand", "scripts/character_states/stand.as");
        sc.emplace("run", "scripts/character_states/run.as");
        sc.emplace("walk_backward", "scripts/character_states/walk_backward.as");

        sc.change_state("stand");
    }
}
