#include "common.h"
#include "popup.h"
#include "sim.h"
#include "tech.h"
#include "entity.h"
#include "vehicle.h"
#include "energy.h"
#include "string.h"

#include "raylib-ex.h"
#include "../imgui/imgui.h"
#include "../imgui/implot.h"

#include <vector>

Popup::Popup(MainCamera* c) {
	camera = c;
	mouseOver = false;
	visible = false;
}

Popup::~Popup() {
}

void Popup::center(int w, int h) {

	const ImVec2 size = {
		(float)w,(float)h
	};

	const ImVec2 pos = {
		((float)GetScreenWidth()-size.x)/2.0f,
		((float)GetScreenHeight()-size.y)/2.0f
	};

	ImGui::SetNextWindowSize(size, ImGuiCond_Once);
	ImGui::SetNextWindowPos(pos, ImGuiCond_Once);

	Vector2 mouse = GetMousePosition();
	mouseOver = mouse.x >= pos.x && mouse.x < pos.x+size.x && mouse.y >= pos.y && mouse.y < pos.y+size.y;
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
	center(100,100);
	ImGui::Begin("##message", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove
	);

	ImGui::Print(text.c_str());

	ImGui::End();
}

StatsPopup2::StatsPopup2(MainCamera* c) : Popup(c) {
}

StatsPopup2::~StatsPopup2() {
}

