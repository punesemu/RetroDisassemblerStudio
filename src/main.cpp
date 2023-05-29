#include "config.h"

#include <filesystem>
#include <iostream>
#include <locale>
#include <sstream>
#include <vector>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "cfgpath.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

#define USE_IMGUI_TABLES
#include "ImGuiFileDialog.h"
#include "dirent/dirent.h"

#include "main.h"
#include "systems/nes/nes_expressions.h"
#include "systems/nes/nes_label.h"
#include "systems/nes/nes_memory.h"
#include "systems/nes/nes_project.h"
#include "systems/nes/nes_system.h"
#include "windows/baseproject.h"
#include "windows/rom_loader.h"
#include "windows/nes/defines.h"
#include "windows/nes/emulator.h"
#include "windows/nes/labels.h"
#include "windows/nes/listing.h"
#include "windows/nes/regions.h"

#include "systems/expressions.h" // TODO temp

#undef DISABLE_IMGUI_SAVE_LOAD_LAYOUT

using namespace std;

MyApp::MyApp(int, char*[])
    : Application("Retro Disassembler Studio", 1800, 1200),
      request_exit(false)
{
    Windows::BaseProject::RegisterProjectInformation(Systems::NES::Project::GetInformationStatic());

    BaseExpressionNodeCreator::RegisterBaseExpressionNodes();
    Systems::NES::ExpressionNodeCreator::RegisterExpressionNodes();

    // register all the windows
#   define REGISTER_WINDOW_TYPE(className) \
        create_window_functions[className::GetWindowClassStatic()] = std::bind(&className::CreateWindow);
    //!REGISTER_WINDOW_TYPE(NES::Windows::Listing);
    //!REGISTER_WINDOW_TYPE(NES::Windows::MemoryRegions);
#   undef REGISTER_WINDOW_TYPE
}

MyApp::~MyApp()
{
}

std::shared_ptr<Windows::BaseWindow> MyApp::CreateMainWindow()
{
    auto main_window = Windows::MainWindow::CreateWindow();

    *main_window->command_signal += [this](shared_ptr<BaseWindow>&, string const& cmd, void*) {
        if(cmd == "RequestExit") this->request_exit = true;
    };

    return main_window;
}

