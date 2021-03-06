#include "common.h"
#include "sim.h"
#include "tech.h"
#include "ledger.h"
#include "entity.h"

#include "json.hpp"
#include <fstream>
using json = nlohmann::json;

#include <filesystem>
namespace fs = std::filesystem;

namespace Save {
	json timeSeriesSave(TimeSeries* ts) {
		json state;
		state["secondMax"] = ts->secondMax;
		state["minuteMax"] = ts->minuteMax;
		state["hourMax"] = ts->hourMax;
		for (uint i = 0; i < 60; i++) {
			state["seconds"][i] = ts->seconds[i];
		}
		for (uint i = 0; i < 60; i++) {
			state["minutes"][i] = ts->minutes[i];
		}
		for (uint i = 0; i < 60; i++) {
			state["hours"][i] = ts->hours[i];
		}
		return state;
	}

	void timeSeriesLoad(TimeSeries* ts, json state) {
		ts->secondMax = state["secondMax"];
		ts->minuteMax = state["minuteMax"];
		ts->hourMax = state["hourMax"];
		for (uint i = 0; i < 60; i++) {
			ts->seconds[i] = state["seconds"][i];
		}
		for (uint i = 0; i < 60; i++) {
			ts->minutes[i] = state["minutes"][i];
		}
		for (uint i = 0; i < 60; i++) {
			ts->hours[i] = state["hours"][i];
		}
	}
}

namespace Sim {
	void save(const char* name) {
		auto path = std::string(name);
		fs::remove_all(path);
		fs::create_directory(path);

		auto out = std::ofstream(path + "/sim.json");

		json state;
		state["tick"] = tick;
		state["seed"] = seed;

		out << state << "\n";

		out.close();

		Spec::saveAll(name);
		Recipe::saveAll(name);
		Tech::saveAll(name);
		Chunk::saveAll(name);
		Entity::saveAll(name);
		Store::saveAll(name);
		Arm::saveAll(name);
		Vehicle::saveAll(name);
		Crafter::saveAll(name);
		Depot::saveAll(name);
		Drone::saveAll(name);
		Burner::saveAll(name);
		Pipe::saveAll(name);
		Conveyor::saveAll(name);
		Unveyor::saveAll(name);
		Loader::saveAll(name);
		Computer::saveAll(name);
		RopewayBucket::saveAll(name);
		Ropeway::saveAll(name);

		Ledger::save(name);
		Recipe::save(name);

		{
			auto out = std::ofstream(path + "/time-series.json");

			json state;
			state["statsElectricityDemand"] = Save::timeSeriesSave(&Sim::statsElectricityDemand);
			state["statsElectricitySupply"] = Save::timeSeriesSave(&Sim::statsElectricitySupply);
			state["statsEntity"] = Save::timeSeriesSave(&Sim::statsEntity);
			state["statsStore"] = Save::timeSeriesSave(&Sim::statsStore);
			state["statsArm"] = Save::timeSeriesSave(&Sim::statsArm);
			state["statsCrafter"] = Save::timeSeriesSave(&Sim::statsCrafter);
			state["statsConveyor"] = Save::timeSeriesSave(&Sim::statsConveyor);
			state["statsPath"] = Save::timeSeriesSave(&Sim::statsPath);
			state["statsVehicle"] = Save::timeSeriesSave(&Sim::statsVehicle);
			state["statsPipe"] = Save::timeSeriesSave(&Sim::statsPipe);
			state["statsShunt"] = Save::timeSeriesSave(&Sim::statsShunt);
			state["statsDepot"] = Save::timeSeriesSave(&Sim::statsDepot);
			state["statsDrone"] = Save::timeSeriesSave(&Sim::statsDrone);

			out << state << "\n";
			out.close();
		}
	}

