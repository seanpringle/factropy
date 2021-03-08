#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM "nop.h"
#include "glad.h"
#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_widgets.cpp"
#include "imgui_tables.cpp"
#include "imgui_impl_glfw.cpp"
#include "imgui_impl_opengl3.cpp"

#include "implot.cpp"
#include "implot_items.cpp"

#include <string>

namespace ImGui {
	void Print(std::string s) {
		TextUnformatted(s.c_str());
	}

	void PrintRight(std::string s) {
		ImVec2 size = CalcTextSize(s.c_str());
		ImVec2 space = GetContentRegionAvail();
		ImGui::SetCursorPosX(GetCursorPosX() + space.x - size.x - GetStyle().ItemSpacing.x);
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
		if (overflow) PushStyleColor(ImGuiCol_PlotHistogram, GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));
		ProgressBar(n, ImVec2(-1,0), "");
		if (overflow) PopStyleColor();
	}

	bool InputIntClamp(const char* label, int* v, int low, int high, int step, int step_fast) {
		bool f = InputInt(label, v, step, step_fast);
		if (f) *v = std::max(low, std::min(*v, high));
		return f;
	}

	void TrySameLine(const char* label, int margin) {
		SameLine();
		ImVec2 size = CalcTextSize(label);
		ImVec2 space = GetContentRegionAvail();
		if (space.x < size.x + margin) NewLine();
	}

	bool SmallButtonInline(const char* label) {
		TrySameLine(label, GetStyle().ItemSpacing.x + GetStyle().FramePadding.x*2);
		return SmallButton(label);
	}

	bool SmallButtonInlineRight(const char* label) {
		TrySameLine(label, GetStyle().ItemSpacing.x + GetStyle().FramePadding.x*2);

		ImVec2 size = CalcTextSize(label);
		ImVec2 space = GetContentRegionAvail();
		ImGui::SetCursorPosX(GetCursorPosX() + space.x - size.x - GetStyle().ItemSpacing.x - GetStyle().FramePadding.x*2);
		return SmallButton(label);
	}
}