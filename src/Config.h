#pragma once

#include "ll/api/Config.h"

#include <string>
#include <vector>
#include <cstddef>

namespace SpawnerSetting {

struct Config {
    int version = 10;

    bool whitelistMode = false;

    bool enableFamilyFilter = false;
    std::vector<std::string> targetFamilies = {"zombie"};

    bool enableIdentifierFilter = false;
    std::vector<std::string> targetMonsterIds = {"minecraft:creeper"};

    float densityMultiplier   = 1.0f;
    float globalCapMultiplier  = 1.0f;

    bool useRegex = false;

    // 设置为 2 表示 2 倍速 即每刻尝试生成 2 次
    int spawnSpeed = 2;

    // Spawner 中记录当前生物数量的结构体内存偏移 (默认 552)
    size_t mobCountOffset = 552;
};

} // namespace SpawnerSetting