	void load(const char* name) {
		auto path = std::string(name);
		auto in = std::ifstream(path + "/sim.json");

		for (std::string line; std::getline(in, line);) {
			auto state = json::parse(line);

			tick = state["tick"];
			reseed(state["seed"]);
		}

		in.close();

		Spec::loadAll(name);
		Recipe::loadAll(name);
		Tech::loadAll(name);
		Chunk::loadAll(name);
		Entity::loadAll(name);
		Store::loadAll(name);
		Arm::loadAll(name);
		Vehicle::loadAll(name);
		Crafter::loadAll(name);
		Depot::loadAll(name);
		Drone::loadAll(name);
		Burner::loadAll(name);
		Pipe::loadAll(name);
		Conveyor::loadAll(name);
		Unveyor::loadAll(name);
		Loader::loadAll(name);
		Computer::loadAll(name);
		RopewayBucket::loadAll(name);
		Ropeway::loadAll(name);

		Ledger::load(name);
		Recipe::load(name);

//		{
//			auto in = std::ifstream(path + "/time-series.json");
//
//			for (std::string line; std::getline(in, line);) {
//				auto state = json::parse(line);
//				Save::timeSeriesLoad(&Sim::statsElectricityDemand, state["statsElectricityDemand"]);
//				Save::timeSeriesLoad(&Sim::statsElectricitySupply, state["statsElectricitySupply"]);
//				Save::timeSeriesLoad(&Sim::statsEntity, state["statsEntity"]);
//				Save::timeSeriesLoad(&Sim::statsStore, state["statsStore"]);
//				Save::timeSeriesLoad(&Sim::statsArm, state["statsArm"]);
//				Save::timeSeriesLoad(&Sim::statsCrafter, state["statsCrafter"]);
//				Save::timeSeriesLoad(&Sim::statsPath, state["statsPath"]);
//				Save::timeSeriesLoad(&Sim::statsVehicle, state["statsVehicle"]);
//				//Save::timeSeriesLoad(&Sim::statsPipe, state["statsPipe"]);
//				Save::timeSeriesLoad(&Sim::statsShunt, state["statsShunt"]);
//				Save::timeSeriesLoad(&Sim::statsDepot, state["statsDepot"]);
//				Save::timeSeriesLoad(&Sim::statsDrone, state["statsDrone"]);
//			}
//
//			in.close();
//		}
	}
}

void Ledger::save(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/ledger.json");

	json state;
	state["balance"] = Ledger::balance.value;
	out << state << "\n";

	for (auto trx: Ledger::transactions) {
		json state;
		state["delta"] = trx.delta.value;
		state["desc"] = trx.desc;
		out << state << "\n";
	}

	out.close();
}

void Ledger::load(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/ledger.json");

	std::string line;
	std::getline(in, line);
	auto state = json::parse(line);
	Ledger::balance.value = state["balance"];

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Ledger::transactions.push_back({
			.delta = Currency(state["delta"]),
			.desc = state["desc"],
		});
	}

	in.close();
}

void Recipe::save(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/recipe.json");

	json state;
	state["miningRate"] = Recipe::miningRate;
	out << state << "\n";

	out.close();
}

void Recipe::load(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/recipe.json");

	std::string line;
	std::getline(in, line);
	auto state = json::parse(line);

	Recipe::miningRate = state["miningRate"];

	in.close();
}

void Chunk::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/chunks.json");

	for (auto pair: all) {
		Chunk *chunk = pair.second;
		json state;
		state["x"] = chunk->x;
		state["y"] = chunk->y;
		json minerals;
		for (auto [tx,ty]: chunk->meshMinerals) {
			minerals.push_back({tx,ty});
		}
		state["minerals"] = minerals;
		for (int ty = 0; ty < size; ty++) {
			for (int tx = 0; tx < size; tx++) {
				Tile* tile = &chunk->tiles[ty][tx];
				state["tiles"][ty][tx] = { tile->x, tile->y, tile->elevation, tile->mineral.iid, tile->mineral.size };
			}
		}
		out << state << "\n";
	}

	out.close();
}

void Chunk::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/chunks.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Chunk *chunk = new Chunk(state["x"], state["y"]);
		for (auto xy: state["minerals"]) {
			chunk->meshMinerals.push_back({xy[0], xy[1]});
		}
		for (int ty = 0; ty < size; ty++) {
			for (int tx = 0; tx < size; tx++) {
				Tile* tile = &chunk->tiles[ty][tx];
				tile->x = state["tiles"][ty][tx][0];
				tile->y = state["tiles"][ty][tx][1];
				tile->elevation = state["tiles"][ty][tx][2];
				tile->mineral = {(uint)(state["tiles"][ty][tx][3]), (uint)(state["tiles"][ty][tx][4])};
			}
		}
		chunk->generated = true;
		chunk->regenerate();
		chunk->findHills();
	}

	in.close();
}

