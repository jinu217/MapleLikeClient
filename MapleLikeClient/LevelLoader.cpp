#include "LevelLoader.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <Windows.h>

#include "third_party/nlohmann/json.hpp"

namespace
{
    sf::Vector2f readVector(const nlohmann::json& value, const std::string& fieldName)
    {
        if (!value.is_array() || value.size() != 2
            || !value[0].is_number() || !value[1].is_number())
        {
            throw std::runtime_error(fieldName + " must be an array containing two numbers");
        }

        return { value[0].get<float>(), value[1].get<float>() };
    }

    void validatePositive(const sf::Vector2f value, const std::string& fieldName)
    {
        if (value.x <= 0.f || value.y <= 0.f)
        {
            throw std::runtime_error(fieldName + " values must be positive");
        }
    }

    std::filesystem::path executableDirectory()
    {
        std::vector<wchar_t> buffer(512);

        while (true)
        {
            const DWORD length = GetModuleFileNameW(
                nullptr,
                buffer.data(),
                static_cast<DWORD>(buffer.size()));
            if (length == 0)
            {
                throw std::runtime_error("Unable to determine executable directory");
            }
            if (length < buffer.size() - 1)
            {
                return std::filesystem::path(std::wstring(buffer.data(), length)).parent_path();
            }

            buffer.resize(buffer.size() * 2);
        }
    }
}

LevelData LevelLoader::load(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input)
    {
        throw std::runtime_error("Unable to open level file: " + path.string());
    }

    try
    {
        const nlohmann::json root = nlohmann::json::parse(input);
        LevelData level;
        level.worldSize = readVector(root.at("worldSize"), "worldSize");
        level.playerSpawn = readVector(root.at("playerSpawn"), "playerSpawn");
        validatePositive(level.worldSize, "worldSize");

        for (const nlohmann::json& item : root.at("platforms"))
        {
            PlatformData platform{
                readVector(item.at("position"), "platform.position"),
                readVector(item.at("size"), "platform.size")
            };
            validatePositive(platform.size, "platform.size");
            level.platforms.push_back(platform);
        }

        for (const nlohmann::json& item : root.at("enemies"))
        {
            level.enemies.push_back({ readVector(item.at("position"), "enemy.position") });
        }

        for (const nlohmann::json& item : root.at("checkpoints"))
        {
            CheckpointData checkpoint{
                readVector(item.at("position"), "checkpoint.position"),
                readVector(item.at("size"), "checkpoint.size")
            };
            validatePositive(checkpoint.size, "checkpoint.size");
            level.checkpoints.push_back(checkpoint);
        }

        if (level.platforms.empty())
        {
            throw std::runtime_error("platforms must contain at least one platform");
        }

        return level;
    }
    catch (const std::exception& error)
    {
        throw std::runtime_error("Invalid level file " + path.string() + ": " + error.what());
    }
}

std::filesystem::path LevelLoader::findLevel(const std::filesystem::path& relativePath)
{
    const std::filesystem::path current = std::filesystem::current_path();
    const std::filesystem::path candidates[]{
        executableDirectory() / relativePath,
        current / relativePath,
        current / "MapleLikeClient" / relativePath,
        current.parent_path() / relativePath
    };

    for (const std::filesystem::path& candidate : candidates)
    {
        if (std::filesystem::exists(candidate))
        {
            return std::filesystem::canonical(candidate);
        }
    }

    throw std::runtime_error("Unable to locate level: " + relativePath.string());
}
