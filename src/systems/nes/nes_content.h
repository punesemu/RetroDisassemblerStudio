#pragma once

#include "systems/nes/nes_defs.h"
#include "systems/nes/nes_memory.h"

namespace NES {

class System;
class ProgramRomBank;

// Content blocks can be data, code, or graphics
// Data can be structured, arrays, etc. So we will need types, arrays, etc
// Will need built-in types like byte, word, function pointer, etc.
struct ContentBlock {
    CONTENT_BLOCK_TYPE type;    // CODE statement, DATA block, CHR, possibly other things
    u16                offset;  // Offset within the rom bank that this content block starts at
    u16                num_listing_items; // number of listing items this ContentBlock produces in the Listing window

    union {
        struct {
            // code contentblock is always one instruction long
        } code;

        struct {
            CONTENT_BLOCK_DATA_TYPE type; // byte, word, pointer, user defined
            u16   count;   // Total number of elements
            u8    elements_per_line; // When displaying the contents of this data, how many elements per line we should display
            void* ptr;     // TODO will need a structure to hold various content blocks
        } data;

        struct {
        } chr;
    };

    u32 GetDataTypeSize() const {
        switch(data.type) {
        case CONTENT_BLOCK_DATA_TYPE_UBYTE:
            return 1;
        case CONTENT_BLOCK_DATA_TYPE_UWORD:
            return 2;
        default:
            assert(false);
            return 0;
        }
    }

    // return size in bytes
    u32 GetSize() const {
        switch(type) {
        case CONTENT_BLOCK_TYPE_DATA:
            return data.count * GetDataTypeSize();

        default:
            assert(false); // TODO
            return 0;
        }
    }

    std::string FormatInstructionField();
    std::string FormatDataElement(u16 n);

};

// A single ListingItem translates to a single row in the Listing window. A listing item can be
// all sorts of row types: comments, labels, actual code, data, etc.
class ListingItem {
public:
    enum TYPE {
        LISTING_ITEM_TYPE_UNKNOWN,
        LISTING_ITEM_TYPE_DATA,
        LISTING_ITEM_TYPE_LABEL,
    };

    ListingItem(TYPE type, GlobalMemoryLocation const& where) :
        item_type(type), global_memory_location(where) 
    { 
    }
    virtual ~ListingItem() { }

    //TYPE GetType() const { return item_type; }
    virtual void RenderContent(std::shared_ptr<System>&) = 0;

protected:
    TYPE                 item_type;
    GlobalMemoryLocation global_memory_location;
};

class ListingItemUnknown : public ListingItem {
public:
    ListingItemUnknown(GlobalMemoryLocation const& where)
        : ListingItem(ListingItem::LISTING_ITEM_TYPE_UNKNOWN, where)      
    { 
    }
    virtual ~ListingItemUnknown() { }

    void RenderContent(std::shared_ptr<System>&) override;
};

class ListingItemData : public ListingItem {
public:
    ListingItemData(GlobalMemoryLocation const& where, std::shared_ptr<ProgramRomBank>& _prg_bank)
        : ListingItem(ListingItem::LISTING_ITEM_TYPE_DATA, where), prg_bank(_prg_bank) 
    { 
    }
    virtual ~ListingItemData() { }

    void RenderContent(std::shared_ptr<System>&) override;

private:
    std::shared_ptr<ProgramRomBank> prg_bank;
};

class ListingItemLabel : public ListingItem {
public:
    ListingItemLabel(GlobalMemoryLocation const& where, std::string const& _name)
        : ListingItem(ListingItem::LISTING_ITEM_TYPE_LABEL, where), label_name(_name) 
    { 
    }
    virtual ~ListingItemLabel() { }

    void RenderContent(std::shared_ptr<System>&) override;

private:
    std::string label_name;
};

}

