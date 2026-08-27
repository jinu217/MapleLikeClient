#pragma once

#include <array>

#include "ItemType.h"

class Inventory
{
public:
    bool add(ItemType type);
    bool consume(ItemType type);
    [[nodiscard]] unsigned int getCount(ItemType type) const;

private:
    static constexpr unsigned int StackLimit = 20;
    std::array<unsigned int, static_cast<std::size_t>(ItemType::Count)> counts{};
};
