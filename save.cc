#include "common.h"
#include "sim.h"
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
		Chunk::saveAll(name);
		Entity::saveAll(name);
		Store::saveAll(name);
		Belt::saveAll(name);
		Lift::saveAll(name);
		Arm::saveAll(name);
		Vehicle::saveAll(name);
		Crafter::saveAll(name);
		Depot::saveAll(name);
		Drone::saveAll(name);
		Burner::saveAll(name);

		{
			auto out = std::ofstream(path + "/time-series.json");

			json state;
			state["statsElectricityDemand"] = Save::timeSeriesSave(&Sim::statsElectricityDemand);
			state["statsElectricitySupply"] = Save::timeSeriesSave(&Sim::statsElectricitySupply);
			state["statsEntity"] = Save::timeSeriesSave(&Sim::statsEntity);
			state["statsStore"] = Save::timeSeriesSave(&Sim::statsStore);
			state["statsArm"] = Save::timeSeriesSave(&Sim::statsArm);
			state["statsCrafter"] = Save::timeSeriesSave(&Sim::statsCrafter);
			state["statsPath"] = Save::timeSeriesSave(&Sim::statsPath);
			state["statsVehicle"] = Save::timeSeriesSave(&Sim::statsVehicle);
			state["statsBelt"] = Save::timeSeriesSave(&Sim::statsBelt);
			state["statsLift"] = Save::timeSeriesSave(&Sim::statsLift);
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
		Chunk::loadAll(name);
		Entity::loadAll(name);
		Store::loadAll(name);
		Belt::loadAll(name);
		Lift::loadAll(name);
		Arm::loadAll(name);
		Vehicle::loadAll(name);
		Crafter::loadAll(name);
		Depot::loadAll(name);
		Drone::loadAll(name);
		Burner::loadAll(name);

		{
			auto in = std::ifstream(path + "/time-series.json");

			for (std::string line; std::getline(in, line);) {
				auto state = json::parse(line);
				Save::timeSeriesLoad(&Sim::statsElectricityDemand, state["statsElectricityDemand"]);
				Save::timeSeriesLoad(&Sim::statsElectricitySupply, state["statsElectricitySupply"]);
				Save::timeSeriesLoad(&Sim::statsEntity, state["statsEntity"]);
				Save::timeSeriesLoad(&Sim::statsStore, state["statsStore"]);
				Save::timeSeriesLoad(&Sim::statsArm, state["statsArm"]);
				Save::timeSeriesLoad(&Sim::statsCrafter, state["statsCrafter"]);
				Save::timeSeriesLoad(&Sim::statsPath, state["statsPath"]);
				Save::timeSeriesLoad(&Sim::statsVehicle, state["statsVehicle"]);
				Save::timeSeriesLoad(&Sim::statsBelt, state["statsBelt"]);
				Save::timeSeriesLoad(&Sim::statsLift, state["statsLift"]);
				Save::timeSeriesLoad(&Sim::statsShunt, state["statsShunt"]);
				Save::timeSeriesLoad(&Sim::statsDepot, state["statsDepot"]);
				Save::timeSeriesLoad(&Sim::statsDrone, state["statsDrone"]);
			}

			in.close();
		}
	}
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
		for (auto [tx,ty]: chunk->minerals) {
			minerals.push_back({tx,ty});
		}
		state["minerals"] = minerals;
		for (int ty = 0; ty < size; ty++) {
			for (int tx = 0; tx < size; tx++) {
				Tile* tile = &chunk->tiles[ty][tx];
				state["tiles"][ty][tx] = { tile->elevation, tile->resource, tile->mineral };
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
			chunk->minerals.push_back({xy[0], xy[1]});
		}
		for (int ty = 0; ty < size; ty++) {
			for (int tx = 0; tx < size; tx++) {
				Tile* tile = &chunk->tiles[ty][tx];
				tile->elevation = state["tiles"][ty][tx][0];
				tile->resource = state["tiles"][ty][tx][1];
				tile->mineral = state["tiles"][ty][tx][2];
			}
		}
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
		out << state << "\n";
	}

	out.close();
}

void Spec::loadAll(const char* name) {
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
		// in case spec state animations changed across save or mod upgrade
		en.state = (uint)std::max(0, std::min((int)en.state, (int)en.spec->states.size()-1));

		en.index();

		if (en.isGhost()) {
			Ghost::create(en.id);
		}

		if (en.isConstruction()) {
			en.construct();
		}

		if (en.isDeconstruction()) {
			en.deconstruct();
		}
	}

	in.close();
}

void Store::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/stores.json");

	for (Store& store: all) {
		json state;
		state["id"] = store.id;
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
		store.activity = state["activity"];

		for (auto stack: state["stacks"]) {
			store.stacks.push_back({
				Item::byName(stack[0])->id,
				stack[1],
			});
		}

		for (auto level: state["levels"]) {
			store.levels.push_back({
				iid: Item::byName(level[0])->id,
				lower: level[1],
				upper: level[2],
				promised: level[3],
				reserved: level[4],
			});
		}

		for (uint did: state["drones"]) {
			store.drones.insert(did);
		}
	}

	in.close();
}

