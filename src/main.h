#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "application.h"
#include "project.h"
#include "signals.h"
#include "systems/nes/nes_memory.h"
#include "windows/base_window.h"

class BaseProject;

class MyApp : public Application {
public:
    static MyApp* Instance(int argc = 0, char** argv = nullptr) {
        static MyApp* instance = nullptr;
        if(instance == nullptr) {
            instance = new MyApp(argc, argv);
        }
        return instance;
    }

    // singleton helper
    MyApp(MyApp const& other) = delete;
    void operator=(MyApp const&) = delete;
    virtual ~MyApp();

    void AddWindow(std::shared_ptr<BaseWindow>);

    // Helper dialog boxes
    bool OKPopup(std::string const& title, std::string const& message);

    // Project
    std::shared_ptr<BaseProject> GetProject() { return current_project; }

    // Signals
//!    typedef signal<std::function<void()>> current_system_changed_t;
//!    std::shared_ptr<current_system_changed_t> current_system_changed;

protected:
    MyApp(int argc, char* argv[]);


    virtual bool Update(double deltaTime) override;
    virtual void RenderGUI() override;
    virtual void RenderMainMenuBar() override;
    virtual void RenderMainStatusBar() override;
    virtual bool OnWindowCreated() override;
    virtual void OnKeyPress(int glfw_key, int scancode, int action, int mods) override;

private:
    void ProcessQueuedWindowsForDelete();
    void ManagedWindowClosedHandler(std::shared_ptr<BaseWindow>);

    void CreateNewProject(std::string const&);
    void ProjectCreatedHandler(std::shared_ptr<BaseWindow>, std::shared_ptr<BaseProject>);

    void CloseProject();

    void OpenROMInfosPane();

    void ListingWindowCommand(std::shared_ptr<BaseWindow> const&, std::string const&, NES::GlobalMemoryLocation const&);

    // Popups. Each of these are called every frame
    void RenderPopups();
    void CreateLabelPopup();
    void DisassemblyPopup();
    void GoToAddressPopup();
    void SaveProjectPopup();
    void SaveProjectThread();
    void LoadProjectPopup();
    void LoadProjectThread();

    bool request_exit;
    bool show_imgui_demo;

    std::string layout_file;

    // ImGui fonts
    void* main_font;
    void* main_font_bold;

    // Managed child windows
    std::vector<std::shared_ptr<BaseWindow>> managed_windows;
    std::vector<std::shared_ptr<BaseWindow>> queued_windows_for_delete;

    // Global popups
    struct {
        struct {
            std::string title = "Create new label...";
            bool        show  = false;
            int         edit;
            char        buf[64];
            std::shared_ptr<void> uhg; // TODO need a base class for GlobalMemoryLocation
        } create_label;

        struct {
            std::shared_ptr<std::thread> thread = nullptr;
            std::string title = "Disassembling...";
            bool        show  = false;
        } disassembly;

        struct {
            std::shared_ptr<BaseWindow> listing;
            std::string title = "Go to address...";
            char        buf[64];
            bool        show = false;
        } goto_address;

        struct {
            std::string title = "Saving Project...";
            std::shared_ptr<std::thread> thread;
            bool show = false;
            bool saving = false;
            bool errored = false;
            std::string errmsg;
        } save_project;

        struct {
            std::string title = "Loading Project...";
            std::shared_ptr<std::thread> thread;
            bool show = false;
            bool loading = true;
            bool errored = false;
            std::string errmsg;
        } load_project;
    } popups;

private:
    std::shared_ptr<BaseProject> current_project;

    typedef std::function<std::shared_ptr<BaseWindow>(void)> create_window_func;
    std::map<std::string, create_window_func> create_window_functions;

private:
    struct WindowFromINI {
        std::string window_class;
        std::string window_id;
    };

    void SetupINIHandlers();
    WindowFromINI* NewINIWindow();
    void CreateINIWindows();
    std::vector<std::shared_ptr<WindowFromINI>> ini_windows;

    std::string project_file_path;
};
