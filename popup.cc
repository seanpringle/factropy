#include "common.h"
#include "popup.h"
#include "sim.h"
#include "tech.h"
#include "entity.h"
#include "vehicle.h"
#include "energy.h"

#include "raylib.h"
#include "imgui/imgui.h"
#include "imgui/implot.h"

#include <vector>

Popup::Popup(MainCamera* c) {
	camera = c;
}

Popup::~Popup() {
}

void Popup::center() {
	ImGui::SetWindowPos({
		((float)GetScreenWidth()-ImGui::GetWindowWidth())/2.0f,
		((float)GetScreenHeight()-ImGui::GetWindowHeight())/2.0f
	}, ImGuiCond_Always);
}

void Popup::show(bool state) {
	visible = state;
}

void Popup::draw() {
}

MessagePopup::MessagePopup(MainCamera* c) : Popup(c) {
}

MessagePopup::~MessagePopup() {
}

void MessagePopup::draw() {
	ImGui::Begin("##message", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove
	);
	center();

	ImGui::Print(text.c_str());

	ImGui::End();
}

StatsPopup2::StatsPopup2(MainCamera* c) : Popup(c) {
}

StatsPopup2::~StatsPopup2() {
}

void StatsPopup2::draw() {
	ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::SetWindowSize({1000.0f,600.0f}, ImGuiCond_Always);
	center();

	ImPlot::SetNextPlotLimits(0,60,0,1.0f);
	if (ImPlot::BeginPlot("Electricity")) {
		std::vector<float> y;
		for (uint i = 59; i >= 1; i--) {
			Energy e = (i*60) > Sim::tick ? 0: Sim::statsElectricityDemand.minutes[Sim::statsElectricityDemand.minute(Sim::tick-(i*60))];
			y.push_back(e.portion(Entity::electricityCapacity));
		}
		ImPlot::PlotLine("Demand", (const float*)y.data(), (int)y.size());
		ImPlot::EndPlot();
	}

	ImGui::End();
}

