#pragma once

#include <string>

class MyApp;

class BaseWindow {
public:
    BaseWindow(MyApp*, std::string const& title);
    virtual ~BaseWindow();

    // Utility
    void SetHidden(bool);
    bool IsHidden();

    // Called from the main application
    void Update(double deltaTime);
    void RenderGUI();

protected:
    // Implemented by derived class
    virtual void UpdateContent(double deltaTime) {};
    virtual void RenderContent() {};

    MyApp* main_application;
private:
    std::string window_title;
    bool hidden;

    static unsigned int next_id;
};