bool MyApp::OnPlatformReady()
{
    // initialize the glClear color for the platform
    clear_color[0] = 0.9375;
    clear_color[1] = 0.9453125;
    clear_color[2] = 0.95703125;
    clear_color[3] = 1.0;

    // set filename to null to disable auto-save/load of ImGui layouts
    // and use ImGui::SaveIniSettingsToDisk() instead.
    ImGuiIO& io = ImGui::GetIO();
#if defined(DISABLE_IMGUI_SAVE_LOAD_LAYOUT)
    io.IniFilename = nullptr;
#else
    char cfgdir[MAX_PATH];
    get_user_config_folder(cfgdir, sizeof(cfgdir), PROJECT_NAME);
    string config_dir(cfgdir);
    layout_file = config_dir + PATH_SEPARATOR_STRING + "imgui_layout.ini";
    io.IniFilename = layout_file.c_str();
#endif

    cout << "[MyApp] ImGui layout file is " << io.IniFilename << endl;

    // Connect handlers for ImGui to store layout data
//!    SetupINIHandlers();

    // load some fonts
    // TODO everything will one day be user customizable
    ImFont* default_font = io.Fonts->AddFontDefault();

    main_font = io.Fonts->AddFontFromFileTTF("ext/iosevka-regular.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesDefault());
    if(main_font == nullptr) {
        cout << "[MyApp] Warning: unable to load iosevka-regular.ttf. Using default font." << endl;
        main_font = default_font;
    }

    main_font_bold = io.Fonts->AddFontFromFileTTF("ext/iosevka-heavy.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesDefault());
    if(main_font_bold == nullptr) { 
        cout << "[MyApp] Warning: unable to load iosevka-bold.ttf. Using default font." << endl;
        main_font_bold = default_font;
    }

    IM_ASSERT(main_font != NULL);
    IM_ASSERT(main_font_bold != NULL);

    // replace the default font
    if(main_font != default_font) {
        io.FontDefault = static_cast<ImFont*>(main_font);
    }

    // scale up some
    io.FontGlobalScale = 1.2f;

    // Create a style
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding.x    = 1;
    style.WindowPadding.y    = 1;
    style.FramePadding.x     = 3;
    style.FramePadding.y     = 3;
    style.CellPadding.x      = 2;
    style.CellPadding.y      = 1;
    style.ItemSpacing.x      = 8;
    style.ItemSpacing.y      = 5;
    style.ScrollbarSize      = 12;
    style.GrabMinSize        = 13;
    style.WindowTitleAlign.x = 0.5f;

    return true;
}

void MyApp::SetupINIHandlers()
{
//!    ImGuiSettingsHandler ini_handler;
//!
//!    ini_handler.TypeName = "RetroGameDisassemblerLayout";
//!    ini_handler.TypeHash = ImHashStr("RetroGameDisassemblerLayout");
//!    
//!    ini_handler.ClearAllFn = [](ImGuiContext*, ImGuiSettingsHandler*) {
//!        // TODO not sure when this is called
//!        assert(false); 
//!    };
//!
//!    ini_handler.ReadOpenFn = [](ImGuiContext*, ImGuiSettingsHandler*, char const* name) -> void* {
//!        // name contains the value in the second set of []. we don't use it, we just assume
//!        // the order is correct, and if it isn't, it really isn't a big deal
//!        WindowFromINI* wfini = MyApp::Instance()->NewINIWindow();
//!        return (void*)wfini;
//!    };
//!
//!    ini_handler.ReadLineFn = [](ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line) {
//!        // for each line within the ini section, this function gets called
//!        WindowFromINI* wfini = (WindowFromINI*)entry;
//!
//!        char buffer[64];
//!        if(sscanf(line, "WindowClass=%63[^\r\n]", buffer) == 1) {
//!            wfini->window_class = string(buffer);
//!        } else if(sscanf(line, "WindowID=%16[^\r\n]", buffer) == 1) {
//!            wfini->window_id = string(buffer);
//!        }
//!
//!    };
//!
//!    ini_handler.ApplyAllFn = [](ImGuiContext*, ImGuiSettingsHandler*) {
//!        // after the entire ini file is loaded, this function is called and we create the windows
//!        MyApp::Instance()->CreateINIWindows();
//!    };
//!
//!    ini_handler.WriteAllFn = [](ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf) {
//!        // this function is called to output data to the ini file
//!
//!        // loop over all managed windows and add them to the ini file under their own heading
//!        MyApp* instance = MyApp::Instance();
//!        int window_index = 0;
//!        for(auto &window : instance->managed_windows) {
//!            buf->appendf("[%s][%d]\n", handler->TypeName, window_index);
//!            buf->appendf("WindowClass=%s\n", window->GetWindowClass());
//!            buf->appendf("WindowID=%s\n", window->GetWindowID().c_str());
//!            buf->appendf("\n");
//!            window_index++;
//!        }
//!    };
//!
//!    // add the handler to the ImGuiContext
//!    ImGuiContext& g = *ImGui::GetCurrentContext();
//!    g.SettingsHandlers.push_back(ini_handler);
}

MyApp::WindowFromINI* MyApp::NewINIWindow()
{
    shared_ptr<WindowFromINI> wfini = make_shared<WindowFromINI>();
    ini_windows.push_back(wfini);
    return wfini.get(); // considered unsafe, but I know it's not stored for use later
}

void MyApp::CreateINIWindows()
{
#if 0 // TODO temporarily disabling the creationg of the windows from the INI file.
      // later, we will want to recreate the last open project. or not? let the user pick.
    // loop over all the INI windows and create them
    for(auto& wfini : ini_windows) {
        if(!create_window_functions.contains(wfini->window_class)) {
            cout << "[MyApp] warning: class type " << wfini->window_class << " from INI doesn't exist" << endl;
            continue;
        }

        // create the window
        auto& create_function = create_window_functions[wfini->window_class];
        auto wnd = create_function();

        // set the ID to match the one in the file
        wnd->SetWindowID(wfini->window_id);

        // add it to the managed windows list
        AddWindow(wnd);
    }
#endif

    // free memory
    ini_windows.clear();
}

bool MyApp::Update(double deltaTime)
{
    return !request_exit;
}

int main(int argc, char* argv[])
{
    return MyApp::Instance(argc, argv)->Run();
}