void Spec::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/specs.json");

	for (auto& pair: all) {
		Spec *spec = pair.second;
		json state;
		state["name"] = spec->name;
		state["licensed"] = spec->licensed;
		out << state << "\n";
	}

	out.close();
}

void Spec::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/specs.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);

		Spec* spec = Spec::byName(state["name"]);
		spec->licensed = state["licensed"];
	}
}

void Recipe::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/recipes.json");

	for (auto& pair: ids) {
		Recipe *recipe = Recipe::get(pair.first);
		json state;
		state["name"] = recipe->name;
		state["licensed"] = recipe->licensed;
		out << state << "\n";
	}

	out.close();
}

void Recipe::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/recipes.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);

		Recipe* recipe = Recipe::byName(state["name"]);
		recipe->licensed = state["licensed"];
	}
}

void Tech::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/techs.json");

	for (auto& pair: Tech::names) {
		Tech *tech = pair.second;
		json state;
		state["name"] = tech->name;
		state["bought"] = tech->bought;
		out << state << "\n";
	}

	out.close();
}

void Tech::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/techs.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);

		Tech* tech = Tech::byName(state["name"]);
		tech->bought = state["bought"];
	}
}

void Entity::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/entities.json");

	json state;
	state["sequence"] = sequence;
	out << state << "\n";

	for (Entity& en: all) {
		json state;
		state["id"] = en.id;
		state["spec"] = en.spec->name;
		state["flags"] = en.flags;
		state["pos"] = { en.pos.x, en.pos.y, en.pos.z };
		state["dir"] = { en.dir.x, en.dir.y, en.dir.z };
		state["state"] = en.state;
		state["health"] = en.health;

		if (en.spec->named) {
			state["name"] = en.name();
		}

		out << state << "\n";
	}

	out.close();
}

void Entity::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/entities.json");

	std::string line;
	std::getline(in, line);
	auto state = json::parse(line);
	sequence = state["sequence"];

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Entity& en = create(state["id"], Spec::byName(state["spec"]));
		en.unindex();

		en.pos = (Point){state["pos"][0], state["pos"][1], state["pos"][2]};
		en.dir = (Point){state["dir"][0], state["dir"][1], state["dir"][2]};
		en.flags = state["flags"];

		en.state = state["state"];
		en.health = state["health"];

		// in case spec state animations changed across save or mod upgrade
		en.state = (uint)std::max(0, std::min((int)en.state, (int)en.spec->states.size()-1));

		en.index();

		if (!en.isGhost()) {
			en.ghost().destroy();
		}

		if (en.isConstruction()) {
			en.construct();
		}

		if (en.isDeconstruction()) {
			en.deconstruct();
		}

		if (en.spec->named) {
			en.rename(state["name"]);
		}

		en.setEnabled(true);
	}

	in.close();
}

void Store::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/stores.json");

	for (Store& store: all) {
		json state;
		state["id"] = store.id;
		state["sid"] = store.sid;
		state["activity"] = store.activity;

		int i = 0;
		for (Stack stack: store.stacks) {
			state["stacks"][i++] = {
				Item::get(stack.iid)->name,
				stack.size,
			};
		}

		i = 0;
		for (auto level: store.levels) {
			state["levels"][i++] = {
				Item::get(level.iid)->name,
				level.lower,
				level.upper,
				level.promised,
				level.reserved,
				level.craft,
			};
		}

		i = 0;
		for (uint did: store.drones) {
			state["drones"][i++] = did;
		}

		out << state << "\n";
	}

	out.close();
}

void Store::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/stores.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Store& store = get(state["id"]);
		store.sid = state["sid"];
		store.activity = state["activity"];

		for (auto stack: state["stacks"]) {
			store.stacks.push_back({
				Item::byName(stack[0])->id,
				stack[1],
			});
		}

		for (auto level: state["levels"]) {
			store.levels.push_back({
				.iid = Item::byName(level[0])->id,
				.lower = level[1],
				.upper = level[2],
				.promised = level[3],
				.reserved = level[4],
				//.craft = level[5],
			});
		}

		for (uint did: state["drones"]) {
			store.drones.insert(did);
		}
	}

	in.close();
}

