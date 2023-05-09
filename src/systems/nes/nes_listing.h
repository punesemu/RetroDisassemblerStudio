#pragma once

#include "systems/nes/nes_defs.h"
#include "systems/nes/nes_memory.h"

namespace NES {

class System;
class ProgramRomBank;

// A single ListingItem translates to a single row in the Listing window. A listing item can be
// all sorts of row types: comments, labels, actual code, data, etc.
class ListingItem {
public:
    ListingItem()
    { 
    }
    virtual ~ListingItem() { }

    virtual void RenderContent(std::shared_ptr<System>&, GlobalMemoryLocation const&) = 0;

protected:
};

class ListingItemUnknown : public ListingItem {
public:
    ListingItemUnknown()
        : ListingItem()      
    { }
    virtual ~ListingItemUnknown() { }

    void RenderContent(std::shared_ptr<System>&, GlobalMemoryLocation const&) override;
};

class ListingItemData : public ListingItem {
public:
    ListingItemData(std::weak_ptr<MemoryRegion> _memory_region, u32 _internal_offset)
        : ListingItem(), memory_region(_memory_region), internal_offset(_internal_offset)
    { }
    virtual ~ListingItemData() { }

    void RenderContent(std::shared_ptr<System>&, GlobalMemoryLocation const&) override;

private:
    std::weak_ptr<MemoryRegion> memory_region;
    u32 internal_offset;
};

class ListingItemLabel : public ListingItem {
public:
    ListingItemLabel(std::string const& _name)
        : ListingItem(), label_name(_name) 
    { }
    virtual ~ListingItemLabel() { }

    void RenderContent(std::shared_ptr<System>&, GlobalMemoryLocation const&) override;

private:
    std::string label_name;
};

}

