#pragma once

#include <memory>
#include <string>

#include "systems/nes/nes_memory.h"

namespace NES {

class Label : public std::enable_shared_from_this<Label> {
public:
    Label(GlobalMemoryLocation const&, std::string const&);
    ~Label();


    GlobalMemoryLocation const& GetMemoryLocation() const { return memory_location; }
    std::string          const& GetString()         const { return label; }

private:
    GlobalMemoryLocation memory_location;
    std::string          label;
};

}
