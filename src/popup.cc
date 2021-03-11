#include "common.h"
#include "popup.h"
#include "sim.h"
#include "tech.h"
#include "entity.h"
#include "vehicle.h"
#include "energy.h"
#include "string.h"
#include "ledger.h"
#include "view.h"

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

	ImGui::SetNextWindowSize(size, ImGuiCond_Always);
	ImGui::SetNextWindowPos(pos, ImGuiCond_Always);

	//Vector2 mouse = GetMousePosition();
	//mouseOver = mouse.x >= pos.x && mouse.x < pos.x+size.x && mouse.y >= pos.y && mouse.y < pos.y+size.y;
}

void Popup::bottomLeft(int w, int h) {

	const ImVec2 size = {
		(float)w,(float)h
	};

	const ImVec2 pos = {
		0.0f,
		(float)GetScreenHeight()-size.y,
	};

	ImGui::SetNextWindowSize(size, ImGuiCond_Always);
	ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
}

void Popup::show(bool state) {
	opened = state && !visible;
	visible = state;
}

void Popup::draw() {
}

MessagePopup::MessagePopup(MainCamera* c) : Popup(c) {
}

MessagePopup::~MessagePopup() {
}

void MessagePopup::draw() {
	bottomLeft(0,100);
	ImGui::Begin("##message", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove
	);

	ImGui::Print(text.c_str());

	mouseOver = ImGui::IsWindowHovered();
	ImGui::End();
}

StatsPopup2::StatsPopup2(MainCamera* c) : Popup(c) {
}

StatsPopup2::~StatsPopup2() {
}

