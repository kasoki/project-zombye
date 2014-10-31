#ifndef __ZOMBYE_ENTITY_MANAGER_HPP__
#define __ZOMBYE_ENTITY_MANAGER_HPP__

#include <memory>
#include <queue>
#include <unordered_map>

#include <zombye/ecs/entity.hpp>

namespace zombye {
    class game;
    class entity_manager {
        game& game_;
        std::unordered_map<unsigned long, std::unique_ptr<entity>> entities_;
        std::queue<unsigned long> deletion_;
    public:
        entity_manager(game& game) noexcept;
        entity_manager(const entity_manager& other) = delete;
        entity_manager(entity_manager&& other) = delete;
        zombye::entity& emplace(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scalation);

        template <typename... arguments>
        zombye::entity& emplace(const std::string& name, arguments... args) {
            // TODO: create new entity
        }

        void erase(unsigned long id);
        void clear();
        zombye::entity* entity(unsigned long id) noexcept;
        entity_manager& operator= (const entity_manager& other) = delete;
        entity_manager& operator= (entity_manager&& other) = delete;
    };
}

#endif