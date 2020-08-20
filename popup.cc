#include "common.h"
#include "popup.h"
#include "sim.h"
#include "entity.h"
#include "vehicle.h"
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

WaypointsPopup::WaypointsPopup() {
	eid = 0;
}

WaypointsPopup::~WaypointsPopup() {
}

void WaypointsPopup::draw() {
	Sim::locked([&]() {
		if (!eid || !Entity::exists(eid)) {
			return;
		}

		Entity& en = Entity::get(eid);

		if (!en.spec->vehicle) {
			return;
		}

		Vehicle& vehicle = Vehicle::get(en.id);

		ImGui::Begin("Waypoints", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGui::SetWindowSize({1000.0f,600.0f}, ImGuiCond_Always);
		center();

		int n = 0;
		int thisWay = 0, dropWay = -1;

		for (auto waypoint: vehicle.waypoints) {
			if (waypoint->stopId) {
				ImGui::Print(waypoint->stopName.c_str());
			} else {
				ImGui::Print(fmt("%f,%f,%f", waypoint->position.x, waypoint->position.y, waypoint->position.z));
			}

			int thisCon = 0, dropCon = -1;
			for (auto condition: waypoint->conditions) {
				if_is<Vehicle::DepartInactivity>(condition, [&](Vehicle::DepartInactivity* con) {
					ImGui::Print(fmt("inactivity %d", con->seconds));
					ImGui::InputInt(fmtc("seconds##%d", n++), &con->seconds);
				});

				if_is<Vehicle::DepartItem>(condition, [&](Vehicle::DepartItem* con) {
					ImGui::Print(fmt("item %d,%d,%d", con->iid, con->op, con->count));
					if (ImGui::BeginCombo(fmtc("combo##%d", n++), (con->iid ? Item::get(con->iid)->name.c_str(): "item"), ImGuiComboFlags_None)) {
						for (auto [iid,item]: Item::ids) {
							if (ImGui::Selectable(fmtc("%s##%d", item->name, n++), con->iid == iid)) {
								con->iid = iid;
							}
						}
						ImGui::EndCombo();
					}
					if (ImGui::BeginCombo(fmtc("combo##%d", n++), fmtc("%u", con->op), ImGuiComboFlags_None)) {
						if (ImGui::Selectable(fmtc("==##%d", n++), con->op == Vehicle::DepartItem::Eq)) {
							con->op = Vehicle::DepartItem::Eq;
						}
						if (ImGui::Selectable(fmtc("!=##%d", n++), con->op == Vehicle::DepartItem::Ne)) {
							con->op = Vehicle::DepartItem::Ne;
						}
						if (ImGui::Selectable(fmtc("<##%d", n++), con->op == Vehicle::DepartItem::Lt)) {
							con->op = Vehicle::DepartItem::Lt;
						}
						if (ImGui::Selectable(fmtc("<=##%d", n++), con->op == Vehicle::DepartItem::Lte)) {
							con->op = Vehicle::DepartItem::Lte;
						}
						if (ImGui::Selectable(fmtc(">##%d", n++), con->op == Vehicle::DepartItem::Gt)) {
							con->op = Vehicle::DepartItem::Gt;
						}
						if (ImGui::Selectable(fmtc(">=##%d", n++), con->op == Vehicle::DepartItem::Gte)) {
							con->op = Vehicle::DepartItem::Gte;
						}
						ImGui::EndCombo();
					}
					ImGui::InputInt(fmtc("count##%d", n++), (int*)(&con->count));
				});

				if (ImGui::Button(fmtc("-con##%d", n++))) {
					dropCon = thisCon;
				}

				thisCon++;
			}

			if (dropCon >= 0) {
				auto it = waypoint->conditions.begin();
				std::advance(it, dropCon);
				delete *it;
				waypoint->conditions.erase(it);
			}

			if (ImGui::Button(fmtc("+activity##%d", n++))) {
				waypoint->conditions.push_back(new Vehicle::DepartInactivity());
			}

			ImGui::SameLine();

			if (ImGui::Button(fmtc("+item##%d", n++))) {
				waypoint->conditions.push_back(new Vehicle::DepartItem());
			}

			if (ImGui::Button(fmtc("-way##%d", n++))) {
				dropWay = thisWay;
			}

			thisWay++;
		}

		if (dropWay >= 0) {
			auto it = vehicle.waypoints.begin();
			std::advance(it, dropWay);
			delete *it;
			vehicle.waypoints.erase(it);
		}

		for (auto [eid,name]: Entity::names) {
			if (ImGui::Button(fmtc("+way %s##%d", name.c_str(), n++))) {
				vehicle.addWaypoint(eid);
			}
		}

		ImGui::End();
	});
}

void WaypointsPopup::useEntity(uint eeid) {
	eid = eeid;
}
