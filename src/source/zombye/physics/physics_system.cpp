#include <algorithm>

#include <zombye/config/config_system.hpp>
#include <zombye/core/game.hpp>
#include <zombye/physics/character_physics_component.hpp>
#include <zombye/physics/debug_renderer.hpp>
#include <zombye/physics/debug_render_bridge.hpp>
#include <zombye/physics/physics_component.hpp>
#include <zombye/physics/physics_system.hpp>
#include <zombye/utils/component_helper.hpp>
#include <zombye/ecs/entity.hpp>

#include <zombye/ecs/entity_manager.hpp>

#define ZDBG_DRAW_WIREFRAME 1
#define ZDBG_DRAW_AAB 2
#define ZDBG_DRAW_CONTACT_POINTS 8
#define ZDBG_DRAW_CONSTRAINTS 2048
#define ZDBG_DRAW_CONSTRAINTS_LIMITS 4096
#define ZDBG_DRAW_NORMALS 16384

#define DEBUG_DRAW_CONFIG (ZDBG_DRAW_WIREFRAME | ZDBG_DRAW_AAB | ZDBG_DRAW_CONTACT_POINTS | ZDBG_DRAW_CONSTRAINTS | ZDBG_DRAW_CONSTRAINTS_LIMITS | ZDBG_DRAW_NORMALS)

zombye::physics_system::physics_system(game& game)
: game_{game}, collision_mesh_manager_{game_} {
    broadphase_ = std::make_unique<btDbvtBroadphase>();
    collision_config_ = std::make_unique<btDefaultCollisionConfiguration>();
    dispatcher_ = std::make_unique<btCollisionDispatcher>(collision_config_.get());
    solver_ = std::make_unique<btSequentialImpulseConstraintSolver>();

    world_ = std::make_unique<btDiscreteDynamicsWorld>(
        dispatcher_.get(),
        broadphase_.get(),
        solver_.get(),
        collision_config_.get());

    world_->setGravity(btVector3(0, -9.81, 0));

    debug_renderer_ = std::make_unique<debug_renderer>(game);
    bt_debug_drawer_ = std::make_unique<debug_render_bridge>(*debug_renderer_);

    world_->setDebugDrawer(bt_debug_drawer_.get());

    auto debug_draw = game_.config()->get("main", "physics_debug_draw").asBool();
    if (debug_draw) {
        bt_debug_drawer_->setDebugMode(DEBUG_DRAW_CONFIG);
    }
}

zombye::physics_system::~physics_system() {

}

btDiscreteDynamicsWorld* zombye::physics_system::world() {
    return world_.get();
}

void zombye::physics_system::update(float delta_time) {
    world_->stepSimulation(delta_time);

    check_collisions();

    for(auto comp : components_) {
        comp->sync();
    }

    for (auto& cp : character_physics_components_) {
        cp->update(delta_time);
        cp->sync();
    }
}

void zombye::physics_system::debug_draw() {
    world_->debugDrawWorld();
    debug_renderer_->draw();
}

void zombye::physics_system::toggle_debug() {
    if(bt_debug_drawer_->getDebugMode() != 0) {
        disable_debug();
    } else {
        enable_debug();
    }
}

void zombye::physics_system::enable_debug() {
    bt_debug_drawer_->setDebugMode(DEBUG_DRAW_CONFIG);
}

void zombye::physics_system::disable_debug() {
    bt_debug_drawer_->setDebugMode(0);
}

void zombye::physics_system::register_component(physics_component* comp) {
    components_.push_back(comp);
}

void zombye::physics_system::unregister_component(physics_component* comp) {
    //auto it = std::find(components_.begin(), components_.end(), comp);

    auto it = std::find_if(components_.begin(), components_.end(), [comp](auto other) {
        return comp == other;
    });

    if(it != components_.end()) {
        auto last = components_.end() - 1;

        if(it != last) {
            *it = std::move(*last);
        }

        components_.pop_back();
    }
}

void zombye::physics_system::register_component(character_physics_component* component) {
    character_physics_components_.emplace_back(component);
}

void zombye::physics_system::unregister_component(character_physics_component* component) {
    remove(character_physics_components_, component);
}

void zombye::physics_system::register_collision_callback(entity* a, entity* b, std::function<void(entity*, entity*)> callback) {
    auto id_a = a->id();
    auto id_b = b->id();

    set_user_pointer(a);
    set_user_pointer(b);

    collision_listeners_[std::make_pair(id_a, id_b)] = callback;
}

void zombye::physics_system::register_collision_begin_callback(entity* a, entity* b, std::function<void(entity*, entity*)> callback) {
    auto id_a = a->id();
    auto id_b = b->id();

    set_user_pointer(a);
    set_user_pointer(b);

    collision_begin_listeners_[std::make_pair(id_a, id_b)] = callback;
}

void zombye::physics_system::register_collision_end_callback(entity* a, entity* b, std::function<void(entity*, entity*)> callback) {
    auto id_a = a->id();
    auto id_b = b->id();

    set_user_pointer(a);
    set_user_pointer(b);

    collision_end_listeners_[std::make_pair(id_a, id_b)] = callback;
}

bool zombye::physics_system::has_collision_callback(entity* a, entity* b) {
    auto id_a = a->id();
    auto id_b = b->id();

    auto it = collision_listeners_.find(std::make_pair(id_a, id_b));

    return it != collision_listeners_.end();
}

