#include "pch.h"
#include "config_window.h"



struct rgba {
    float r, g, b, a;
    rgba(float ir, float ig, float ib, float ia) : r(ir), g(ig), b(ib), a(ia) {}
};

void config_window::initialize()
{
	auto& instance = get();

    //// Create a window called "My First Tool", with a menu bar.
    //ImGui::Begin("My First Tool", NULL, ImGuiWindowFlags_MenuBar);
    //if (ImGui::BeginMenuBar())
    //{
    //    if (ImGui::BeginMenu("testmenu"))
    //    {
    //        if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
    //        if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
    //        //if (ImGui::MenuItem("Close", "Ctrl+W")) { my_tool_active = false; }
    //        ImGui::EndMenu();
    //    }
    //    ImGui::EndMenuBar();
    //}

    //// Edit a color stored as 4 floats
    //rgba my_color = rgba(0.f, 0.f, 0.f, 0.f);
    //ImGui::ColorEdit4("Color", &my_color.r);

    //// Generate samples and plot them
    //float samples[100];
    //for (int n = 0; n < 100; n++)
    //    samples[n] = sinf(n * 0.2f + ImGui::GetTime() * 1.5f);
    //ImGui::PlotLines("Samples", samples, 100);

    //// Display contents in a scrolling region
    //ImGui::TextColored(ImVec4(1, 1, 0, 1), "Important Stuff");
    //ImGui::BeginChild("Scrolling");
    //for (int n = 0; n < 50; n++)
    //    ImGui::Text("%04d: Some text", n);
    //ImGui::EndChild();
    //ImGui::End();
}