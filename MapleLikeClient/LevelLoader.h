#pragma once

#include <filesystem>

#include "LevelData.h"

class LevelLoader
{
public:
    [[nodiscard]] static LevelData load(const std::filesystem::path& path);
    [[nodiscard]] static std::filesystem::path findLevel(const std::filesystem::path& relativePath);
};