void StatsPopup2::draw() {
	center(1500,1500);
	ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

	double tickMax = 1.0;
	std::list<Spec*> specs;
	for (auto& pair: Spec::all) {
		auto& spec = pair.second;
		if (!(spec->energyConsume || spec->energyDrain) || spec->statsGroup != spec) continue;
		tickMax = std::max(tickMax, spec->energyConsumption.tickMax);
		specs.push_back(spec);
	}

	std::map<Spec*,ImVec4> colors;

	auto limit = Energy(tickMax).magnitude();

	const double yticks[] = {0, limit};
	const char* ylabels[] = {"0", limit.formatRate().c_str()};

	ImPlot::SetNextPlotLimits(0,60,0,limit, ImGuiCond_Always);
	ImPlot::SetNextPlotTicksY(yticks, 2, ylabels);
	ImPlot::SetNextPlotTicksX(nullptr, 0, 0);
	if (ImPlot::BeginPlot("Energy Consumption", nullptr, nullptr, ImVec2(-1,400), ImPlotFlags_NoLegend | ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMousePos)) {
		for (auto& spec: specs) {
			std::vector<float> y;
			for (uint i = 59; i >= 1; i--) {
				Energy e = i > Sim::tick ? 0: spec->energyConsumption.ticks[spec->energyConsumption.tick(Sim::tick-i)];
				y.push_back(e);
			}
			ImPlot::PlotLine(fmtc("%s", spec->name), (const float*)y.data(), (int)y.size());
			colors[spec] = ImPlot::GetLastItemColor();
		}

		ImPlot::EndPlot();
	}

	specs.sort([&](const Spec* a, const Spec* b) {
		auto energyA = Energy(a->energyConsumption.ticks[a->energyConsumption.tick(Sim::tick-1)]);
		auto energyB = Energy(b->energyConsumption.ticks[b->energyConsumption.tick(Sim::tick-1)]);
		return energyA < energyB;
	});

	specs.reverse();

	for (auto& spec: specs) {
		auto energy = Energy(spec->energyConsumption.ticks[spec->energyConsumption.tick(Sim::tick-1)]);
		ImGui::Print(fmtc("%s %s", spec->name, energy.formatRate()));
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

		center(1000,600);
		ImGui::Begin("Waypoints", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

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
	center(1000,600);
	ImGui::Begin("Tech", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::SetWindowSize({1000.0f,600.0f}, ImGuiCond_Always);

	int n = 0;
	for (auto [name,tech]: Tech::names) {
		if (tech->bought) {
			ImGui::Print(fmtc("%s %s", name, tech->cost.format()));
		} else {
			if (ImGui::Button(fmtc("%s %s##%d", name, tech->cost.format(), n++))) {
				Sim::locked([tech=tech]() {
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

struct {
	char build[50];
} searches;

void BuildPopup2::draw() {
	center(1000,1000);
	inputFocused = false;
	using namespace ImGui;

	Begin("Build", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		PushID("build");

		InputText("search", searches.build, sizeof(searches.build));
		inputFocused = IsItemActive();

		BeginTable("specs", 3);

		TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 128);
		TableSetupColumn("spec", ImGuiTableColumnFlags_WidthFixed, 250);
		TableSetupColumn("materials", ImGuiTableColumnFlags_WidthStretch);
		TableHeadersRow();

		for (auto& [name,spec]: Spec::all) {
			if (!spec->build)
				continue;

			if (searches.build[0] && !std::strstr(name.c_str(), searches.build))
				continue;

			TableNextRow();

			TableNextColumn();
			if (ImageButton(spec->texture.texture.id, ImVec2(128, 128), ImVec2(0,0))) {
				camera->build(spec);
				show(false);
			}

			TableNextColumn();
			Print(fmtc("%s", spec->name));

			TableNextColumn();
			std::vector<std::string> materials;
			for (auto& stack: spec->materials) {
				materials.push_back(fmt("%s (%d)", Item::get(stack.iid)->name, (int)stack.size));
			}
			Print(fmtc("%s", concatenate(materials)));
		}

		EndTable();
		PopID();

	End();
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

		center(1000,1000);
		Begin(fmtc("%s###Entity", en.name()), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		PushID("entity");
			if (en.spec->enable) {
				bool enabled = en.isEnabled();
				if (Checkbox("Enabled", &enabled)) {
					en.setEnabled(enabled);
				}
			}
		PopID();

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
				if (Button(fmtc("d##%d", id++), ImVec2(-1,0))) {
					down = level.iid;
				}

				TableNextColumn();
				if (Button(fmtc("x##%d", id++), ImVec2(-1,0))) {
					clear = level.iid;
				}
			}

			for (Stack stack: store.stacks) {
				if (store.level(stack.iid) != nullptr) continue;
				Item *item = Item::get(stack.iid);

				int limit = (uint)store.limit().value/item->mass.value;
				int step  = (int)std::max(1U, (uint)limit/100);

				TableNextRow();

				TableNextColumn();
				Print(fmtc("%s", item->name));

				TableNextColumn();
				Print(fmtc("%d", store.count(stack.iid)));

				TableNextColumn();
				TableNextColumn();

				TableNextColumn();
				if (Button("+", ImVec2(-1,0))) {
					uint size = stack.size%step ? stack.size+step-(stack.size%step): stack.size;
					store.levelSet(stack.iid, 0, size);
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

			Print(fmtc("drones: %u, arms: %u", store.drones.size(), store.arms.size()));

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

		if (en.spec->ropeway) {
			PushID("ropeway");
			auto& ropeway = en.ropeway();

			Print(fmtc("ropeway %fm", ropeway.length()));

			PushID("ropeway-inputFilters");
				for (uint iid: ropeway.inputFilters) {
					Item* item = Item::get(iid);
					if (Button(item->name.c_str())) {
						ropeway.inputFilters.erase(iid);
					}
				}

				std::vector<const char*> inputOptions;

				for (auto& [name,item]: Item::names) {
					if (!ropeway.inputFilters.count(item->id)) {
						inputOptions.push_back(name.c_str());
					}
				}

				if (BeginCombo("input filters", nullptr)) {
					for (auto& s: inputOptions) {
						if (Selectable(s, false)) {
							ropeway.inputFilters.insert(Item::byName(s)->id);
						}
					}
					EndCombo();
				}
			PopID();

			PushID("ropeway-outputFilters");
				for (uint iid: ropeway.outputFilters) {
					Item* item = Item::get(iid);
					if (Button(item->name.c_str())) {
						ropeway.outputFilters.erase(iid);
					}
				}

				std::vector<const char*> outputOptions;

				for (auto& [name,item]: Item::names) {
					if (!ropeway.outputFilters.count(item->id)) {
						outputOptions.push_back(name.c_str());
					}
				}

				if (BeginCombo("output filters", nullptr)) {
					for (auto& s: outputOptions) {
						if (Selectable(s, false)) {
							ropeway.outputFilters.insert(Item::byName(s)->id);
						}
					}
					EndCombo();
				}
			PopID();

			PopID();
		}

		End();
	});
}