void Vehicle::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/vehicles.json");

	for (auto& vehicle: all) {
		json state;

		state["id"] = vehicle.id;
		state["pause"] = vehicle.pause;
		state["patrol"] = vehicle.patrol;
		state["handbrake"] = vehicle.handbrake;

		int i = 0;
		for (Point point: vehicle.path) {
			state["path"][i++] = {point.x, point.y, point.z};
		}

		state["waypoint"] = -1;

		i = 0;
		for (auto wp: vehicle.waypoints) {

			if (wp == vehicle.waypoint) {
				state["waypoint"] = i;
			}

			json wstate;
			wstate["position"] = {wp->position.x, wp->position.y, wp->position.z};
			wstate["stopId"] = wp->stopId;

			int j = 0;
			for (auto condition: wp->conditions) {

				if_is<Vehicle::DepartInactivity>(condition, [&](Vehicle::DepartInactivity* con) {
					int k = j++;
					wstate["conditions"][k]["type"] = "inactivity";
					wstate["conditions"][k]["seconds"] = con->seconds;
				});

				if_is<Vehicle::DepartItem>(condition, [&](Vehicle::DepartItem* con) {
					if (con->iid) {
						int k = j++;
						wstate["conditions"][k]["type"] = "item";
						wstate["conditions"][k]["item"] = Item::get(con->iid)->name;
						wstate["conditions"][k]["op"] = con->op;
						wstate["conditions"][k]["count"] = con->count;
					}
				});
			}

			state["waypoints"][i++] = wstate;
		}

		out << state << "\n";
	}

	out.close();
}

void Vehicle::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/vehicles.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Vehicle& vehicle = get(state["id"]);
		vehicle.pause = state["pause"];
		vehicle.patrol = state["patrol"];
		vehicle.handbrake = state["handbrake"];

		for (auto array: state["path"]) {
			vehicle.path.push_back(Point(array[0], array[1], array[2]));
		}

		int i = 0;
		for (auto wstate: state["waypoints"]) {
			uint stopId = wstate["stopId"];

			if (stopId) {
				auto wp = vehicle.addWaypoint(stopId);

				for (auto cstate: wstate["conditions"]) {
					std::string type = cstate["type"];

					if (type == "inactivity") {
						auto con = new Vehicle::DepartInactivity();
						con->seconds = cstate["seconds"];
						wp->conditions.push_back(con);
					}

					if (type == "item") {
						auto con = new Vehicle::DepartItem();
						con->iid = Item::byName(cstate["item"])->id;
						con->op = cstate["op"];
						con->count = cstate["count"];
						wp->conditions.push_back(con);
					}

				}
			} else {
				auto pos = wstate["position"];
				vehicle.addWaypoint(Point(pos[0], pos[1], pos[2]));
			}

			if (i++ == state["waypoint"]) {
				vehicle.waypoint = vehicle.waypoints.back();
			}
		}
	}

	in.close();
}

void Arm::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/arms.json");

	for (auto& arm: all) {

		json state;
		state["id"] = arm.id;
		if (arm.iid) state["item"] = Item::get(arm.iid)->name;
		state["orientation"] = arm.orientation;
		state["stage"] = arm.stage;
		state["pause"] = arm.pause;
		state["filter"] = arm.filter;

		out << state << "\n";
	}

	out.close();
}

void Arm::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/arms.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Arm& arm = get(state["id"]);
		arm.iid = state["item"].is_null() ? 0: Item::byName(state["item"])->id;
		arm.orientation = state["orientation"];
		arm.stage = state["stage"];
		arm.pause = state["pause"];

		for (uint iid: state["filter"]) {
			arm.filter.insert(iid);
		}
	}

	in.close();
}

void Crafter::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/crafters.json");

	for (auto& crafter: all) {

		json state;
		state["id"] = crafter.id;
		state["working"] = crafter.working;
		state["progress"] = crafter.progress;
		state["completed"] = crafter.completed;
		state["once"] = crafter.once;
		if (crafter.recipe) state["recipe"] = crafter.recipe->name;

		out << state << "\n";
	}

	out.close();
}

