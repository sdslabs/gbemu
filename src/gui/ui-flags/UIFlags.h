#pragma once

#include "imgui.h"
#include "imgui/imgui/backends/imgui_impl_opengl3.h"
#include "imgui/imgui/backends/imgui_impl_sdlrenderer2.h"

namespace gbemuGUI
{
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	static ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	void ImGuiThemeSetup();
};
