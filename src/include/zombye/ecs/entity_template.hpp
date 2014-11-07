#ifndef __ZOMBYE_ENTITY_TEMPLATE_HPP__
#define __ZOMBYE_ENTITY_TEMPLATE_HPP__

#include <memory>
#include <string>
#include <vector>

#include <zombye/ecs/value_pack.hpp>

namespace zombye {
    class entity_template {
        std::string name_;
        std::vector<std::unique_ptr<value_pack>> value_packs_;
    public:
        entity_template(const std::string& name) noexcept;
        entity_template(const entity_template& other) = delete;
        entity_template(entity_template&& other) = delete;
        void emplace_back(value_pack* value_pack);
        const std::string& name() const {
            return name_;
        }
        auto begin() const {
            return value_packs_.begin();
        }
        auto end() const {
            return value_packs_.end();
        }
        const auto& get() const {
            return value_packs_;
        }
        entity_template& operator= (const entity_template& other) = delete;
        entity_template& operator= (entity_template&& other) = delete;
    };
}

#endif