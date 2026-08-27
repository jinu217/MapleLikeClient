#include "Inventory.h"

namespace
{
    std::size_t indexOf(const ItemType type)
    {
        return static_cast<std::size_t>(type);
    }
}

bool Inventory::add(const ItemType type)
{
    unsigned int& count = counts[indexOf(type)];
    if (count >= StackLimit)
    {
        return false;
    }

    ++count;
    return true;
}

bool Inventory::consume(const ItemType type)
{
    unsigned int& count = counts[indexOf(type)];
    if (count == 0)
    {
        return false;
    }

    --count;
    return true;
}

unsigned int Inventory::getCount(const ItemType type) const
{
    return counts[indexOf(type)];
}
