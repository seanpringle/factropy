#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM "nop.h"
#include "glad.h"
#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_widgets.cpp"
#include "imgui_impl_glfw.cpp"
#include "imgui_impl_opengl3.cpp"

#include "implot.cpp"
//#include "implot_items.cpp"

#include <string>

namespace ImGui {
	void Print(std::string s) {
		TextUnformatted(s.c_str());
	}

	void PrintRight(std::string s) {
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(s.c_str()).x + ImGui::GetStyle().ItemSpacing.x);
		TextUnformatted(s.c_str());
	}

	void LevelBar(float n, std::string s) {
		ProgressBar(n, ImVec2(-1,0), s.c_str());
	}

	void LevelBar(float n) {
		LevelBar(n, "");
	}

	void OverflowBar(float n) {
		bool overflow = n > 0.999f;
		if (overflow) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));
		ProgressBar(n, ImVec2(-1,0), "");
		if (overflow) ImGui::PopStyleColor();
	}
}