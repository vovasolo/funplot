// Function plotter based on VFormula class
// UI is based on Dear ImGui demo, a standalone example application for SDL2 + OpenGL

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <stdio.h>
#include <SDL.h>
#include <cmath>
#include <string>
#include "vformula.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif
#ifdef _WIN32
#include <windows.h>        // SetProcessDPIAware()
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../emscripten/emscripten_mainloop_stub.h"
#endif

//#include <iostream>

struct parameter {
    std::string name;
    float value;
    float min;
    float max;
    parameter(std::string name_, float val, float min_, float max_) : name(name_), value(val), min(min_), max(max_) {;}
};

// Main code
int main(int, char**)
{
// setup vformula
    VFormula vf;
    std::string formula;
    int errpos = 1024;
    int n_const = vf.GetConstCount();
    std::vector <parameter> pars;

// Implot stuff
    float xmin=0, xmax=1;
    float ymin=0, ymax=1;
    bool new_limits = false;

    // Setup SDL
#ifdef _WIN32
    ::SetProcessDPIAware();
#endif
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    float main_scale = ImGui_ImplSDL2_GetContentScaleForDisplay(0);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return 1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr)
    {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details. If you like the default font but want it to scale better, consider using the 'ProggyVector' from the same author!
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    // Background color
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Show control window 
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("VFormula plotter"); 
            
            static char buf[256], par_name[16];
            static float par_val, par_min, par_max;

            ImGui::Text("y = "); ImGui::SameLine();
            ImGui::InputText( "##formula", buf, 256);

            ImGui::SameLine();
            
            if (ImGui::Button("Plot")) {
                formula = std::string(buf);
                errpos = vf.ParseExpr(formula);
            }

            if (errpos == 1024) {
                if (vf.Validate())
                    ImGui::Text("OK");
                else {
                    ImGui::Text("Fail");
                    errpos = 0;
                }
            } else {
                if (errpos != 0) {
                    ImGui::Text("Err."); ImGui::SameLine();
    //               ImGui::Text(buf);
                    ImGui::Text((std::string(errpos, ' ')+ "^").c_str());
                } else {
                    ImGui::Text("Fail:"); ImGui::SameLine();
                    ImGui::Text(vf.GetErrorString().c_str());
                }
            }

            ImGui::SeparatorText("Plot limits");
            ImGui::SetNextItemWidth(200);
            if (ImGui::InputFloat("xmin", &xmin, 0.1, 1.0))
                new_limits = true; 
            ImGui::SetNextItemWidth(200);
            ImGui::SameLine(); 
            if (ImGui::InputFloat("xmax", &xmax,  0.1, 1.0))
                new_limits = true;
            ImGui::SetNextItemWidth(200);
            if (ImGui::InputFloat("ymin", &ymin,  0.1, 1.0))
                new_limits = true; 
            ImGui::SetNextItemWidth(200);
            ImGui::SameLine(); 
            if (ImGui::InputFloat("ymax", &ymax,  0.1, 1.0))
                new_limits = true;

            ImGui::SeparatorText("Parameters");
            for (size_t i=0; i<pars.size(); i++) {
                parameter p = pars[i];
                ImGui::SliderFloat(p.name.c_str(), &pars[i].value, p.min, p.max);
                vf.SetConstant(p.name.c_str(), pars[i].value);
            }
                
            if (ImGui::Button("Add parameter"))
                ImGui::OpenPopup("add_par_popup");

            if (ImGui::BeginPopup("add_par_popup"))
            {
                ImGui::InputText("Name:", par_name, 16);
                ImGui::InputFloat("Value", &par_val);
                ImGui::InputFloat("Min", &par_min);
                ImGui::InputFloat("Max", &par_max);
                if (ImGui::Button("Add")) {
                    vf.AddConstant(par_name, par_val);
                    pars.push_back(parameter(par_name, par_val, par_min, par_max));
                }
                ImGui::EndPopup();
            }

            // Stack machine under the collapsing header
            if (ImGui::CollapsingHeader("Stack machine", ImGuiTreeNodeFlags_None)) {
                std::vector <std::string> prg = vf.GetPrg();
                //ImGui::SeparatorText("Byte code");
                if (ImGui::TreeNode("Byte code")) {
                    for (auto &cmd : prg) {
                        ImGui::Text(cmd.c_str());
                    }
                    ImGui::TreePop();
                }
                ImGui::SeparatorText("Memory map");
                if (ImGui::TreeNodeEx("Constants", ImGuiTreeNodeFlags_DefaultOpen)) {
                    std::vector <std::string> cmap = vf.GetConstMap();
                    for (int i=0; i<cmap.size(); i++) {
                        ImGui::Text("%02d: ", i); ImGui::SameLine();
                        ImGui::Text(cmap[i].c_str());
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNodeEx("Variables", ImGuiTreeNodeFlags_DefaultOpen)) {
                    std::vector <std::string> vmap = vf.GetVarMap();
                    for (int i=0; i<vmap.size(); i++) {
                        ImGui::Text("%02d: ", i); ImGui::SameLine();
                        ImGui::Text(vmap[i].c_str());
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Functions")) {
                    std::vector <std::string> fmap = vf.GetFuncMap();
                    for (int i=0; i<fmap.size(); i++) {
                        ImGui::Text("%02d: ", i); ImGui::SameLine();
                        ImGui::Text(fmap[i].c_str());
                    }
                    ImGui::TreePop();
                }
                //for (int i = 0; i < 5; i++)
                //    ImGui::Text("Some content %d", i);
            }

            ImGui::SeparatorText("Execution stats");

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // Show window with the function plot
        {
//            static float xmin=0, xmax=1;
            
            float xs1[1001], ys1[1001];
            for (int i = 0; i < 1001; ++i) {
                xs1[i] = xmin + i * 0.001f * (xmax - xmin);
                ys1[i] = errpos == 1024 ? vf.Eval(xs1[i]) : 0.;
            }

            ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
            ImGui::Begin("ImPlot window");
            if (ImPlot::BeginPlot("Line Plots", ImVec2(-1, -1))) {
                ImPlot::SetupAxes("x","y");
                ImPlot::SetupAxesLimits(-5, 5, -5, 5, ImPlotCond_Once);
                if (new_limits) {
                    ImPlot::SetupAxesLimits(xmin, xmax, ymin, ymax, ImPlotCond_Always);
                    
                    new_limits = false;
                } else {
                    ImPlotRect limits = ImPlot::GetPlotLimits();                
                    xmin = limits.Min().x; xmax = limits.Max().x;
                    ymin = limits.Min().y; ymax = limits.Max().y;
                }

//                ImPlot::SetupAxesLimits(0,10,-1.5,1.5);
                ImPlot::PlotLine("f(x)", xs1, ys1, 1001);
//                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
//                ImPlot::PlotLine("g(x)", xs2, ys2, 20, ImPlotLineFlags_Segments);

                ImPlot::EndPlot();
            }
            ImGui::End();

        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
