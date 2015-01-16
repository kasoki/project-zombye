#ifndef __ZOMBYE_SKELETON_HPP__
#define __ZOMBYE_SKELETON_HPP__

#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace zombye {
    class rendering_system;
}

namespace zombye {
    struct bone {
        unsigned int id;
        glm::mat4 transform;
    };

    struct keyframe {
        float time;
        glm::vec3 translate;
        glm::quat rotate;
        glm::vec3 scale;
    };

    struct track {
        unsigned int id;
        std::vector<keyframe> keyframes;
    };

    struct animation {
        std::string name;
        float length;
        std::vector<track> tracks;
    };

    class skeleton {
        std::vector<bone> bones_;
        std::unordered_map<std::string, zombye::animation> animations_;
    public:
        skeleton(rendering_system& rendering_system, const std::vector<char>& source) noexcept;
        skeleton(const skeleton& other) = delete;
        skeleton(skeleton&& other) noexcept = default;
        skeleton() = default;
        skeleton& operator=(const skeleton& other) = delete;
        skeleton& operator=(skeleton&& other) noexcept = default;

        const auto& bones() const noexcept {
            return bones_;
        }

        const auto& animation(const std::string& name) const {
            auto it = animations_.find(name);
            if (it == animations_.end()) {
                throw std::invalid_argument("no animation named " + name + " in skeleton");
            }
            return it->second;
        }
    };
}

#endif