bool zombye::physics_system::has_collision_begin_callback(entity* a, entity* b) {
    auto id_a = a->id();
    auto id_b = b->id();

    auto it = collision_begin_listeners_.find(std::make_pair(id_a, id_b));

    return it != collision_begin_listeners_.end();
}

bool zombye::physics_system::has_collision_end_callback(entity* a, entity* b) {
    auto id_a = a->id();
    auto id_b = b->id();

    auto it = collision_end_listeners_.find(std::make_pair(id_a, id_b));

    return it != collision_end_listeners_.end();
}

void zombye::physics_system::fire_collision_callback(entity* a, entity* b) {
    auto id_a = a->id();
    auto id_b = b->id();

    if(has_collision_callback(a, b)) {
        collision_listeners_[std::make_pair(id_a, id_b)](a, b);
    }
}

void zombye::physics_system::fire_collision_begin_callback(entity* a, entity* b) {
    auto id_a = a->id();
    auto id_b = b->id();

    if(has_collision_begin_callback(a, b)) {
        collision_begin_listeners_[std::make_pair(id_a, id_b)](a, b);
    }
}

void zombye::physics_system::fire_collision_end_callback(entity* a, entity* b) {
    auto id_a = a->id();
    auto id_b = b->id();

    if(has_collision_end_callback(a, b)) {
        collision_end_listeners_[std::make_pair(id_a, id_b)](a, b);
    }
}

void zombye::physics_system::check_collisions() {
    auto num = world_->getDispatcher()->getNumManifolds();

    for(auto i = 0u; i < num; i++) {
        auto contact = world_->getDispatcher()->getManifoldByIndexInternal(i);

        auto a = static_cast<const btCollisionObject*>(contact->getBody0());
        auto b = static_cast<const btCollisionObject*>(contact->getBody1());

        auto num_contacts = contact->getNumContacts();

        for(auto j = 0u; j < num_contacts; j++) {
            auto& point = contact->getContactPoint(j);

            if(point.getDistance() < 0.f) {
                auto user_data_a = static_cast<physics_user_data*>(a->getUserPointer());
                auto user_data_b = static_cast<physics_user_data*>(b->getUserPointer());

                if(user_data_a != nullptr && user_data_b != nullptr) {
                    auto entity_a = game_.entity_manager().resolve(user_data_a->entity_id_);
                    auto entity_b = game_.entity_manager().resolve(user_data_b->entity_id_);

                    // both were set as user pointer
                    if(entity_a != nullptr && entity_b != nullptr) {
                        check_collision_begin_callback(entity_a, entity_b);

                        if(has_collision_callback(entity_a, entity_b)) {
                            fire_collision_callback(entity_a, entity_b);
                        } else if(has_collision_callback(entity_b, entity_a)) {
                            fire_collision_callback(entity_b, entity_a);
                        }
                    }
                }
            }
        }
    }

    //check_collision_end_callbacks();
}

void zombye::physics_system::set_user_pointer(entity* en) {
    auto physics_comp = en->component<physics_component>();

    btCollisionObject* obj = nullptr;

    // has no physics component? Maybe it's a character_physics_component
    if(physics_comp == nullptr) {
        auto character_physics_comp = en->component<character_physics_component>();

        if(character_physics_comp != nullptr) {
            obj = character_physics_comp->collision_object();
        } else {
            // this means the character has no kind of physics thing Oo
            zombye::log(LOG_ERROR, "Entity (id: " + std::to_string(en->id()) + ") has no physics or character controller component!");
        }
    } else {
        obj = physics_comp->collision_object();
    }

    if(obj != nullptr) {
        auto user_data = new physics_user_data;
        user_data->entity_id_ = en->id();

        obj->setUserPointer((void*)user_data);
    } else {
        zombye::log(LOG_ERROR, "Entity (id: " + std::to_string(en->id()) + ") can't set user pointer...");
    }
}

void zombye::physics_system::check_collision_begin_callback(entity* a, entity* b) {
    auto collided = [this](auto a, auto b) {
        auto pair = std::make_pair(a->id(), b->id());
        auto it = did_collide_.find(pair);

        if(it != did_collide_.end()) {
            did_collide_.at(pair) = true;
        }

        did_collide_[pair] = true;
    };

    // they didn't collide already, call begin callback
    if(!did_collide(a, b)) {
        collided(a, b);
        fire_collision_begin_callback(a, b);
    } else if(!did_collide(b, a)) {
        collided(b, a);
        fire_collision_begin_callback(b, a);
    }
}

void zombye::physics_system::check_collision_end_callbacks() {
    for(auto& c : did_collide_) {
        auto collision_happened = c.second;

        if(collision_happened) {
            auto id_pair = c.first;

            auto id_a = std::get<0>(id_pair);
            auto id_b = std::get<1>(id_pair);

            auto& em = game_.entity_manager();

            auto entity_a = em.resolve(id_a);
            auto entity_b = em.resolve(id_b);

            fire_collision_end_callback(entity_a, entity_b);
            did_collide_[id_pair] = false;
        }
    }
}

bool zombye::physics_system::did_collide(entity* a, entity* b) {
    auto pair = std::make_pair(a->id(), b->id());
    auto it = did_collide_.find(pair);

    if(it != did_collide_.end()) {
        return did_collide_.at(pair);
    }

    return false;
}