void Belt::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/belts.json");

	for (Belt& belt: all) {
		json state;
		state["id"] = belt.id;
		state["offset"] = belt.offset;

		out << state << "\n";
	}

	out.close();

	BeltSegment::saveAll(name);
}

void Belt::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/belts.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Belt& belt = get(state["id"]);
		belt.offset = state["offset"];
	}

	in.close();

	BeltSegment::loadAll(name);
}

void BeltSegment::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/belt-segments.json");

	for (BeltSegment* segment: all) {
		json state;

		int i = 0;
		for (Belt* belt: segment->belts) {
			state["belts"][i++] = belt->id;
		}

		i = 0;
		for (BeltItem beltItem: segment->items) {
			state["items"][i++] = {
				Item::get(beltItem.iid)->name,
				beltItem.offset,
			};
		}

		state["pauseOffload"] = segment->pauseOffload;
		state["pauseLoad"] = segment->pauseLoad;

		out << state << "\n";
	}

	out.close();
}

void BeltSegment::loadAll(const char* name) {
	while (all.size()) {
		delete *all.begin();
	}

	auto path = std::string(name);
	auto in = std::ifstream(path + "/belt-segments.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		BeltSegment* segment = new BeltSegment();

		segment->changed = true;
		segment->pauseOffload = state["pauseOffload"];
		segment->pauseLoad = state["pauseLoad"];

		for (auto id: state["belts"]) {
			Belt* belt = &Belt::get(id);
			segment->belts.push_back(belt);
			belt->segment = segment;
		}

		for (auto array: state["items"]) {
			segment->items.push_back({
				iid: Item::byName(array[0])->id,
				offset: array[1],
			});
		}
	}

	in.close();
}

void Lift::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/lifts.json");

	for (auto& pair: all) {
		Lift& lift = pair.second;

		json state;
		state["id"] = lift.id;
		if (lift.iid) state["item"] = Item::get(lift.iid)->name;
		state["ascent"] = lift.ascent;
		state["mode"] = lift.mode;
		state["stage"] = lift.stage;
		state["pause"] = lift.pause;

		out << state << "\n";
	}

	out.close();
}

void Lift::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/lifts.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Lift& lift = get(state["id"]);

		lift.iid = state["item"].is_null() ? 0: Item::byName(state["item"])->id;
		lift.ascent = state["ascent"];
		lift.mode = state["mode"];
		lift.stage = state["stage"];
		lift.pause = state["pause"];
	}

	in.close();
}

void Vehicle::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/vehicles.json");

	for (Vehicle& vehicle: all) {
		json state;
		state["id"] = vehicle.id;
		state["pause"] = vehicle.pause;

		int i = 0;
		for (Point point: vehicle.path) {
			state["path"][i++] = {point.x, point.y, point.z};
		}

		for (Point point: vehicle.legs) {
			state["legs"][i++] = {point.x, point.y, point.z};
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

		for (auto array: state["path"]) {
			vehicle.path.push_back(Point(array[0], array[1], array[2]));
		}

		for (auto array: state["legs"]) {
			vehicle.legs.push_back(Point(array[0], array[1], array[2]));
		}
	}

	in.close();
}

void Arm::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/arms.json");

	for (auto& pair: all) {
		Arm& arm = pair.second;

		json state;
		state["id"] = arm.id;
		if (arm.iid) state["item"] = Item::get(arm.iid)->name;
		state["orientation"] = arm.orientation;
		state["stage"] = arm.stage;
		state["pause"] = arm.pause;

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
	}

	in.close();
}

void Crafter::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/crafters.json");

	for (auto& pair: all) {
		Crafter& crafter = pair.second;

		json state;
		state["id"] = crafter.id;
		state["working"] = crafter.working;
		state["progress"] = crafter.progress;
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
		crafter.recipe = state["recipe"].is_null() ? NULL: Recipe::byName(state["recipe"]);
	}

	in.close();
}

void Depot::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/depots.json");

	for (auto& pair: all) {
		Depot& depot = pair.second;

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

		burner.store.activity = state["store"]["activity"];

		for (auto stack: state["store"]["stacks"]) {
			burner.store.stacks.push_back({
				Item::byName(stack[0])->id,
				stack[1],
			});
		}

		for (auto level: state["store"]["levels"]) {
			burner.store.levels.push_back({
				iid: Item::byName(level[0])->id,
				lower: level[1],
				upper: level[2],
				promised: level[3],
				reserved: level[4],
			});
		}

		for (uint did: state["drones"]) {
			burner.store.drones.insert(did);
		}
	}

	in.close();
}

