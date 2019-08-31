#pragma once
#include "object.h"
namespace Optifuser {
std::vector<std::unique_ptr<Object>>
LoadObj(const std::string file, bool ignoreSpecification = true,
        glm::vec3 upAxis = {0, 1, 0}, glm::vec3 forwardAxis = {0, 0, -1});
}