void Crafter::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/crafters.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Crafter& crafter = get(state["id"]);
		crafter.working = state["working"];
		crafter.progress = state["progress"];
		crafter.completed = state["completed"];
		crafter.once = state["once"];
		crafter.recipe = state["recipe"].is_null() ? NULL: Recipe::byName(state["recipe"]);
	}

	in.close();
}

void Depot::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/depots.json");

	for (auto& depot: all) {

		json state;
		state["id"] = depot.id;

		int i = 0;
		for (uint did: depot.drones) {
			state["drones"][i++] = did;
		}

		out << state << "\n";
	}

	out.close();
}

void Depot::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/depots.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Depot& depot = get(state["id"]);

		for (uint did: state["drones"]) {
			depot.drones.insert(did);
		}
	}

	in.close();
}

void Drone::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/drones.json");

	for (auto& drone: all) {

		json state;
		state["id"] = drone.id;
		state["dep"] = drone.dep;
		state["src"] = drone.src;
		state["dst"] = drone.dst;
		state["srcGhost"] = drone.srcGhost;
		state["dstGhost"] = drone.dstGhost;
		state["stage"] = drone.stage;
		state["stack"] = { Item::get(drone.stack.iid)->name, drone.stack.size };

		out << state << "\n";
	}

	out.close();
}

void Drone::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/drones.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Drone& drone = get(state["id"]);
		drone.dep = state["dep"];
		drone.src = state["src"];
		drone.dst = state["dst"];
		drone.srcGhost = state["srcGhost"];
		drone.dstGhost = state["dstGhost"];
		drone.stage = state["stage"];
		drone.stack = { Item::byName(state["stack"][0])->id, state["stack"][1] };
	}

	in.close();
}

void Burner::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/burners.json");

	for (auto& burner: all) {

		json state;
		state["id"] = burner.id;
		state["energy"] = burner.energy.value;
		state["buffer"] = burner.buffer.value;

		state["store"]["sid"] = burner.store.sid;
		state["store"]["activity"] = burner.store.activity;

		int i = 0;
		for (Stack stack: burner.store.stacks) {
			state["store"]["stacks"][i++] = {
				Item::get(stack.iid)->name,
				stack.size,
			};
		}

		i = 0;
		for (auto level: burner.store.levels) {
			state["store"]["levels"][i++] = {
				Item::get(level.iid)->name,
				level.lower,
				level.upper,
				level.promised,
				level.reserved,
			};
		}

		i = 0;
		for (uint did: burner.store.drones) {
			state["store"]["drones"][i++] = did;
		}

		out << state << "\n";
	}

	out.close();
}

void Burner::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/burners.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Burner& burner = get(state["id"]);
		burner.energy.value = state["energy"];
		burner.buffer.value = state["buffer"];

		burner.store.sid = state["store"]["sid"];
		burner.store.activity = state["store"]["activity"];

		for (auto stack: state["store"]["stacks"]) {
			burner.store.stacks.push_back({
				Item::byName(stack[0])->id,
				stack[1],
			});
		}

		for (auto level: state["store"]["levels"]) {
			burner.store.levels.push_back({
				.iid = Item::byName(level[0])->id,
				.lower = level[1],
				.upper = level[2],
				.promised = level[3],
				.reserved = level[4],
			});
		}

		for (uint did: state["drones"]) {
			burner.store.drones.insert(did);
		}
	}

	in.close();
}

void Pipe::saveAll(const char* name) {

	for (auto network: PipeNetwork::all) {
		network->cacheState();
	}

	auto path = std::string(name);
	auto out = std::ofstream(path + "/pipes.json");

	for (auto& pipe: all) {

		json state;
		state["id"] = pipe.id;
		state["cacheFid"] = pipe.cacheFid;
		state["cacheTally"] = pipe.cacheTally;
		out << state << "\n";
	}

	out.close();
}

void Pipe::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/pipes.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Pipe& pipe = get(state["id"]);
		pipe.cacheFid = state["cacheFid"];
		pipe.cacheTally = state["cacheTally"];
	}

	PipeNetwork::rebuild = true;

	in.close();
}

