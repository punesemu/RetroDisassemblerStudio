#pragma once

#include <memory>

#include "systems/nes/nes_memory.h"
#include "systems/nes/nes_system.h"

namespace NES {

class CartridgeView;
class System;

class Cartridge : public std::enable_shared_from_this<Cartridge> {
public:
    struct {
        u8 num_prg_rom_banks;
        u32 prg_rom_size;

        u8 num_chr_rom_banks;
        u32 chr_rom_size;

        u8 mapper;
        MIRRORING mirroring;

        bool has_sram;
        bool has_trainer;
    } header;

    Cartridge(std::shared_ptr<System>&);
    ~Cartridge();

    bool LoadHeader(u8*);

    bool                               CanBank(GlobalMemoryLocation const&);
    std::shared_ptr<ProgramRomBank>&   GetProgramRomBank(u8 bank) { return program_rom_banks[bank]; }
    std::shared_ptr<CharacterRomBank>& GetCharacterRomBank(u8 bank) { return character_rom_banks[bank]; }
    int                                GetNumMemoryRegions() const;
    std::shared_ptr<MemoryRegion>      GetMemoryRegion(GlobalMemoryLocation const&);
    std::shared_ptr<MemoryRegion>      GetMemoryRegionByIndex(int);

    std::shared_ptr<MemoryView>        CreateMemoryView();

    u16 GetResetVectorBank();

    void NoteReferences();

    u8 ReadProgramRom(int, u16);
    u8 ReadCharacterRom(int, u16);

    bool Save(std::ostream&, std::string&);
    bool Load(std::istream&, std::string&, std::shared_ptr<System>&);
private:
    void CreateMemoryRegions();

    std::weak_ptr<System> parent_system;

    std::vector<std::shared_ptr<ProgramRomBank>>   program_rom_banks;
    std::vector<std::shared_ptr<CharacterRomBank>> character_rom_banks;
};

class CartridgeView : public MemoryView {
public:
    CartridgeView(std::shared_ptr<Cartridge> const&);
    ~CartridgeView();

    u8 Read(u16) override;
    void Write(u16, u8) override;

    u8 ReadPPU(u16) override;
    void WritePPU(u16, u8) override;

    friend class Cartridge;

private:
    std::shared_ptr<Cartridge> cartridge;
    int prg_rom_bank_low;
    int prg_rom_bank_high;
};

}