void StatsPopup2::draw() {
	using namespace ImGui;

	std::hash<std::string> hash;

	center(1500,1500);
	Begin("Stats", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

	if (BeginTabBar("stats-tabs", ImGuiTabBarFlags_None)) {

		if (BeginTabItem("Energy")) {

			Sim::locked([&]() {
				double tickMax = 1.0;
				std::list<Spec*> specs;
				for (auto& pair: Spec::all) {
					auto& spec = pair.second;
					if (!spec->consumeElectricity) continue;
					if (!(spec->energyConsume || spec->energyDrain)) continue;
					if (spec->statsGroup != spec) continue;
					tickMax = std::max(tickMax, spec->energyConsumption.tickMax);
					specs.push_back(spec);
				}

				specs.sort([&](const Spec* a, const Spec* b) {
					auto energyA = Energy(a->energyConsumption.ticks[a->energyConsumption.tick(Sim::tick-1)]);
					auto energyB = Energy(b->energyConsumption.ticks[b->energyConsumption.tick(Sim::tick-1)]);
					return energyA < energyB;
				});

				specs.reverse();

				auto limit = Energy(tickMax).magnitude();

				const double yticks[] = {0, limit};
				const char* ylabels[] = {"0", limit.formatRate().c_str()};

				ImPlot::SetNextPlotLimits(0,60,0,limit, ImGuiCond_Always);
				ImPlot::SetNextPlotTicksY(yticks, 2, ylabels);
				ImPlot::SetNextPlotTicksX(nullptr, 0, 0);
				if (ImPlot::BeginPlot("Electricity Consumption", nullptr, nullptr, ImVec2(-1,700), ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMousePos)) {
					for (auto& spec: specs) {
						std::vector<float> y;
						for (uint i = 59; i >= 1; i--) {
							Energy e = i > Sim::tick ? 0: spec->energyConsumption.ticks[spec->energyConsumption.tick(Sim::tick-i)];
							y.push_back(e);
						}

						float r = 1.0f - ((float)(hash(fmt("R%sR", spec->name)) % 61) / 100.0f * 1.25f);
						float g = 1.0f - ((float)(hash(fmt("G%sG", spec->name)) % 61) / 100.0f * 1.25f);
						float b = 1.0f - ((float)(hash(fmt("B%sB", spec->name)) % 61) / 100.0f * 1.25f);
						ImPlot::SetNextLineStyle(ImVec4(r,g,b,1.0f));

						auto energy = Energy(spec->energyConsumption.ticks[spec->energyConsumption.tick(Sim::tick-1)]);
						ImPlot::PlotLine(fmtc("%s %s", spec->name, energy.formatRate()), (const float*)y.data(), (int)y.size());
					}

					ImPlot::EndPlot();
				}
			});

			EndTabItem();
		}

		if (BeginTabItem("Debug")) {

			Sim::locked([&]() {
				{
					double tickMax = 0.0f;
					tickMax = std::max(tickMax, Sim::statsEntity.tickMax);
					tickMax = std::max(tickMax, Sim::statsGhost.tickMax);
					tickMax = std::max(tickMax, Sim::statsStore.tickMax);
					tickMax = std::max(tickMax, Sim::statsArm.tickMax);
					tickMax = std::max(tickMax, Sim::statsCrafter.tickMax);
					tickMax = std::max(tickMax, Sim::statsConveyor.tickMax);
					tickMax = std::max(tickMax, Sim::statsUnveyor.tickMax);
					tickMax = std::max(tickMax, Sim::statsLoader.tickMax);
					tickMax = std::max(tickMax, Sim::statsRopeway.tickMax);
					tickMax = std::max(tickMax, Sim::statsRopewayBucket.tickMax);
					tickMax = std::max(tickMax, Sim::statsProjector.tickMax);
					tickMax = std::max(tickMax, Sim::statsPath.tickMax);
					tickMax = std::max(tickMax, Sim::statsVehicle.tickMax);
					tickMax = std::max(tickMax, Sim::statsPipe.tickMax);
					tickMax = std::max(tickMax, Sim::statsShunt.tickMax);
					tickMax = std::max(tickMax, Sim::statsDepot.tickMax);
					tickMax = std::max(tickMax, Sim::statsDrone.tickMax);
					tickMax = std::max(tickMax, Sim::statsMissile.tickMax);
					tickMax = std::max(tickMax, Sim::statsExplosion.tickMax);
					tickMax = std::max(tickMax, Sim::statsTurret.tickMax);
					tickMax = std::max(tickMax, Sim::statsComputer.tickMax);

					tickMax = std::max(1.0, std::ceil(tickMax));

					auto ylabel = fmt("%dms", (int)tickMax);

					const double yticks[] = {0, tickMax};
					const char* ylabels[] = {"0", ylabel.c_str()};

					ImPlot::SetNextPlotLimits(0,60,0,tickMax, ImGuiCond_Always);
					ImPlot::SetNextPlotTicksY(yticks, 2, ylabels);
					ImPlot::SetNextPlotTicksX(nullptr, 0, 0);

					if (ImPlot::BeginPlot("Simulation", nullptr, nullptr, ImVec2(-1,700), ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMousePos)) {

						auto plot = [&](std::string name, TimeSeries* series) {

							float r = 1.0f - ((float)(hash(fmt("R%sR", name)) % 61) / 100.0f * 1.25f);
							float g = 1.0f - ((float)(hash(fmt("G%sG", name)) % 61) / 100.0f * 1.25f);
							float b = 1.0f - ((float)(hash(fmt("B%sB", name)) % 61) / 100.0f * 1.25f);
							ImPlot::SetNextLineStyle(ImVec4(r,g,b,1.0f));

							std::vector<float> y;
							for (uint i = 59; i >= 1; i--) {
								float ms = i > Sim::tick ? 0: series->ticks[series->tick(Sim::tick-i)];
								y.push_back(ms);
							}

							ImPlot::PlotLine(fmtc("%0.2f %s", series->tickMax, name), (const float*)y.data(), (int)y.size());
						};

						plot("Entity", &Sim::statsEntity);
						plot("Ghost", &Sim::statsGhost);
						plot("Store", &Sim::statsStore);
						plot("Arm", &Sim::statsArm);
						plot("Crafter", &Sim::statsCrafter);
						plot("Conveyor", &Sim::statsConveyor);
						plot("Unveyor", &Sim::statsUnveyor);
						plot("Loader", &Sim::statsLoader);
						plot("Ropeway", &Sim::statsRopeway);
						plot("RopewayBucket", &Sim::statsRopewayBucket);
						plot("Projector", &Sim::statsProjector);
						plot("Path", &Sim::statsPath);
						plot("Vehicle", &Sim::statsVehicle);
						plot("Pipe", &Sim::statsPipe);
						plot("Shunt", &Sim::statsShunt);
						plot("Depot", &Sim::statsDepot);
						plot("Drone", &Sim::statsDrone);
						plot("Missile", &Sim::statsMissile);
						plot("Explosion", &Sim::statsExplosion);
						plot("Turret", &Sim::statsTurret);
						plot("Computer", &Sim::statsComputer);

						ImPlot::EndPlot();
					}
				}

				{
					double tickMax = 16.0f;
					//tickMax = std::max(tickMax, camera->statsUpdate.tickMax);
					//tickMax = std::max(tickMax, camera->statsDraw.tickMax);
					//tickMax = std::max(tickMax, camera->statsFrame.tickMax);
					//tickMax = std::max(1.0, std::ceil(tickMax));

					auto ylabel = fmt("%dms", (int)tickMax);

					const double yticks[] = {0, tickMax};
					const char* ylabels[] = {"0", ylabel.c_str()};

					ImPlot::SetNextPlotLimits(0,60,0,tickMax, ImGuiCond_Always);
					ImPlot::SetNextPlotTicksY(yticks, 2, ylabels);
					ImPlot::SetNextPlotTicksX(nullptr, 0, 0);

					if (ImPlot::BeginPlot("Renderer", nullptr, nullptr, ImVec2(-1,700), ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMousePos)) {

						std::hash<std::string> hash;

						auto plot = [&](std::string name, TimeSeries* series) {

							float r = 1.0f - ((float)(hash(fmt("R%sR", name)) % 61) / 100.0f * 1.25f);
							float g = 1.0f - ((float)(hash(fmt("G%sG", name)) % 61) / 100.0f * 1.25f);
							float b = 1.0f - ((float)(hash(fmt("B%sB", name)) % 61) / 100.0f * 1.25f);
							ImPlot::SetNextLineStyle(ImVec4(r,g,b,1.0f));

							std::vector<float> y;
							for (uint i = 59; i >= 1; i--) {
								float ms = i > Sim::tick ? 0: series->ticks[series->tick(camera->frame-i)];
								y.push_back(ms);
							}

							ImPlot::PlotLine(fmtc("%0.2f %s", series->tickMax, name), (const float*)y.data(), (int)y.size());
						};

						plot("Frame", &camera->statsFrame);
						plot("Update", &camera->statsUpdate);
						plot("Draw", &camera->statsDraw);

						ImPlot::EndPlot();
					}
				}

				EndTabItem();
			});
		}

		EndTabBar();
	}

	mouseOver = ImGui::IsWindowHovered();
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

		if (opened && en.spec->named) {
			std::snprintf(name, sizeof(name), "%s", en.name().c_str());
		}

		auto focusedTab = [&]() {
			bool focused = opened;
			opened = false;
			return (focused ? ImGuiTabItemFlags_SetSelected: ImGuiTabItemFlags_None) | ImGuiTabItemFlags_NoPushId;
		};

		center(1000,1000);
		Begin(fmtc("%s###entity", en.name()), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		if (BeginTabBar("entity-tabs", ImGuiTabBarFlags_None)) {

			if (en.spec->arm && BeginTabItem("Arm", nullptr, focusedTab())) {
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

				EndTabItem();
			}

			if (en.spec->ropeway && BeginTabItem("Ropeway", nullptr, focusedTab())) {
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

				EndTabItem();
			}

			bool operableStore = en.spec->store && (en.spec->storeSetLower || en.spec->storeSetUpper);

			if (operableStore && BeginTabItem("Storage", nullptr, focusedTab())) {
				Store& store = en.store();

				PushID("store-manual");

					int id = 0;

					uint clear = 0;
					uint down = 0;

					Mass usage = store.usage();
					Mass limit = store.limit();

					Print("Storage");
					LevelBar(usage.portion(limit));

					BeginTable("levels", en.spec->crafter ? 8: 7);

					TableSetupColumn("item", ImGuiTableColumnFlags_WidthFixed, 250);
					TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 100);
					TableSetupColumn(fmtc("##%d", id++), ImGuiTableColumnFlags_WidthFixed, 50);
					TableSetupColumn("lower", ImGuiTableColumnFlags_WidthStretch);
					TableSetupColumn("upper", ImGuiTableColumnFlags_WidthStretch);
					TableSetupColumn(fmtc("##%d", id++), ImGuiTableColumnFlags_WidthFixed, 50);
					TableSetupColumn(fmtc("##%d", id++), ImGuiTableColumnFlags_WidthFixed, 50);

					if (en.spec->crafter) {
						TableSetupColumn(fmtc("craft##%d", id++), ImGuiTableColumnFlags_WidthFixed, 50);
					}

					TableHeadersRow();

					for (auto& level: store.levels) {
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
						if (en.spec->storeSetLower) {
							SetNextItemWidth(-0.001f);
							if (InputIntClamp(fmtc("##%d", id++), &lower, 0, limit, step, step*10)) {
								store.levelSet(level.iid, lower, upper);
							}
						}

						TableNextColumn();
						if (en.spec->storeSetUpper) {
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

						if (en.spec->crafter) {
							TableNextColumn();
							Checkbox(fmtc("##%d", id++), &level.craft);
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

						if (en.spec->crafter) {
							TableNextColumn();
						}
					}

					EndTable();

					std::vector<const char*> options;

					for (auto& [name,item]: Item::names) {
						if (!store.level(item->id) && !store.count(item->id)) {
							options.push_back(name.c_str());
						}
					}

					SetNextItemWidth(200);
					if (BeginCombo(fmtc("add item##%d", id++), nullptr)) {
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

				EndTabItem();
			}

			if (en.spec->crafter && en.spec->crafterShowTab && BeginTabItem("Crafting", nullptr, focusedTab())) {
				Crafter& crafter = en.crafter();

				PushID("crafter");

				const char* selected = nullptr;
				std::vector<const char*> options;

				for (auto& [name,recipe]: Recipe::names) {
					if (recipe->licensed && crafter.craftable(recipe)) {
						options.push_back(name.c_str());
						if (crafter.recipe == recipe) {
							selected = name.c_str();
						}
					}
				}

				if (BeginCombo("recipe", selected)) {
					for (auto& s: options) {
						if (Selectable(s, s == selected)) {
							crafter.craft(Recipe::byName(s));
						}
					}
					EndCombo();
				}

				LevelBar(crafter.progress);

				if (en.spec->store && !en.spec->storeSetLower && !en.spec->storeSetUpper) {
					Store& store = en.store();
					PushID("store-view");

					Mass usage = store.usage();
					Mass limit = store.limit();

					Print("Storage");
					LevelBar(usage.portion(limit));

					for (auto level: store.levels) {
						Print(fmtc("%s(%u)", Item::get(level.iid)->name, store.count(level.iid)));
					}

					PopID();
				}

				PopID();

				EndTabItem();
			}

			if (en.spec->vehicle && BeginTabItem("Vehicle", nullptr, focusedTab())) {
				Vehicle& vehicle = en.vehicle();

				PushID("vehicle");

				bool patrol = en.vehicle().patrol;
				if (Checkbox("patrol", &patrol)) {
					vehicle.patrol = patrol;
				}

				bool handbrake = vehicle.handbrake;
				if (Checkbox("handbrake", &handbrake)) {
					vehicle.handbrake = handbrake;
				}

				int n = 0;
				int thisWay = 0, dropWay = -1;

				for (auto waypoint: vehicle.waypoints) {
					if (waypoint->stopId && Entity::exists(waypoint->stopId)) {
						Print(Entity::get(waypoint->stopId).name().c_str());
					} else {
						Print(fmt("%f,%f,%f", waypoint->position.x, waypoint->position.y, waypoint->position.z));
					}

					if (waypoint == vehicle.waypoint) {
						SameLine();
						Print("(current)");
					}

					int thisCon = 0, dropCon = -1;
					for (auto condition: waypoint->conditions) {

						if_is<Vehicle::DepartInactivity>(condition, [&](auto con) {
							Print(fmt("inactivity %d", con->seconds));
							InputInt(fmtc("seconds##%d", n++), &con->seconds);
						});

						if_is<Vehicle::DepartItem>(condition, [&](auto con) {

							SetNextItemWidth(200);
							if (BeginCombo(fmtc("##%d", n++), (con->iid ? Item::get(con->iid)->name.c_str(): "item"), ImGuiComboFlags_None)) {
								for (auto [iid,item]: Item::ids) {
									if (Selectable(fmtc("%s##%d", item->name, n++), con->iid == iid)) {
										con->iid = iid;
									}
								}
								EndCombo();
							}

							std::map<uint,std::string> ops = {
								{ Vehicle::DepartItem::Eq,  "==" },
								{ Vehicle::DepartItem::Ne,  "!=" },
								{ Vehicle::DepartItem::Lt,  "<" },
								{ Vehicle::DepartItem::Lte, "<=" },
								{ Vehicle::DepartItem::Gt,  ">" },
								{ Vehicle::DepartItem::Gte, ">=" },
							};

							SameLine();
							SetNextItemWidth(100);
							if (BeginCombo(fmtc("##%d", n++), ops[con->op].c_str(), ImGuiComboFlags_None)) {
								for (auto [op,opname]: ops) {
									if (Selectable(fmtc("%s##%d-%u", opname.c_str(), n++, op), con->op == op)) {
										con->op = op;
									}
								}
								EndCombo();
							}

							SameLine();
							SetNextItemWidth(200);
							InputInt(fmtc("##%d", n++), (int*)(&con->count));
						});

						SameLine();
						if (Button(fmtc("-con##%d", n++))) {
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

					if (Button(fmtc("+activity##%d", n++))) {
						waypoint->conditions.push_back(new Vehicle::DepartInactivity());
					}

					SameLine();

					if (Button(fmtc("+item##%d", n++))) {
						waypoint->conditions.push_back(new Vehicle::DepartItem());
					}

					if (Button(fmtc("-way##%d", n++))) {
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

				PopID();

				EndTabItem();
			}

			if (BeginTabItem("Settings", nullptr, focusedTab())) {

				PushID("entity");
					if (en.spec->enable) {
						bool enabled = en.isEnabled();
						if (Checkbox("Enabled", &enabled)) {
							en.setEnabled(enabled);
						}
					}
					if (en.spec->generateElectricity) {
						bool generating = en.isGenerating();
						if (Checkbox("Generate Electricity", &generating)) {
							en.setGenerating(generating);
						}
					}
					if (en.spec->named) {
						bool saveEnter = InputText("Name", name, sizeof(name), ImGuiInputTextFlags_EnterReturnsTrue);
						inputFocused = IsItemActive();
						SameLine();
						if (Button("Save") || saveEnter) {
							if (std::strlen(name)) {
								en.rename(name);
							}
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
				PopID();

				EndTabItem();
			}

			EndTabBar();
		}

		mouseOver = IsWindowHovered();
		End();
	});
}

RecipePopup::RecipePopup(MainCamera* c) : Popup(c) {
	for (auto& [_,item]: Item::names)
		sorted.items.push_back(item);
	for (auto& [_,fluid]: Fluid::names)
		sorted.fluids.push_back(fluid);
	for (auto& [_,recipe]: Recipe::names)
		sorted.recipes.push_back(recipe);
	for (auto& [_,spec]: Spec::all)
		if (spec->build) sorted.specs.push_back(spec);
	for (auto& [_,tech]: Tech::names)
		sorted.techs.push_back(tech);

	std::sort(sorted.items.begin(),   sorted.items.end(),   [&](auto a, auto b) { return a->name < b->name; });
	std::sort(sorted.fluids.begin(),  sorted.fluids.end(),  [&](auto a, auto b) { return a->name < b->name; });
	std::sort(sorted.recipes.begin(), sorted.recipes.end(), [&](auto a, auto b) { return a->name < b->name; });
	std::sort(sorted.specs.begin(),   sorted.specs.end(),   [&](auto a, auto b) { return a->name < b->name; });
}

RecipePopup::~RecipePopup() {
}

void RecipePopup::draw() {
	using namespace ImGui;

	Sim::locked([&]() {
		center(1500,1000);
		Begin("recipes##recipe", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		Checkbox("show unavailable stuff", &showUnavailable);

		BeginTable("results", 4);

		TableSetupColumn("items & fluids");
		TableSetupColumn("recipes");
		TableSetupColumn("structures & vehicles");
		TableSetupColumn("technologies");
		TableHeadersRow();

		TableNextColumn();
		BeginChild("results-items-and-fluids");
		for (auto& item: sorted.items) drawItem(item);
		for (auto& fluid: sorted.fluids) drawFluid(fluid);
		EndChild();

		TableNextColumn();
		BeginChild("results-recipes");
		for (auto& recipe: sorted.recipes) drawRecipe(recipe);
		EndChild();

		TableNextColumn();
		BeginChild("results-specs");
		for (auto& spec: sorted.specs) drawSpec(spec);
		EndChild();

		TableNextColumn();
		BeginChild("results-techs");
		for (auto& tech: sorted.techs) drawTech(tech);
		EndChild();

		EndTable();

		mouseOver = IsWindowHovered();
		End();
	});
}

void RecipePopup::drawItem(Item* item) {
	using namespace ImGui;

	if (!showUnavailable && !item->manufacturable()) return;

	if (locate.item == item) {
		SetScrollHereY();
		locate.item = nullptr;
	}

	int pop = 0;

	if (highlited.item[item]) {
		PushStyleColor(ImGuiCol_Header, GetStyleColorVec4(ImGuiCol_HeaderHovered));
		pop++;
	}

	if (!item->manufacturable()) {
		PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled));
		pop++;
	}

	SetNextItemOpen(expanded.item[item]);
	expanded.item[item] = CollapsingHeader(fmtc("%s##item-%d", item->name, item->id));

	if (expanded.item[item]) {

		Print("produced by");
		miniset<Recipe*> producers;
		for (auto& [_,recipe]: Recipe::names) {
			if (recipe->mine == item->id) {
				producers.insert(recipe);
			}
			if (recipe->outputItems.count(item->id)) {
				producers.insert(recipe);
			}
		}
		for (auto& recipe: producers) {
			drawRecipeButton(recipe);
		}

		Print("consumed by");
		miniset<Recipe*> recipeConsumers;
		for (auto& [_,recipe]: Recipe::names) {
			if (recipe->inputItems.count(item->id)) {
				recipeConsumers.insert(recipe);
			}
		}
		for (auto& recipe: recipeConsumers) {
			drawRecipeButton(recipe);
		}
		miniset<Spec*> specConsumers;
		for (auto& [_,spec]: Spec::all) {
			for (auto [iid,_]: spec->materials) {
				if (iid == item->id) {
					specConsumers.insert(spec);
				}
			}
		}
		for (auto& spec: specConsumers) {
			drawSpecButton(spec);
		}
	}

	if (pop) {
		PopStyleColor(pop);
	}

	highlited.item[item] = false;
}

void RecipePopup::drawItemButton(Item* item, int count) {
	using namespace ImGui;
	if (SmallButtonInline(fmtc("%s(%d)", item->name, count))) {
		locate.item = item;
		expanded.item[item] = !expanded.item[item];
	}
	highlited.item[item] = highlited.item[item] || IsItemHovered();
}

void RecipePopup::drawFluid(Fluid* fluid) {
	using namespace ImGui;

	if (!showUnavailable && !fluid->manufacturable()) return;

	if (locate.fluid == fluid) {
		SetScrollHereY();
		locate.fluid = nullptr;
	}

	int pop = 0;

	if (highlited.fluid[fluid]) {
		PushStyleColor(ImGuiCol_Header, GetStyleColorVec4(ImGuiCol_HeaderHovered));
		pop++;
	}

	if (!fluid->manufacturable()) {
		PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled));
		pop++;
	}

	SetNextItemOpen(expanded.fluid[fluid]);
	expanded.fluid[fluid] = CollapsingHeader(fmtc("%s##fluid-%d", fluid->name, fluid->id));

	if (expanded.fluid[fluid]) {

		Print("produced by");
		miniset<Recipe*> producers;
		for (auto& [_,recipe]: Recipe::names) {
			if (recipe->outputFluids.count(fluid->id)) {
				producers.insert(recipe);
			}
		}
		for (auto& recipe: producers) {
			drawRecipeButton(recipe);
		}

		Print("consumed by");
		miniset<Recipe*> consumers;
		for (auto& [_,recipe]: Recipe::names) {
			if (recipe->inputFluids.count(fluid->id)) {
				consumers.insert(recipe);
			}
		}
		for (auto& recipe: consumers) {
			drawRecipeButton(recipe);
		}
	}

	if (pop) {
		PopStyleColor(pop);
	}

	highlited.fluid[fluid] = false;
}

void RecipePopup::drawFluidButton(Fluid* fluid, int count) {
	using namespace ImGui;
	if (SmallButtonInline(fmtc("%s(%d)", fluid->name, count))) {
		locate.fluid = fluid;
		expanded.fluid[fluid] = !expanded.fluid[fluid];
	}
	highlited.fluid[fluid] = highlited.fluid[fluid] || IsItemHovered();
}

void RecipePopup::drawRecipe(Recipe* recipe) {
	using namespace ImGui;

	if (!showUnavailable && !recipe->licensed) return;

	if (locate.recipe == recipe) {
		SetScrollHereY();
		locate.recipe = nullptr;
	}

	int pop = 0;

	if (highlited.recipe[recipe]) {
		PushStyleColor(ImGuiCol_Header, GetStyleColorVec4(ImGuiCol_HeaderHovered));
		pop++;
	}

	if (!recipe->licensed) {
		PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled));
		pop++;
	}

	SetNextItemOpen(expanded.recipe[recipe]);
	expanded.recipe[recipe] = CollapsingHeader(fmtc("%s##recipe-%s", recipe->name, recipe->name));

	if (expanded.recipe[recipe]) {

		if (recipe->inputItems.size() || recipe->inputFluids.size()) {
			Print("inputs");
			for (auto [iid,count]: recipe->inputItems) {
				drawItemButton(Item::get(iid), count);
			}
			for (auto [fid,count]: recipe->inputFluids) {
				drawFluidButton(Fluid::get(fid), count);
			}
		}

		if (recipe->outputItems.size() || recipe->outputFluids.size()) {
			Print("outputs");
			for (auto [iid,count]: recipe->outputItems) {
				drawItemButton(Item::get(iid), count);
			}
			for (auto [fid,count]: recipe->outputFluids) {
				drawFluidButton(Fluid::get(fid), count);
			}
		}

		miniset<Spec*> specs;
		for (auto& [_,spec]: Spec::all) {
			for (auto& tag: spec->recipeTags) {
				if (recipe->tags.count(tag)) {
					specs.insert(spec);
				}
			}
		}
		if (specs.size()) {
			Print("made in");
			for (auto& spec: specs) {
				drawSpecButton(spec);
			}
		}

		miniset<Tech*> techs;
		for (auto& [_,tech]: Tech::names) {
			if (tech->licenseRecipes.count(recipe)) {
				techs.insert(tech);
			}
		}
		if (techs.size()) {
			Print("licensed by");
			for (auto& tech: techs) {
				drawTechButton(tech);
			}
		}
	}

	if (pop) {
		PopStyleColor(pop);
	}

	highlited.recipe[recipe] = false;
}

void RecipePopup::drawRecipeButton(Recipe* recipe) {
	using namespace ImGui;
	if (SmallButtonInline(recipe->name.c_str())) {
		locate.recipe = recipe;
		expanded.recipe[recipe] = !expanded.recipe[recipe];
	}
	highlited.recipe[recipe] = highlited.recipe[recipe] || IsItemHovered();
}

void RecipePopup::drawSpec(Spec* spec) {
	using namespace ImGui;

	if (!showUnavailable && !spec->licensed) return;

	if (locate.spec == spec) {
		SetScrollHereY();
		locate.spec = nullptr;
	}

	int pop = 0;

	if (highlited.spec[spec]) {
		PushStyleColor(ImGuiCol_Header, GetStyleColorVec4(ImGuiCol_HeaderHovered));
		pop++;
	}

	if (!spec->licensed) {
		PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled));
		pop++;
	}

	SetNextItemOpen(expanded.spec[spec]);
	expanded.spec[spec] = CollapsingHeader(fmtc("%s##build-spec-%s", spec->name, spec->name));

	if (expanded.spec[spec]) {

		if (spec->licensed && Button(fmtc("build##%s", spec->name), ImVec2(-1,0))) {
			camera->build(spec);
			show(false);
		}

		if (spec->materials.size()) {
			Print("materials");
			for (auto& [iid,count]: spec->materials) {
				drawItemButton(Item::get(iid), count);
			}
		}
		miniset<Tech*> techs;
		for (auto& tech: sorted.techs) {
			if (tech->licenseSpecs.count(spec)) {
				techs.insert(tech);
			}
		}
		if (techs.size()) {
			Print("licensed by");
			for (auto& tech: techs) {
				drawTechButton(tech);
			}
		}
	}

	if (pop) {
		PopStyleColor(pop);
	}

	highlited.spec[spec] = false;
}

void RecipePopup::drawSpecButton(Spec* spec) {
	using namespace ImGui;
	if (!spec->build) return;

	if (SmallButtonInline(spec->name.c_str())) {
		locate.spec = spec;
		expanded.spec[spec] = !expanded.spec[spec];
	}
	highlited.spec[spec] = highlited.spec[spec] || IsItemHovered();
}

void RecipePopup::drawTech(Tech* tech) {
	using namespace ImGui;

	if (!showUnavailable && Ledger::balance < tech->cost) return;

	if (locate.tech == tech) {
		SetScrollHereY();
		locate.tech = nullptr;
	}

	int pop = 0;

	if (highlited.tech[tech]) {
		PushStyleColor(ImGuiCol_Header, GetStyleColorVec4(ImGuiCol_HeaderHovered));
		pop++;
	}

	if (!tech->bought) {
		PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled));
		pop++;
	}

	SetNextItemOpen(expanded.tech[tech]);
	expanded.tech[tech] = CollapsingHeader(fmtc("%s##tech-%s", tech->name, tech->name));

	SameLine();
	PrintRight(tech->cost.format().c_str());

	if (expanded.tech[tech]) {

		if (!tech->bought && Ledger::balance >= tech->cost && Button(fmtc("buy##buy-tech-%s", tech->name), ImVec2(-1,0))) {
			tech->buy();
		}

		if (tech->licenseSpecs.size()) {
			Print("specs");
			for (auto& spec: tech->licenseSpecs) {
				drawSpecButton(spec);
			}
		}
		if (tech->licenseRecipes.size()) {
			Print("recipes");
			for (auto& recipe: tech->licenseRecipes) {
				drawRecipeButton(recipe);
			}
		}
	}

	if (pop) {
		PopStyleColor(pop);
	}

	highlited.tech[tech] = false;
}

void RecipePopup::drawTechButton(Tech* tech) {
	using namespace ImGui;
	if (SmallButtonInline(tech->name.c_str())) {
		locate.tech = tech;
		expanded.tech[tech] = !expanded.tech[tech];
	}
	highlited.tech[tech] = highlited.tech[tech] || IsItemHovered();
}
