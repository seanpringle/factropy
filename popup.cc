#include "common.h"
#include "popup.h"
#include "sim.h"
#include "entity.h"
#include "energy.h"

#include "raylib.h"
#include "imgui/imgui.h"
#include "imgui/implot.h"

#include <vector>

Popup::Popup() {
}

Popup::~Popup() {
}

void Popup::center() {
	ImGui::SetWindowPos({
		((float)GetScreenWidth()-ImGui::GetWindowWidth())/2.0f,
		((float)GetScreenHeight()-ImGui::GetWindowHeight())/2.0f
	}, ImGuiCond_Always);
}

void Popup::draw() {
}

StatsPopup2::StatsPopup2() {
}

StatsPopup2::~StatsPopup2() {
}

void StatsPopup2::draw() {
	ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::SetWindowSize({1000.0f,600.0f}, ImGuiCond_Always);
	center();

	ImPlot::SetNextPlotLimits(0,60,0,1.0f);
	if (ImPlot::BeginPlot("Electricity")) {
		std::vector<float> y;
		for (uint i = 59; i >= 1; i--) {
			Energy e = (i*60) > Sim::tick ? 0: Sim::statsElectricityDemand.minutes[Sim::statsElectricityDemand.minute(Sim::tick-(i*60))];
			y.push_back(e.portion(Entity::electricityCapacity));
		}
		ImPlot::PlotLine("Demand", y.data(), y.size());
		ImPlot::EndPlot();
	}

	ImGui::End();
}