WaypointsPopup::WaypointsPopup(MainCamera* c) : Popup(c) {
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

		ImGui::Begin("Waypoints", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
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
				if_is<Vehicle::DepartInactivity>(condition, [&](auto con) {
					ImGui::Print(fmt("inactivity %d", con->seconds));
					ImGui::InputInt(fmtc("seconds##%d", n++), &con->seconds);
				});

				if_is<Vehicle::DepartItem>(condition, [&](auto con) {
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

TechPopup::TechPopup(MainCamera* c) : Popup(c) {
}

TechPopup::~TechPopup() {
}

void TechPopup::draw() {
	ImGui::Begin("Tech", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::SetWindowSize({1000.0f,600.0f}, ImGuiCond_Always);
	center();

	int n = 0;
	for (auto [name,tech]: Tech::names) {
		if (tech->bought) {
			ImGui::Print(fmtc("%s %s", name, tech->cost.format()));
		} else {
			if (ImGui::Button(fmtc("%s %s##%d", name, tech->cost.format(), n++))) {
				Sim::locked([&]() {
					tech->buy();
				});
			}
		}
	}

	ImGui::End();
}

BuildPopup2::BuildPopup2(MainCamera* c) : Popup(c) {
}

BuildPopup2::~BuildPopup2() {
}

void BuildPopup2::draw() {
	ImGui::Begin("Build", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::SetWindowSize({1000.0f,1000.0f}, ImGuiCond_Always);
	center();

	int n = 0;
	for (auto pair: Spec::all) {
		Spec* spec = pair.second;

		if (n%6) ImGui::SameLine();
		if (ImGui::ImageButton(spec->texture.texture.id,
			ImVec2(128, 128),
			ImVec2(0,0)
		)){
			camera->build(spec);
			show(false);
		}

		n++;
	}

	ImGui::End();
}


EntityPopup2::EntityPopup2(MainCamera* c) : Popup(c) {
}

EntityPopup2::~EntityPopup2() {
}

void EntityPopup2::useEntity(uint eeid) {
	eid = eeid;
}

void EntityPopup2::draw() {
	using namespace ImGui;

	Sim::locked([&]() {
		if (!eid || !Entity::exists(eid) || Entity::get(eid).isGhost()) {
			eid = 0;
			show(false);
			return;
		}

		Entity &en = Entity::get(eid);

		Begin(fmtc("%s###Entity", en.name()), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		SetWindowSize({1000.0f,1000.0f}, ImGuiCond_Always);
		center();

		if (en.spec->named) {
			char name[50]; std::snprintf(name, sizeof(name), "%s", en.name().c_str());
			if (InputText("Name", name, sizeof(name))) {
				en.rename(name);
			}
		}

		if (en.spec->consumeChemical) {
			Burner& burner = en.burner();
			Print("Fuel");
			LevelBar(burner.energy.portion(burner.buffer));
			for (Stack stack: burner.store.stacks) {
				Item* item = Item::get(stack.iid);
				Print(fmtc("%s(%u)", item->name, stack.size));
			}
		}

		if (en.spec->crafter) {
			Crafter& crafter = en.crafter();
			PushID("crafter");
			LevelBar(crafter.progress);

			const char* selected = nullptr;
			std::vector<const char*> options;

			for (auto& [name,recipe]: Recipe::names) {
				bool show = true;
				if (en.spec->recipeTags.size()) {
					show = false;
					for (auto tag: en.spec->recipeTags) {
						show = show || recipe->tags.count(tag);
					}
				}
				if (show) {
					options.push_back(name.c_str());
					if (crafter.recipe == recipe) {
						selected = name.c_str();
					}
				}
			}

			if (BeginCombo("recipe", selected)) {
				for (auto& s: options) {
					if (Selectable(s, s == selected)) {
						crafter.nextRecipe = Recipe::byName(s);
					}
				}
				EndCombo();
			}

			PopID();
		}

		if (en.spec->arm) {
			Arm& arm = en.arm();
			PushID("arm");
			LevelBar(arm.orientation);

			for (uint iid: arm.filter) {
				Item* item = Item::get(iid);
				if (Button(item->name.c_str())) {
					arm.filter.erase(iid);
				}
			}

			std::vector<const char*> options;

			for (auto& [name,item]: Item::names) {
				if (!arm.filter.count(item->id)) {
					options.push_back(name.c_str());
				}
			}

			if (BeginCombo("filter", nullptr)) {
				for (auto& s: options) {
					if (Selectable(s, false)) {
						arm.filter.insert(Item::byName(s)->id);
					}
				}
				EndCombo();
			}

			PopID();
		}

		if (en.spec->lift) {
			Lift& lift = en.lift();
			PushID("lift");

			const char* options[] = {"raise", "lower"};
			int selected = lift.mode == Lift::Raise ? 0: 1;

			if (BeginCombo("mode", options[selected])) {
				for (int i = 0; i < 2; i++) {
					Selectable(options[i], selected == i);
				}
				EndCombo();
			}

			Print(fmtc("%s",
				lift.stage == Lift::Lowered ? "lowered":
					lift.stage == Lift::Lowering ? "lowering":
						lift.stage == Lift::Raised ? "raised":
							lift.stage == Lift::Raising ? "raising": "wtf")
			);

			PopID();
		}

		if (en.spec->store) {
			Store& store = en.store();
			PushID("store");

			int id = 0;

			uint clear = 0;
			uint down = 0;

			Mass usage = store.usage();
			Mass limit = store.limit();

			Print("Storage");
			LevelBar(usage.portion(limit));

			BeginTable("levels", 7);

			TableSetupColumn("item", ImGuiTableColumnFlags_WidthFixed, 250);
			TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 100);
			TableSetupColumn(fmtc("##%d", id++), ImGuiTableColumnFlags_WidthFixed, 50);
			TableSetupColumn("lower", ImGuiTableColumnFlags_WidthStretch);
			TableSetupColumn("upper", ImGuiTableColumnFlags_WidthStretch);
			TableSetupColumn(fmtc("##%d", id++), ImGuiTableColumnFlags_WidthFixed, 50);
			TableSetupColumn(fmtc("##%d", id++), ImGuiTableColumnFlags_WidthFixed, 50);
			TableHeadersRow();

			for (Store::Level level: store.levels) {
				Item *item = Item::get(level.iid);
				TableNextRow();

				int limit = (int)std::max(level.upper, (uint)store.limit().value/item->mass.value);
				int step  = (int)std::max(1U, (uint)limit/100);

				TableNextColumn();
				Print(fmtc("%s", item->name));

				TableNextColumn();
				Print(fmtc("%d", store.count(level.iid)));

				TableNextColumn();
				if (store.isRequesting(level.iid)) {
					Print("r");
				} else
				if (store.isActiveProviding(level.iid)) {
					Print("a");
				} else
				if (store.isProviding(level.iid)) {
					Print("p");
				}

				int lower = level.lower;
				int upper = level.upper;

				TableNextColumn();
				if (en.spec->enableSetLower) {
					SetNextItemWidth(-0.001f);
					if (InputIntClamp(fmtc("##%d", id++), &lower, 0, limit, step, step*10)) {
						store.levelSet(level.iid, lower, upper);
					}
				}

				TableNextColumn();
				if (en.spec->enableSetUpper) {
					SetNextItemWidth(-0.001f);
					if (InputIntClamp(fmtc("##%d", id++), &upper, 0, limit, step, step*10)) {
						store.levelSet(level.iid, lower, upper);
					}
				}

				TableNextColumn();
				if (Button("d", ImVec2(-1,0))) {
					down = level.iid;
				}

				TableNextColumn();
				if (Button("x", ImVec2(-1,0))) {
					clear = level.iid;
				}
			}

			for (Stack stack: store.stacks) {
				if (store.level(stack.iid) != nullptr) continue;
				Item *item = Item::get(stack.iid);
				TableNextRow();

				TableNextColumn();
				Print(fmtc("%s", item->name));

				TableNextColumn();
				Print(fmtc("%d", store.count(stack.iid)));

				TableNextColumn();
				TableNextColumn();

				TableNextColumn();
				if (Button("+", ImVec2(-1,0))) {
					store.levelSet(stack.iid, 0, 0);
				}

				TableNextColumn();
				TableNextColumn();
			}

			EndTable();

			std::vector<const char*> options;

			for (auto& [name,item]: Item::names) {
				if (!store.level(item->id) && !store.count(item->id)) {
					options.push_back(name.c_str());
				}
			}

			SetNextItemWidth(200);
			if (BeginCombo(fmtc("##%d", id++), nullptr)) {
				for (auto& s: options) {
					if (Selectable(s, false)) {
						store.levelSet(Item::byName(s)->id, 0, 0);
					}
				}
				EndCombo();
			}

			if (clear) {
				store.levelClear(clear);
			}

			if (down) {
				auto level = store.level(down);
				uint lower = level->lower;
				uint upper = level->upper;
				store.levelClear(down);
				store.levelSet(down, lower, upper);
			}

			PopID();
		}

		if (en.spec->vehicle) {
			PushID("vehicle");

			bool patrol = en.vehicle().patrol;
			if (Checkbox("patrol", &patrol)) {
				en.vehicle().patrol = patrol;
			}

			bool handbrake = en.vehicle().handbrake;
			if (Checkbox("handbrake", &handbrake)) {
				en.vehicle().handbrake = handbrake;
			}

			PopID();
		}

		End();
	});
}