// Copyright (c) 2023, Charles Mason <chuck+github@borboggle.com>
// All rights reserved.
// 
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. 
#include <iomanip>
#include <iostream>
#include <sstream>

#include "imgui.h"
#include "imgui_internal.h"

#include "systems/nes/defines.h"
#include "systems/nes/enum.h"
#include "systems/nes/label.h"
#include "systems/nes/memory.h"
#include "systems/nes/system.h"

#include "windows/nes/emulator.h"
#include "windows/nes/defines.h"
#include "windows/nes/listing.h"
#include "windows/nes/project.h"
#include "windows/nes/references.h"

using namespace std;

namespace Windows::NES {

REGISTER_WINDOW(References);

shared_ptr<References> References::CreateWindow()
{
    return make_shared<References>(shared_ptr<Label>(nullptr));
}

shared_ptr<References> References::CreateWindow(reference_type const& reference_to)
{
    return make_shared<References>(reference_to);
}

References::References(reference_type const& _reference_to)
    : BaseWindow(), reference_to(_reference_to), selected_row(-1)
{
    SetNoScrollbar(true);
   
    // create internal signals

    if(auto system = GetSystem()) {
        // grab a weak_ptr so we don't have to continually use dynamic_pointer_cast
        current_system = system;

        stringstream ss;
        ss << "References: ";

        if(auto label = get_if<shared_ptr<Label>>(&reference_to)) {
            ss << (*label)->GetString();
            changed_connection = (*label)->reverse_references_changed->connect([this]() {
                need_repopulate = true;
            });

            label_deleted_connection = system->label_deleted->connect([this, label](shared_ptr<Label> const& other, int nth) {
                // if our label is deleted, close the window
                if(other->GetString() == (*label)->GetString()) {
                    CloseWindow();
                }
            });
        } else if(auto define = get_if<shared_ptr<Define>>(&reference_to)) {
            ss << (*define)->GetName();
            changed_connection = (*define)->reverse_references_changed->connect([this]() {
                need_repopulate = true;
            });
        } else if(auto ee = get_if<shared_ptr<EnumElement>>(&reference_to)) {
            ss << (*ee)->GetName();
            changed_connection = (*ee)->reverse_references_changed->connect([this]() {
                need_repopulate = true;
            });
        }

        SetTitle(ss.str());
 
        // watch for new labels
        //label_created_connection = 
        //    system->label_created->connect(std::bind(&References::LabelCreated, this, placeholders::_1, placeholders::_2));
    }
}

References::~References()
{
}

void References::Update(double deltaTime) 
{
    if(need_repopulate) {
        PopulateLocations();
        need_repopulate = false;
        need_resort = true;
    }
}

void References::Render() 
{
    // All access goes through the system
    auto system = current_system.lock();
    if(!system) return;

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    static ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterH 
            | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Sortable
            | ImGuiTableFlags_ScrollY;

    if(ImGui::BeginTable("ReferencesTable", 1, flags)) {
        ImGui::TableSetupColumn("Location", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort, 0.0f, 0);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        // Sort our data if sort specs have been changed!
        if(ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs(); sort_specs->SpecsDirty || need_resort) {
            sort(locations.begin(), locations.end(), [&](location_type& a, location_type& b)->bool {
                const ImGuiTableColumnSortSpecs* spec = &sort_specs->Specs[0];

                bool diff = false; // result

                // determine the types. Defines come before memory locations
                auto a_memory = get_if<shared_ptr<GlobalMemoryLocation>>(&a);
                auto a_define = get_if<shared_ptr<Define>>(&a);
                auto a_ee     = get_if<shared_ptr<EnumElement>>(&a);
                auto b_memory = get_if<shared_ptr<GlobalMemoryLocation>>(&b);
                auto b_define = get_if<shared_ptr<Define>>(&b);
                auto b_ee     = get_if<shared_ptr<EnumElement>>(&b);

                if(a_memory && b_memory) {
                    diff = system->GetSortableMemoryLocation(**a_memory) <= system->GetSortableMemoryLocation(**b_memory);
                } else if(a_define && b_define) {
                    diff = (*a_define)->GetName() <= (*b_define)->GetName(); // standard string compare
                } else if(a_ee && a_ee) {
                    diff = (*a_ee)->GetName() <= (*b_ee)->GetName(); // standard string compare
                }

                // flip the direction if descending
                if(spec->SortDirection == ImGuiSortDirection_Descending) diff = !diff;

                return diff;
            });

            sort_specs->SpecsDirty = false;
        }

        for(int i = 0; i < locations.size(); i++) {
            auto location = locations[i];

            stringstream ss;
            std::function<void()> go;
            if(auto memory_ptr = get_if<shared_ptr<GlobalMemoryLocation>>(&location)) {
                auto memory = *memory_ptr;

                if(system->CanBank(*memory)) {
                    if(auto memory_region = system->GetMemoryRegion(*memory)) {
                        ss << memory_region->GetName() << ":";
                    }
                }

                (*memory).FormatAddress(ss, false, false);

                go = [this, memory]() {
                    if(auto listing = GetMyListing()) {
                        listing->GoToAddress(*memory, true);
                    }
                };
            } else if(auto define = get_if<shared_ptr<Define>>(&location)) {
                ss << "Define: " << (*define)->GetName();

                go = [this, define]() {
                    cout << WindowPrefix() << "TODO: Highlight define" << endl;
                    //!if(auto wnd = GetMySystemInstance()->FindMostRecentChildWindow<Windows::NES::Defines>()) {
                    //!    auto deref = *define;
                    //!    wnd->Highlight(deref);
                    //!}
                };
            } else if(auto ee = get_if<shared_ptr<EnumElement>>(&location)) {
                ss << "Enum: " << (*ee)->GetFormattedName("_");

                go = [this, ee]() {
                    cout << WindowPrefix() << "TODO: Highlight enum" << endl;
                };
            }

            // Render location text
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
            char buf[64];
            snprintf(buf, sizeof(buf), "%s##rlt_selectable_row%d", ss.str().c_str(), i);

            // go to the selected address when a row is selected
            if(ImGui::Selectable(buf, selected_row == i, selectable_flags)) {
                selected_row = i;

                go();
            }
        }

        ImGui::EndTable();
    }

    ImGui::PopStyleVar(2);
}

void References::PopulateLocations()
{
    locations.clear();
	std::visit([this](auto&& arg) { PopulateLocations(arg); }, reference_to);	
}

} //namespace Windows::NES