void Conveyor::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/conveyors.json");

	for (auto& conveyor: all) {

		json state;
		state["id"] = conveyor.id;
		state["iid"] = conveyor.iid;
		state["offset"] = conveyor.offset;
		state["prev"] = conveyor.prev;
		state["next"] = conveyor.next;
		state["side"] = conveyor.side;
		out << state << "\n";
	}

	out.close();
}

void Conveyor::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/conveyors.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Conveyor& conveyor = get(state["id"]);
		conveyor.iid = state["iid"];
		conveyor.offset = state["offset"];
		conveyor.prev = state["prev"];
		conveyor.next = state["next"];
		conveyor.side = state["side"];
	}

	in.close();
}

void Unveyor::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/unveyors.json");

	for (auto& unveyor: all) {

		json state;
		state["id"] = unveyor.id;
		state["partner"] = unveyor.partner;

		for (uint i = 0; i < unveyor.items.size(); i++) {
			auto& item = unveyor.items[i];
			state["items"][i] = { item.offset, item.iid };
		}

		out << state << "\n";
	}

	out.close();
}

void Unveyor::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/unveyors.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Unveyor& unveyor = get(state["id"]);
		unveyor.partner = state["partner"];

		for (auto item: state["items"]) {
			unveyor.items.push_back((Unveyor::item){ .offset = item[0], .iid = item[1] });
		}
	}

	in.close();
}

void Loader::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/loaders.json");

	for (auto& loader: all) {

		json state;
		state["id"] = loader.id;
		state["storeId"] = loader.storeId;
		state["pause"] = loader.pause;
		out << state << "\n";
	}

	out.close();
}

void Loader::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/loaders.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Loader& loader = get(state["id"]);
		loader.storeId = state["storeId"];
		loader.pause = state["pause"];
	}

	in.close();
}

void Computer::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/computers.json");

	for (auto& computer: all) {

		json state;
		state["id"] = computer.id;
		out << state << "\n";
	}

	out.close();
}

void Computer::loadAll(const char* name) {
	//auto path = std::string(name);
	//auto in = std::ifstream(path + "/computers.json");

	//for (std::string line; std::getline(in, line);) {
	//	auto state = json::parse(line);
	//	Computer& computer = get(state["id"]);
	//	computer.iid = state["iid"];
	//	computer.offset = state["offset"];
	//	computer.prev = state["prev"];
	//	computer.next = state["next"];
	//}

	//in.close();
}

void Ropeway::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/ropeways.json");

	for (auto& ropeway: all) {

		json state;
		state["id"] = ropeway.id;
		state["prev"] = ropeway.prev;
		state["next"] = ropeway.next;
		state["cycle"] = ropeway.cycle;

		int i = 0;
		for (auto& bid: ropeway.buckets) {
			state["buckets"][i++] = bid;
		}

		i = 0;
		for (auto& iid: ropeway.inputFilters) {
			state["inputFilters"][i++] = iid;
		}

		i = 0;
		for (auto& iid: ropeway.outputFilters) {
			state["outputFilters"][i++] = iid;
		}

		out << state << "\n";
	}

	out.close();
}

void Ropeway::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/ropeways.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Ropeway& ropeway = get(state["id"]);
		ropeway.prev = state["prev"];
		ropeway.next = state["next"];
		ropeway.cycle = state["cycle"];

		for (uint bid: state["buckets"]) {
			ropeway.buckets.push_back(bid);
		}

		for (uint bid: state["inputFilters"]) {
			ropeway.inputFilters.insert(bid);
		}

		for (uint bid: state["outputFilters"]) {
			ropeway.outputFilters.insert(bid);
		}

		ropeway.check = true;
	}

	in.close();
}

void RopewayBucket::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/ropeway-buckets.json");

	for (auto& bucket: all) {

		json state;
		state["id"] = bucket.id;
		state["rid"] = bucket.rid;
		state["step"] = bucket.step;
		out << state << "\n";
	}

	out.close();
}

void RopewayBucket::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/ropeway-buckets.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		RopewayBucket& bucket = get(state["id"]);
		bucket.rid = state["rid"];
		bucket.step = state["step"];
	}

	in.close();
}
