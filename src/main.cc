
// This is a bit spaghetti-ish and eventually needs to be refactored.
// Probably scripted in Wren.

#include "raylib-ex.h"

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/implot.h"

#include "common.h"
#include "mod.h"
#include "sim.h"
#include "energy.h"
#include "chunk.h"
#include "spec.h"
#include "entity.h"
#include "view.h"
#include "item.h"
#include "fluid.h"
#include "tech.h"
#include "recipe.h"
#include "ledger.h"
#include "popup.h"
#include <ctime>
#include <filesystem>
#include <thread>
#include <atomic>

const char* quotes[] = {
	"Yeah, but she's our witch. So cut her the hell down!",
	"Gotta say, Doctor, your talent for alienating folk is near miraculous.",
	"We came here in peace, we expect to go in one... piece.",
	"Doors and corners.",
	"Indeed.",
	"That man is playing Galaga! Thought we wouldn't notice. But we did.",
	"Oh, that's a crown. I thought it was a big eyebrow.",
	"I make grave mistakes all the time. Everything seems to work out.",
	"Though a candle burns in my house... there's nobody home.",
};

void scenario() {
	Sim::reset();
	Sim::reseed(879600773);
	Sim::reseedThread();
	//Sim::seed(4);

	auto droplet = Thing("models/fluid.stl");

	Mesh oreLD = GenMeshSphere(0.6,6,6);
	Thing::meshes.push_back(oreLD);

	Item* item;

	auto thingSheet = Thing("models/sheet-hd.stl", "models/sheet-ld.stl");
	auto thingIngot = Thing("models/ingot.stl");

	auto thingContainer = Thing("models/container-hd.stl", "models/container-ld.stl");
	auto thingFan = Thing("models/fan-hd.stl", "models/fan-ld.stl");
	auto thingGear = Thing("models/gear-hd.stl", "models/gear-ld.stl");
	auto thingMiner = Thing("models/miner.stl");
	auto thingTruckChassisEngineer = Thing("models/truck-chassis-engineer.stl");
	//auto thingTruckChassisHauler = Thing("models/truck-chassis-hauler.stl");
	auto thingTruckWheel = Thing("models/truck-wheel.stl");

	item = new Item(Item::next(), "gear-wheel");
	item->parts = {
		(new Part(thingGear))->paint(0x888888ff)->gloss(16)->scale(0.8,0.8,0.8),
	};
	item->armV = 0.38f;

	auto thingBatteryTerminal = Thing("models/battery-terminal-hd.stl", "models/battery-terminal-ld.stl");

	item = new Item(Item::next(), "battery");
	item->parts = {
		(new Part(Thing("models/battery-body-hd.stl", "models/battery-body-ld.stl")))->paint(0xcc6600ff),
		(new Part(Thing("models/battery-cap-hd.stl", "models/battery-cap-ld.stl")))->paint(0x888888ff),
		(new Part(thingBatteryTerminal))->paint(0x660000ff)->translate(0,0,-0.1),
		(new Part(thingBatteryTerminal))->paint(0x004400ff)->translate(0,0,0.1),
	};

	item = new Item(Item::next(), "electric-motor");
	item->parts = {
		(new Part(Thing("models/electric-motor-body-hd.stl", "models/electric-motor-body-ld.stl")))->paint(0xB22222ff),
		(new Part(Thing("models/electric-motor-foot-hd.stl", "models/electric-motor-foot-ld.stl")))->paint(0xB0C4DEff),
		(new Part(Thing("models/electric-motor-shaft-hd.stl", "models/electric-motor-shaft-ld.stl")))->paint(0x888888ff),
	};

	Fluid* fluid = new Fluid(Fluid::next(), "water");
	fluid->color = BLUE;

	fluid = new Fluid(Fluid::next(), "steam");
	fluid->thermal = Energy::kJ(20);
	fluid->color = GRAY;

	Tech* tech;
	Recipe* recipe;

	for (auto iid: Item::mining) {
		Item* item = Item::get(iid);
		Recipe* recipe = new Recipe(Recipe::next(), "mine-" + item->name);
		recipe->energyUsage = Energy::kJ(200);
		recipe->tags = {"mining"};
		recipe->mine = iid;
		recipe->parts = {
			(new Part(thingMiner))->paint(0xb25607ff)->scale(0.1f,0.1f,0.1f),
		};
		for (auto part: item->parts) {
			recipe->parts.push_back(part);
		}
		recipe->licensed = true;
	}

	for (uint i = 1; i < 10; i++) {
		tech = new Tech(Tech::next(), fmt("mining-%u", i));
		tech->tags = {"mining"};
		tech->cost = Currency::k(i);
		tech->miningRate = 1.0f + ((float)i * 0.1);
		//tech->parts = {
		//	(new Part(thingMiner))->paint(0xb25607ff)->scale(0.1f,0.1f,0.1f),
		//};
	}

	recipe = new Recipe(Recipe::next(), "copper-smelting1");
	recipe->energyUsage = Energy::kJ(300);
	recipe->tags = {"smelting"};
	recipe->inputItems = {
		{ Item::byName("copper-ore")->id, 1 },
	};
	recipe->outputItems = {
		{ Item::byName("copper-ingot")->id, 1 },
	};
	recipe->parts = {
		(new Part(thingIngot))->paint(0xda8a67ff),
	};
	recipe->licensed = true;

	recipe = new Recipe(Recipe::next(), "steel-smelting1");
	recipe->energyUsage = Energy::kJ(600);
	recipe->tags = {"smelting"};
	recipe->inputItems = {
		{ Item::byName("iron-ore")->id, 2 },
	};
	recipe->outputItems = {
		{ Item::byName("steel-ingot")->id, 1 },
	};
	recipe->parts = {
		(new Part(thingIngot))->paint(0xB0C4DEff),
	};
	recipe->licensed = true;

	recipe = new Recipe(Recipe::next(), "brick-smelting1");
	recipe->energyUsage = Energy::kJ(300);
	recipe->tags = {"smelting"};
	recipe->inputItems = {
		{ Item::byName("stone")->id, 1 },
	};
	recipe->outputItems = {
		{ Item::byName("brick")->id, 2 },
	};
	recipe->parts = Item::byName("brick")->parts;
	recipe->licensed = true;

	recipe = new Recipe(Recipe::next(), "copper-sheet");
	recipe->energyUsage = Energy::kJ(300);
	recipe->tags = {"crafting"};
	recipe->inputItems = {
		{ Item::byName("copper-ingot")->id, 1 },
	};
	recipe->outputItems = {
		{ Item::byName("copper-sheet")->id, 2 },
	};
	recipe->parts = {
		(new Part(thingSheet))->paint(0xda8a67ff),
	};
	recipe->licensed = true;

	recipe = new Recipe(Recipe::next(), "steel-sheet");
	recipe->energyUsage = Energy::kJ(300);
	recipe->tags = {"crafting"};
	recipe->inputItems = {
		{ Item::byName("steel-ingot")->id, 1 },
	};
	recipe->outputItems = {
		{ Item::byName("steel-sheet")->id, 2 },
	};
	recipe->parts = {
		(new Part(thingSheet))->paint(0xB0C4DEff),
	};
	recipe->licensed = true;

	recipe = new Recipe(Recipe::next(), "copper-wire");
	recipe->energyUsage = Energy::kJ(300);
	recipe->tags = {"crafting"};
	recipe->inputItems = {
		{ Item::byName("copper-ingot")->id, 1 },
	};
	recipe->outputItems = {
		{ Item::byName("copper-wire")->id, 2 },
	};
	recipe->parts = Item::byName("copper-wire")->parts;
	recipe->licensed = true;

	recipe = new Recipe(Recipe::next(), "circuit-board");
	recipe->energyUsage = Energy::kJ(600);
	recipe->tags = {"crafting"};
	recipe->inputItems = {
		{ Item::byName("log")->id, 1 },
		{ Item::byName("copper-wire")->id, 10 },
	};
	recipe->outputItems = {
		{ Item::byName("circuit-board")->id, 50 },
	};
	recipe->parts = Item::byName("circuit-board")->parts;
	recipe->licensed = true;

	recipe = new Recipe(Recipe::next(), "gear-wheel");
	recipe->energyUsage = Energy::kJ(600);
	recipe->tags = {"crafting"};
	recipe->inputItems = {
		{ Item::byName("steel-ingot")->id, 1 },
	};
	recipe->outputItems = {
		{ Item::byName("gear-wheel")->id, 2 },
	};
	recipe->parts = Item::byName("gear-wheel")->parts;
	recipe->licensed = true;

	recipe = new Recipe(Recipe::next(), "pipe");
	recipe->energyUsage = Energy::kJ(600);
	recipe->tags = {"crafting"};
	recipe->inputItems = {
		{ Item::byName("copper-ingot")->id, 1 },
	};
	recipe->outputItems = {
		{ Item::byName("pipe")->id, 1 },
	};
	recipe->parts = Item::byName("pipe")->parts;
	recipe->licensed = true;

	recipe = new Recipe(Recipe::next(), "battery");
	recipe->energyUsage = Energy::kJ(900);
	recipe->tags = {"crafting"};
	recipe->inputItems = {
		{ Item::byName("steel-sheet")->id, 1 },
		{ Item::byName("copper-sheet")->id, 1 },
	};
	recipe->outputItems = {
		{ Item::byName("battery")->id, 2 },
	};
	recipe->parts = Item::byName("battery")->parts;

	tech = new Tech(Tech::next(), "batteries");
	tech->tags = {"products"};
	tech->cost = Currency::k(1);
	tech->licenseRecipes.insert(Recipe::byName("battery"));

	recipe = new Recipe(Recipe::next(), "electric-motor");
	recipe->energyUsage = Energy::kJ(900);
	recipe->tags = {"crafting"};
	recipe->inputItems = {
		{ Item::byName("steel-sheet")->id, 1 },
		{ Item::byName("circuit-board")->id, 1 },
		{ Item::byName("gear-wheel")->id, 1 },
	};
	recipe->outputItems = {
		{ Item::byName("electric-motor")->id, 1 },
	};
	recipe->parts = Item::byName("electric-motor")->parts;

	tech = new Tech(Tech::next(), "electric-motors");
	tech->tags = {"products"};
	tech->cost = Currency::k(1);
	tech->licenseRecipes.insert(Recipe::byName("electric-motor"));

	Spec* spec = new Spec("provider-container");
	spec->licensed = true;
	spec->collision = {0, 0, 0, 2, 2, 5};
	spec->selection = spec->collision;
	spec->parts = {
		(new Part(thingContainer))->paint(0xB22222ff)->gloss(16),
	};
	spec->store = true;
	spec->capacity = Mass::kg(1000);
	spec->storeSetUpper = true;
	spec->logistic = true;
	spec->rotate = true;
	spec->health = 10;
	spec->materials = {
		{Item::byName("steel-sheet")->id, 5},
	};

	spec = new Spec("requester-container");
	spec->collision = {0, 0, 0, 2, 2, 5};
	spec->selection = spec->collision;
	spec->parts = {
		(new Part(thingContainer))->paint(0x0044ccff)->gloss(16),
	};
	spec->store = true;
	spec->capacity = Mass::kg(1000);
	spec->logistic = true;
	spec->storeSetLower = true;
	spec->storeSetUpper = true;
	spec->rotate = true;
	spec->health = 10;
	spec->materials = {
		{Item::byName("copper-sheet")->id, 5},
	};

	tech = new Tech(Tech::next(), "requester-containers");
	tech->tags = {"storage"};
	tech->cost = Currency::k(10);
	tech->licenseSpecs.insert(Spec::byName("requester-container"));

	auto thingAssemblerPiston = Thing("models/assembler-piston-hd.stl", "models/assembler-piston-ld.stl");

	spec = new Spec("assembler");
	spec->licensed = true;
	spec->store = true;
	spec->capacity = Mass::kg(100);
	spec->loadPriority = true;
	spec->rotate = true;
	spec->crafter = true;
	spec->enable = true;
	spec->crafterProgress = false;
	spec->recipeTags = {"crafting"};
	spec->consumeElectricity = true;
	spec->energyConsume = Energy::kW(300);
	spec->energyDrain = Energy::kW(9);
	spec->collision = {0, 0, 0, 6, 3, 6};
	spec->selection = spec->collision;
	spec->health = 10;
	spec->pipeInputConnections = {
		Point(3.0f, -1.0f, 1.5f).transform(Mat4::rotateY(DEG2RAD*0)),
		Point(3.0f, -1.0f, 1.5f).transform(Mat4::rotateY(DEG2RAD*90)),
		Point(3.0f, -1.0f, 1.5f).transform(Mat4::rotateY(DEG2RAD*180)),
		Point(3.0f, -1.0f, 1.5f).transform(Mat4::rotateY(DEG2RAD*270)),
	};
	spec->pipeOutputConnections = {
		Point(3.0f, -1.0f, -1.5f).transform(Mat4::rotateY(DEG2RAD*0)),
		Point(3.0f, -1.0f, -1.5f).transform(Mat4::rotateY(DEG2RAD*90)),
		Point(3.0f, -1.0f, -1.5f).transform(Mat4::rotateY(DEG2RAD*180)),
		Point(3.0f, -1.0f, -1.5f).transform(Mat4::rotateY(DEG2RAD*270)),
	};
	spec->parts = {
		(new Part(Thing("models/assembler-chassis-hd.stl", "models/assembler-chassis-ld.stl")))->paint(0x009900ff)->gloss(16),
		(new PartSpinner(thingFan, 1))->paint(0x666666ff)->rotate(Point::East, 70)->translate(0,0.5,2.5)->gloss(16),
		(new PartSpinner(thingFan, 1))->paint(0x666666ff)->rotate(Point::West, 70)->translate(0,0.5,-2.5)->gloss(16),
		(new PartSpinner(thingFan, 1))->paint(0x666666ff)->rotate(Point::North, 70)->translate(2.5,0.5,0)->gloss(16),
		(new PartSpinner(thingFan, 1))->paint(0x666666ff)->rotate(Point::South, 70)->translate(-2.5,0.5,0)->gloss(16),
		(new Part(thingAssemblerPiston))->paint(0x666666ff)->gloss(16),
		(new Part(thingAssemblerPiston))->paint(0x666666ff)->translate(0,0,1.2)->gloss(16),
		(new Part(thingAssemblerPiston))->paint(0x666666ff)->translate(0,0,-1.2)->gloss(16),
		(new Part(thingGear))->paint(0xcaccceff)->scale(1.5,1.8,1.5)->rotate(Point::East, 90)->translate(1,1,-1.2)->gloss(16),
		(new Part(thingGear))->paint(0xcaccceff)->scale(1.1,1.8,1.1)->rotate(Point::East, 90)->translate(1,1,-0.6)->gloss(16),
		(new Part(thingGear))->paint(0xcaccceff)->scale(1.3,1.8,1.3)->rotate(Point::East, 90)->translate(1,1, 0.0)->gloss(16),
		(new Part(thingGear))->paint(0xcaccceff)->scale(1.2,1.8,1.2)->rotate(Point::East, 90)->translate(1,1, 0.6)->gloss(16),
		(new Part(thingGear))->paint(0xcaccceff)->scale(1.4,1.8,1.4)->rotate(Point::East, 90)->translate(1,1, 1.2)->gloss(16),
	};
	spec->materials = {
		{ Item::byName("steel-sheet")->id, 5 },
		{ Item::byName("circuit-board")->id, 3 },
	};

	{
		Mat4 state0 = Mat4::identity;

		for (int i = 0; i < 360; i++) {
			Mat4 state1 = Mat4::rotateY((float)(i*5)*DEG2RAD);

			Mat4 piston1 = state0;
			Mat4 piston2 = state0;
			Mat4 piston3 = state0;

			if (i >=   0 && i <  60) piston1 = Mat4::translate(0,(float)(i%60)*-0.01f,0);
			if (i >=  60 && i < 120) piston1 = Mat4::translate(0,(float)-0.60+(i%60)*0.01f,0);

			if (i >= 120 && i < 180) piston2 = Mat4::translate(0,(float)(i%60)*-0.01f,0);
			if (i >= 180 && i < 240) piston2 = Mat4::translate(0,(float)-0.60+(i%60)*0.01f,0);

			if (i >= 240 && i < 300) piston3 = Mat4::translate(0,(float)(i%60)*-0.01f,0);
			if (i >= 300 && i < 360) piston3 = Mat4::translate(0,(float)-0.60+(i%60)*0.01f,0);

			spec->states.push_back({
				state0,
				state1,
				state1,
				state1,
				state1,

				piston1,
				piston2,
				piston3,

				Mat4::rotateY((float)i*1.0f*DEG2RAD),
				Mat4::rotateY((float)i*-0.8f*DEG2RAD),
				Mat4::rotateY((float)i*0.6f*DEG2RAD),
				Mat4::rotateY((float)i*-0.4f*DEG2RAD),
				Mat4::rotateY((float)i*0.2f*DEG2RAD),
			});
		}
	}

	spec = new Spec("furnace");
	spec->licensed = true;
	spec->collision = {0, 0, 0, 4, 4, 4};
	spec->selection = spec->collision;
	spec->parts = {
		(new Part(Thing("models/furnace-hd.stl", "models/furnace-ld.stl")))->paint(0xDAA520ff),
		(new Part(Thing("models/furnace-fire-hd.stl", "models/furnace-fire-ld.stl")))->paint(0x000000ff),
		(new Part(Thing("models/furnace-smoke-hd.stl", "models/furnace-smoke-ld.stl")))->paint(0x000000ff),
		(new PartSmoke(2400, 20, 0.01, 0.5f, 0.01f, 0.005f, 0.1f, 0.99f, 60, 180))->translate(0,2,0),
	};
	spec->health = 10;
	spec->store = true;
	spec->capacity = Mass::kg(100);
	spec->rotate = true;
	spec->crafter = true;
	spec->loadPriority = true;
	spec->crafterProgress = true;
	spec->recipeTags = {"smelting"};
	spec->consumeChemical = true;
	spec->energyConsume = Energy::kW(100);
	spec->energyDrain = Energy::kW(3);
	spec->materials = {
		{ Item::byName("copper-sheet")->id, 3 },
		{ Item::byName("brick")->id, 3 },
	};

	for (uint i = 0; i < 10; i++) {
		float fi = (float)i;
		spec->states.push_back({
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::scale(0.1f*fi, 0.1f*fi, 0.1f*fi),
		});
	}

	for (uint i = 0; i < 80; i++) {
		spec->states.push_back({
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
		});
	}

	for (uint i = 0; i < 10; i++) {
		float fi = (float)(9-i);
		spec->states.push_back({
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::scale(0.1f*fi, 0.1f*fi, 0.1f*fi),
		});
	}

	spec = new Spec("miner");
	spec->licensed = true;
	spec->store = true;
	spec->capacity = Mass::kg(10);
	spec->rotate = true;
	spec->place = Spec::Footings;
	spec->footings = {
		{.place = Spec::Hill, .point = Point(-2,-3,0)},
		{.place = Spec::Hill, .point = Point(2,-3,0)},
		{.place = Spec::Land, .point = Point(0,-3,4)},
	};
	spec->crafter = true;
	spec->crafterProgress = false;
	spec->enable = true;
	spec->recipeTags = {"mining"};
	spec->consumeElectricity = true;
	spec->energyConsume = Energy::kW(100);
	spec->energyDrain = Energy::kW(3);
	spec->collision = {0, 0, 0, 5, 5, 10};
	spec->selection = spec->collision;
	spec->health = 10;
	spec->parts = {
		(new Part(thingMiner))->paint(0xCD853Fff),
		(new Part(thingGear))->paint(0xccccccff)->scale(3,3,3)->rotate(Point::East, 90)->translate(0,0,3.75)->gloss(16),
	};
	spec->materials = {
		{ Item::byName("steel-sheet")->id, 3 },
	};

	{
		Mat4 state0 = Mat4::identity;

		for (int i = 0; i < 180; i++) {
			Mat4 state1 = Mat4::rotateY((float)i*DEG2RAD);

			spec->states.push_back({
				state0,
				state1,
			});
		}
	}

	spec = new Spec("offshore-pump");
	spec->licensed = true;
	spec->rotate = true;
	spec->place = Spec::Water;
	spec->pipeOutputConnections = {
		{1.5f, -1.0f, 0.0f},
	};
	spec->consumeElectricity = true;
	spec->energyConsume = Energy::kW(100);
	spec->energyDrain = Energy::kW(3);
	spec->collision = {0, 0, 0, 3, 3, 3};
	spec->selection = spec->collision;
	spec->crafter = true;
	spec->enable = true;
	spec->recipeTags = {"offshore-pumping"};
	spec->health = 10;
	spec->parts = {
		(new Part(Thing("models/offshore-pump-chassis-hd.stl", "models/offshore-pump-chassis-ld.stl")))->paint(0x4444ccff)->translate(0,-1.5,0),
		(new Part(Thing("models/offshore-pump-pipe-hd.stl", "models/offshore-pump-pipe-ld.stl")))->paint(0xccccccff)->translate(0,-1.5,0),
	};
	spec->materials = {
		{ Item::byName("steel-sheet")->id, 3 },
	};

	recipe = new Recipe(Recipe::next(), "offshore-pumping");
	recipe->energyUsage = Energy::kJ(10);
	recipe->tags = {"offshore-pumping"};
	recipe->outputFluids = {
		{ Fluid::byName("water")->id, 1000 },
	};
	recipe->licensed = true;

	auto waterDroplet = new Part(droplet);
	waterDroplet->color = Fluid::byName("water")->color;

	recipe->parts = {waterDroplet};

	auto beltSurface = Thing("models/belt-surface-hd.stl", "models/belt-surface-ld.stl");
	auto beltRidge = Thing("models/belt-ridge-hd.stl", "models/belt-ridge-ld.stl");

	spec = new Spec("conveyor");
	spec->licensed = true;
	spec->collision = {0, 0, 0, 1, 2, 1};
	spec->selection = {0, -0.75, 0, 1, 0.5, 1};
	spec->rotate = true;
	spec->conveyor = true;
	spec->conveyorInput = Point::North;
	spec->conveyorOutput = Point::South;
	spec->consumeElectricity = true;
	spec->energyDrain = Energy::kW(1);
	spec->health = 10;

	{
		Point base = Point::South * 0.5f;
		Point step = Point::North * (1.0f/30.0f);
		for (int i = 0; i < 30; i++) {
			Point p = base + (step * (float)i);
			spec->conveyorTransforms.push_back(p.translation());
		}
	}

	{
		std::vector<Mat4> ridgeTransforms(spec->conveyorTransforms.rbegin(), spec->conveyorTransforms.rend());
		spec->parts = {
			(new Part(Thing("models/belt-base-hd.stl", "models/belt-base-ld.stl")))->paint(0xffff00ff)->translate(0,-1.5,0),
			(new Part(beltSurface))->paint(0x111111ff)->translate(0,-1.5,0),
			(new PartCycle2(beltRidge, ridgeTransforms))->paint(0xffff00ff)->translate(0,-1.5,0)->ld(false),
		};
	}

	spec->materials = {
		{ Item::byName("gear-wheel")->id, 1 },
		{ Item::byName("steel-sheet")->id, 1 },
	};

	spec = new Spec("conveyor-right");
	spec->licensed = true;
	spec->build = false;
	spec->collision = {0, 0, 0, 1, 2, 1};
	spec->selection = {0, -0.75, 0, 1, 0.5, 1};
	spec->selection = spec->collision;
	spec->rotate = true;
	spec->conveyor = true;
	spec->conveyorInput = Point::East;
	spec->conveyorOutput = Point::South;
	spec->consumeElectricity = true;
	spec->energyDrain = Energy::kW(1);
	spec->health = 10;

	{
		Point base = Point::North*0.5f;
		Mat4 a = Mat4::translate(Point::South*0.5f + Point::East*0.5f);
		float step = -(90.0f/30.0f)*DEG2RAD;
		for (int i = 29; i >= 0; i--) {
			Mat4 r = Mat4::rotateY(step * (float)i);
			Mat4 t = Mat4::translate(base.transform(r * a));
			Mat4 d = Mat4::rotateY(-step * (float)(29-i));
			spec->conveyorTransforms.push_back(d * t);
		}
	}

	{
		std::vector<Mat4> ridgeTransforms(spec->conveyorTransforms.rbegin(), spec->conveyorTransforms.rend());
		spec->parts = {
			(new Part(Thing("models/belt-right-base-hd.stl", "models/belt-right-base-ld.stl")))
				->paint(0xffff00ff)->scale(0.001, 0.001, 0.001)->translate(0,-1.5,0),
			(new Part(Thing("models/belt-right-surface-hd.stl")))->paint(0x111111ff)->translate(0,-1.5,0),
			(new PartCycle2(beltRidge, ridgeTransforms))->paint(0xffff00ff)->translate(0,-1.5,0)->ld(false),
		};
	}

	spec->materials = {
		{ Item::byName("gear-wheel")->id, 1 },
		{ Item::byName("steel-sheet")->id, 1 },
	};

	spec = new Spec("conveyor-left");
	spec->licensed = true;
	spec->build = false;
	spec->collision = {0, 0, 0, 1, 2, 1};
	spec->selection = {0, -0.75, 0, 1, 0.5, 1};
	spec->selection = spec->collision;
	spec->rotate = true;
	spec->conveyor = true;
	spec->conveyorInput = Point::West;
	spec->conveyorOutput = Point::South;
	spec->consumeElectricity = true;
	spec->energyDrain = Energy::kW(1);
	spec->health = 10;

	{
		Point base = Point::North*0.5f;
		Mat4 a = Mat4::translate(Point::South*0.5f + Point::West*0.5f);
		float step = (90.0f/30.0f)*DEG2RAD;
		for (int i = 29; i >= 0; i--) {
			Mat4 r = Mat4::rotateY(step * (float)i);
			Mat4 t = Mat4::translate(base.transform(r * a));
			Mat4 d = Mat4::rotateY(-step * (float)(29-i));
			spec->conveyorTransforms.push_back(d * t);
		}
	}

	{
		std::vector<Mat4> ridgeTransforms(spec->conveyorTransforms.rbegin(), spec->conveyorTransforms.rend());
		spec->parts = {
			(new Part(Thing("models/belt-left-base-hd.stl")))->paint(0xffff00ff)->translate(0,-1.5,0),
			(new Part(Thing("models/belt-left-surface-hd.stl")))->paint(0x111111ff)->translate(0,-1.5,0),
			(new PartCycle2(beltRidge, ridgeTransforms))->paint(0xffff00ff)->translate(0,-1.5,0)->ld(false),
		};
	}

	spec->materials = {
		{ Item::byName("gear-wheel")->id, 1 },
		{ Item::byName("steel-sheet")->id, 1 },
	};

	// conveyors are modelled for output==direction, but it seems easier to visualise in-game
	// as input==direction. So the cycle order is reversed for a clockwise rotation
	Spec::byName("conveyor")->cycle = Spec::byName("conveyor-left");
	Spec::byName("conveyor-left")->cycle = Spec::byName("conveyor-right");
	Spec::byName("conveyor-right")->cycle = Spec::byName("conveyor");

	Spec::byName("conveyor-right")->pipette = Spec::byName("conveyor");
	Spec::byName("conveyor-left")->pipette = Spec::byName("conveyor");

	Spec::byName("conveyor-left")->statsGroup = Spec::byName("conveyor");
	Spec::byName("conveyor-right")->statsGroup = Spec::byName("conveyor");

	spec = new Spec("unveyor-entry");
	spec->licensed = true;
	spec->collision = {0, 0, 0, 1, 2, 2};
	spec->selection = spec->collision;
	spec->rotate = true;
	spec->unveyor = true;
	spec->unveyorEntry = true;
	spec->unveyorRange = 10.0f;
	spec->conveyor = true;
	spec->conveyorInput = Point::North*1.5f;
	spec->conveyorOutput = Point::Zero;
	spec->consumeElectricity = true;
	spec->energyDrain = Energy::kW(1);
	spec->health = 10;

	{
		Point base = Point::Zero;
		Point step = Point::North * (1.0f/30.0f);
		for (int i = 0; i < 30; i++) {
			Point p = base + (step * (float)i);
			spec->conveyorTransforms.push_back(p.translation());
		}
	}

	{
		std::vector<Mat4> ridgeTransforms(spec->conveyorTransforms.rbegin(), spec->conveyorTransforms.rend());
		spec->parts = {
			(new Part(Thing("models/unveyor-base-hd.stl", "models/unveyor-base-ld.stl")))
				->scale(0.001, 0.001, 0.001)->paint(0xffff00ff)->translate(0,-1.25,0),
			(new Part(Thing("models/belt-base-hd.stl", "models/belt-base-ld.stl")))->paint(0xffff00ff)->translate(0,-1.5,-0.5),
			(new Part(beltSurface))->paint(0x111111ff)->translate(0,-1.5,-0.5),
			(new PartCycle2(beltRidge, ridgeTransforms))->paint(0xffff00ff)->translate(0,-1.5,0)->ld(false),
		};
	}

	spec->materials = {
		{ Item::byName("steel-sheet")->id, 2 },
		{ Item::byName("gear-wheel")->id, 2 },
	};

	spec = new Spec("unveyor-exit");
	spec->licensed = true;
	spec->collision = {0, 0, 0, 1, 2, 2};
	spec->selection = spec->collision;
	spec->rotate = true;
	spec->unveyor = true;
	spec->unveyorEntry = false;
	spec->unveyorRange = 10.0f;
	spec->conveyor = true;
	spec->conveyorInput = Point::Zero;
	spec->conveyorOutput = Point::South*1.5f;
	spec->consumeElectricity = true;
	spec->energyDrain = Energy::kW(1);
	spec->health = 10;

	{
		Point base = Point::South;
		Point step = Point::North * (1.0f/30.0f);
		for (int i = 0; i < 30; i++) {
			Point p = base + (step * (float)i);
			spec->conveyorTransforms.push_back(p.translation());
		}
	}

	{
		std::vector<Mat4> ridgeTransforms(spec->conveyorTransforms.rbegin(), spec->conveyorTransforms.rend());
		spec->parts = {
			(new Part(Thing("models/unveyor-base-hd.stl", "models/unveyor-base-ld.stl")))->paint(0xffff00ff)
				->scale(0.001, 0.001, 0.001)->rotate(Point::Up, 180)->translate(0,-1.25,0),
			(new Part(Thing("models/belt-base-hd.stl", "models/belt-base-ld.stl")))->paint(0xffff00ff)->translate(0,-1.5,0.5),
			(new Part(beltSurface))->paint(0x111111ff)->translate(0,-1.5,0.5),
			(new PartCycle2(beltRidge, ridgeTransforms))->paint(0xffff00ff)->translate(0,-1.5,0)->ld(false),
		};
	}

	spec->materials = {
		{ Item::byName("steel-sheet")->id, 2 },
		{ Item::byName("gear-wheel")->id, 2 },
	};

	Spec::byName("unveyor-entry")->cycle = Spec::byName("unveyor-exit");
	Spec::byName("unveyor-entry")->cycleReverseDirection = true;
	Spec::byName("unveyor-exit")->cycle = Spec::byName("unveyor-entry");
	Spec::byName("unveyor-exit")->cycleReverseDirection = true;

	Spec::byName("unveyor-exit")->statsGroup = Spec::byName("unveyor-entry");

	spec = new Spec("loader");
	spec->collision = {0, 0, 0, 1, 2, 2};
	spec->selection = spec->collision;
	spec->rotate = true;
	spec->loader = true;
	spec->loaderUnload = false;
	spec->loaderPoint = Point::South*1.5f;
	spec->conveyor = true;
	spec->conveyorInput = Point::North*1.5f;
	spec->conveyorOutput = Point::Zero;
	spec->consumeElectricity = true;
	spec->energyDrain = Energy::kW(1);
	spec->health = 10;

	{
		Point base = Point::Zero;
		Point step = Point::North * (1.0f/30.0f);
		for (int i = 0; i < 30; i++) {
			Point p = base + (step * (float)i);
			spec->conveyorTransforms.push_back(p.translation());
		}
	}

	{
		std::vector<Mat4> ridgeTransforms(spec->conveyorTransforms.rbegin(), spec->conveyorTransforms.rend());
		spec->parts = {
			(new Part(Thing("models/loader-base-hd.stl", "models/loader-base-ld.stl")))
				->scale(0.001, 0.001, 0.001)->paint(0xffff00ff)->translate(0,-1,0),
			(new Part(Thing("models/belt-base-hd.stl", "models/belt-base-ld.stl")))->paint(0xffff00ff)->translate(0,-1.5,-0.5),
			(new Part(beltSurface))->paint(0x111111ff)->translate(0,-1.5,-0.5),
			(new PartCycle2(beltRidge, ridgeTransforms))->paint(0xffff00ff)->translate(0,-1.5,0)->ld(false),
		};
	}

	spec->materials = {
		{ Item::byName("steel-sheet")->id, 1 },
		{ Item::byName("gear-wheel")->id, 2 },
	};

	spec = new Spec("unloader");
	spec->collision = {0, 0, 0, 1, 2, 2};
	spec->selection = spec->collision;
	spec->rotate = true;
	spec->loader = true;
	spec->loaderUnload = true;
	spec->loaderPoint = Point::North*1.5f;
	spec->conveyor = true;
	spec->conveyorInput = Point::Zero;
	spec->conveyorOutput = Point::South*1.5f;
	spec->consumeElectricity = true;
	spec->energyDrain = Energy::kW(1);
	spec->health = 10;

	{
		Point base = Point::South;
		Point step = Point::North * (1.0f/30.0f);
		for (int i = 0; i < 30; i++) {
			Point p = base + (step * (float)i);
			spec->conveyorTransforms.push_back(p.translation());
		}
	}

	{
		std::vector<Mat4> ridgeTransforms(spec->conveyorTransforms.rbegin(), spec->conveyorTransforms.rend());
		spec->parts = {
			(new Part(Thing("models/loader-base-hd.stl", "models/loader-base-ld.stl")))->paint(0xffff00ff)
				->scale(0.001, 0.001, 0.001)->rotate(Point::Up, 180)->translate(0,-1,0),
			(new Part(Thing("models/belt-base-hd.stl", "models/belt-base-ld.stl")))->paint(0xffff00ff)->translate(0,-1.5,0.5),
			(new Part(beltSurface))->paint(0x111111ff)->translate(0,-1.5,0.5),
			(new PartCycle2(beltRidge, ridgeTransforms))->paint(0xffff00ff)->translate(0,-1.5,0)->ld(false),
		};
	}

	spec->materials = {
		{ Item::byName("steel-sheet")->id, 1 },
		{ Item::byName("gear-wheel")->id, 2 },
	};

	Spec::byName("loader")->cycle = Spec::byName("unloader");
	Spec::byName("loader")->cycleReverseDirection = true;
	Spec::byName("unloader")->cycle = Spec::byName("loader");
	Spec::byName("unloader")->cycleReverseDirection = true;

	Spec::byName("unloader")->statsGroup = Spec::byName("loader");

	tech = new Tech(Tech::next(), "loaders");
	tech->tags = {"logistics"};
	tech->cost = Currency::k(5);
	tech->licenseSpecs.insert(Spec::byName("loader"));
	tech->licenseSpecs.insert(Spec::byName("unloader"));
	//tech->parts = {
	//	(new Part(thingMiner))->paint(0xb25607ff)->scale(0.1f,0.1f,0.1f),
	//};

	spec = new Spec("ropeway-terminus");
	spec->collision = {0, 0, 0, 5, 10, 5};
	spec->selection = spec->collision;
	spec->enable = true;
	spec->rotate = false;
	spec->ropeway = true;
	spec->ropewayTerminus = true;
	spec->ropewayCableEast = (Point::East*1.5f) + (Point::Up*5.0f);
	spec->store = true;
	spec->capacity = Mass::kg(1000);
	spec->storeSetUpper = true;
	spec->consumeElectricity = true;
	spec->energyConsume = Energy::MW(1);
	spec->energyDrain = Energy::kW(33);
	spec->health = 10;

	spec->parts = {
		(new Part(Thing("models/cablecar-terminus-hd.stl", "models/cablecar-terminus-ld.stl")))->paint(0xffff00ff)->translate(0,-5,0),
		(new Part(Thing("models/cablecar-mast-hd.stl", "models/cablecar-mast-ld.stl")))->paint(0xffff00ff)->translate(0,-5,0)->pivots(),
	};

	spec = new Spec("ropeway-tower");
	spec->collision = {0, 0, 0, 3, 10, 3};
	spec->selection = spec->collision;
	spec->place = Spec::Land | Spec::Hill;
	spec->placeOnHill = true;
	spec->rotate = false;
	spec->ropeway = true;
	spec->ropewayTower = true;
	spec->ropewayCableEast = (Point::East*1.5f) + (Point::Up*5.0f);
	spec->consumeElectricity = true;
	spec->energyConsume = Energy::kW(100);
	spec->energyDrain = Energy::kW(3);
	spec->health = 10;

	spec->parts = {
		(new Part(Thing("models/cablecar-tower-hd.stl", "models/cablecar-tower-ld.stl")))->paint(0xffff00ff)->translate(0,-5,0),
		(new Part(Thing("models/cablecar-mast-hd.stl", "models/cablecar-mast-ld.stl")))->paint(0xffff00ff)->translate(0,-5,0)->pivots(),
	};

	spec = new Spec("ropeway-bucket");
	spec->build = false;
	spec->collision = {0, 0, 0, 2, 4, 2};
	spec->selection = spec->collision;
	spec->rotate = false;
	spec->ropewayBucket = true;
	spec->align = false;
	spec->store = true;
	spec->capacity = Mass::kg(1000);
	spec->health = 10;

	spec->parts = {
		(new Part(Thing("models/cablecar-bucket-hd.stl", "models/cablecar-bucket-ld.stl")))->paint(0xffff00ff)->translate(0,-2,0),
	};

	Spec::byName("ropeway-terminus")->cycle = Spec::byName("ropeway-tower");
	Spec::byName("ropeway-tower")->cycle = Spec::byName("ropeway-terminus");

	Spec::byName("ropeway-terminus")->ropewayBucketSpec = Spec::byName("ropeway-bucket");

	tech = new Tech(Tech::next(), "ropeways");
	tech->tags = {"logistics"};
	tech->cost = Currency::k(10);
	tech->licenseSpecs.insert(Spec::byName("ropeway-terminus"));
	tech->licenseSpecs.insert(Spec::byName("ropeway-tower"));

	spec = new Spec("fluid-tank");
	spec->licensed = true;
	spec->pipe = true;
	spec->collision = {0, 0, 0, 5, 3, 5};
	spec->selection = spec->collision;
	spec->pipeConnections = {Point::North*2.5f+Point::Down, Point::South*2.5f+Point::Down, Point::East*2.5f+Point::Down, Point::West*2.5f+Point::Down};
	spec->pipeCapacity = Liquid::l(50000);
	spec->health = 10;

	spec->parts = {
		(new Part(Thing("models/fluid-tank-base-hd.stl", "models/fluid-tank-base-ld.stl")))->paint(0xffa500ff)->translate(0,-1.5,0),
	};

	spec->materials = {
		{ Item::byName("steel-sheet")->id, 3 },
	};

	spec = new Spec("pipe-straight");
	spec->licensed = true;
	spec->pipe = true;
	spec->pipeCapacity = Liquid::l(100);
	spec->collision = {0, 0, 0, 1, 1, 1};
	spec->selection = spec->collision;
	spec->rotate = true;
	spec->pipeConnections = {Point::North*0.5f, Point::South*0.5f};
	spec->health = 10;

	spec->parts = {
		(new Part(Thing("models/pipe-straight-hd.stl", "models/pipe-straight-ld.stl")))->paint(0xffa500ff)->rotate(Point::Up, -90),
	};

	spec->materials = {
		{ Item::byName("pipe")->id, 1 },
	};

	spec = new Spec("pipe-cross");
	spec->licensed = true;
	spec->build = false;
	spec->pipe = true;
	spec->pipeCapacity = Liquid::l(100);
	spec->collision = {0, 0, 0, 1, 1, 1};
	spec->selection = spec->collision;
	spec->rotate = true;
	spec->pipeConnections = {Point::North*0.5f, Point::South*0.5f, Point::East*0.5f, Point::West*0.5f};
	spec->health = 10;

	spec->parts = {
		(new Part(Thing("models/pipe-cross-hd.stl", "models/pipe-cross-ld.stl")))->paint(0xffa500ff)->rotate(Point::Up, -90),
	};

	spec->materials = {
		{ Item::byName("pipe")->id, 1 },
	};

	spec = new Spec("pipe-tee");
	spec->licensed = true;
	spec->build = false;
	spec->pipe = true;
	spec->pipeCapacity = Liquid::l(100);
	spec->collision = {0, 0, 0, 1, 1, 1};
	spec->selection = spec->collision;
	spec->rotate = true;
	spec->pipeConnections = {Point::South*0.5f, Point::East*0.5f, Point::West*0.5f};
	spec->health = 10;

	spec->parts = {
		(new Part(Thing("models/pipe-tee-hd.stl", "models/pipe-tee-ld.stl")))->paint(0xffa500ff)->rotate(Point::Up, -90),
	};

	spec->materials = {
		{ Item::byName("pipe")->id, 1 },
	};

	spec = new Spec("pipe-elbow");
	spec->licensed = true;
	spec->build = false;
	spec->pipe = true;
	spec->pipeCapacity = Liquid::l(100);
	spec->collision = {0, 0, 0, 1, 1, 1};
	spec->selection = spec->collision;
	spec->rotate = true;
	spec->pipeConnections = {Point::South*0.5f, Point::East*0.5f};
	spec->health = 10;

	spec->parts = {
		(new Part(Thing("models/pipe-elbow-hd.stl", "models/pipe-elbow-ld.stl")))->paint(0xffa500ff)->rotate(Point::Up, -90),
	};

	spec->materials = {
		{ Item::byName("pipe")->id, 1 },
	};

	spec = new Spec("pipe-ground");
	spec->licensed = true;
	spec->pipe = true;
	spec->pipeCapacity = Liquid::l(500);
	spec->pipeUnderground = true;
	spec->pipeUndergroundRange = 10.0f;
	spec->collision = {0, 0, 0, 1, 1, 1};
	spec->selection = spec->collision;
	spec->rotate = true;
	spec->pipeConnections = {Point::North*0.5f};
	spec->health = 10;

	spec->parts = {
		(new Part(Thing("models/pipe-ground-hd.stl", "models/pipe-ground-ld.stl")))->paint(0xffa500ff)->rotate(Point::Up, -90),
	};

	spec->materials = {
		{ Item::byName("pipe")->id, 5 },
	};

	Spec::byName("pipe-straight")->cycle = Spec::byName("pipe-ground");
	Spec::byName("pipe-ground")->cycle = Spec::byName("pipe-elbow");
	Spec::byName("pipe-elbow")->cycle = Spec::byName("pipe-tee");
	Spec::byName("pipe-tee")->cycle = Spec::byName("pipe-cross");
	Spec::byName("pipe-cross")->cycle = Spec::byName("pipe-straight");

	std::vector<Spec*> rocks;

	for (int i = 1; i < 4; i++) {
		auto name = "rock" + std::to_string(i);
		auto part = "models/" + name + ".stl";

		spec = new Spec(name);
		spec->build = false;
		spec->collision = {0, 0, 0, 2, 1, 2};
		spec->selection = spec->collision;
		spec->health = 100;
		spec->pivot = true;
		spec->junk = true;
		spec->parts = {
			(new Part(Thing(part)))->paint(0x888888ff),
		};
		spec->materials = {
			{Item::byName("stone")->id, 1},
		};

		rocks.push_back(spec);
	}

	Chunk::generator([=](Chunk *chunk) {
		for (int y = 0; y < Chunk::size; y++) {
			for (int x = 0; x < Chunk::size; x++) {
				Box bounds = {(float)chunk->x*Chunk::size+x, 0.0f, (float)chunk->y*Chunk::size+y, 2.0f, 1.0f, 2.0f};
				if (Chunk::isLand(bounds.grow(1.0f))) {
					double n = Sim::noise2D(chunk->x*Chunk::size+x + 1000000, chunk->y*Chunk::size+y + 1000000, 8, 0.6, 0.015);
					if (n < 0.4 && Sim::random() < 0.04) {
						Spec *spec = rocks[Sim::choose(rocks.size())];
						Entity::create(Entity::next(), spec)
							.look(Point::South.randomHorizontal())
							.move((Point){
								(float)(chunk->x*Chunk::size+x),
								spec->collision.h/2.0f,
								(float)(chunk->y*Chunk::size+y),
							})
							.materialize()
						;
					}
				}
			}
		}
	});

	std::vector<Spec*> trees;

	spec = new Spec("tree1");
	spec->build = false;
	spec->collision = {0, 0, 0, 2, 5, 2};
	spec->selection = spec->collision;
	spec->pivot = true;
	spec->junk = true;
	spec->health = 10;
	spec->parts = {
		(new Part(Thing("models/tree1.stl").smooth()))->paint(0x556B2Fff)->translate(0,-2.5,0),
	};
	spec->materials = {
		{Item::byName("log")->id, 1},
	};

	trees.push_back(spec);

	spec = new Spec("tree2");
	spec->build = false;
	spec->collision = {0, 0, 0, 2, 6, 2};
	spec->selection = spec->collision;
	spec->pivot = true;
	spec->junk = true;
	spec->health = 10;
	spec->parts = {
		(new Part(Thing("models/tree2.stl").smooth()))->paint(0x228B22ff)->translate(-5,-3,0),
	};
	spec->materials = {
		{Item::byName("log")->id, 1},
	};

	trees.push_back(spec);

	Chunk::generator([=](Chunk *chunk) {
		for (int y = 0; y < Chunk::size; y++) {
			for (int x = 0; x < Chunk::size; x++) {
				float e = chunk->tiles[y][x].elevation;
				if (e > -0.01) {
					double n = Sim::noise2D(chunk->x*Chunk::size+x + 2000000, chunk->y*Chunk::size+y + 2000000, 8, 0.6, 0.015);
					if (n < 0.4 && Sim::random() < 0.04) {
						Spec *spec = trees[Sim::choose(trees.size())];
						Entity::create(Entity::next(), spec)
							.look(Point::South.randomHorizontal())
							.move((Point){
								(float)(chunk->x*Chunk::size+x),
								(e*100.0f) + spec->collision.h/2.0f,
								(float)(chunk->y*Chunk::size+y),
							})
							.materialize()
						;
					}
				}
			}
		}
	});

	spec = new Spec("truck-engineer");
	spec->collision = {0, 0, 0, 2, 2, 3};
	spec->selection = spec->collision;
	spec->parts = {
		(new Part(thingTruckChassisEngineer))->paint(0xffa500ff)->translate(0,0.3,0),
		(new Part(thingTruckWheel))->paint(0x444444ff)->translate(-0.8,-0.75,-1),
		(new Part(thingTruckWheel))->paint(0x444444ff)->translate(-0.8,-0.75,0),
		(new Part(thingTruckWheel))->paint(0x444444ff)->translate(-0.8,-0.75,1),
		(new Part(thingTruckWheel))->paint(0x444444ff)->translate(0.8,-0.75,-1),
		(new Part(thingTruckWheel))->paint(0x444444ff)->translate(0.8,-0.75,0),
		(new Part(thingTruckWheel))->paint(0x444444ff)->translate(0.8,-0.75,1),
	};
	spec->health = 100;
	spec->align = false;
	spec->pivot = true;
	spec->vehicle = true;
	spec->energyConsume = Energy::kW(50);
	spec->consumeChemical = true;
	spec->store = true;
	spec->crafter = true;
	spec->crafterShowTab = false;
	spec->crafterManageStore = false;
	spec->crafterEnergyConsume = Energy::kW(300);
	spec->recipeTags = {"smelting", "crafting"};
	spec->capacity = Mass::kg(1000);
	spec->logistic = true;
	spec->storeSetLower = true;
	spec->storeSetUpper = true;
	spec->generateElectricity = true;
	spec->energyGenerate = Energy::MW(1);
	spec->forceDelete = true;

	spec->depot = true;
	spec->drones = 10;

	spec->costGreedy = 1.3;
	spec->clearance = 1.5;

	spec->materials = {
		{ Item::byName("electric-motor")->id, 2 },
		{ Item::byName("steel-sheet")->id, 2 },
		{ Item::byName("gear-wheel")->id, 2 },
		{ Item::byName("circuit-board")->id, 2 },
	};

	tech = new Tech(Tech::next(), "truck-engineers");
	tech->tags = {"vehicles"};
	tech->cost = Currency::k(10);
	tech->licenseSpecs.insert(Spec::byName("truck-engineer"));

	spec = new Spec("truck-hauler");
	spec->collision = {0, 0, 0, 2, 2, 3};
	spec->selection = spec->collision;
	spec->parts = {
		(new Part(thingTruckChassisEngineer))->paint(0xffcc00ff)->translate(0,0.3,0),
		Spec::byName("truck-engineer")->parts[1],
		Spec::byName("truck-engineer")->parts[2],
		Spec::byName("truck-engineer")->parts[3],
		Spec::byName("truck-engineer")->parts[4],
		Spec::byName("truck-engineer")->parts[5],
		Spec::byName("truck-engineer")->parts[6],
	};
	spec->health = 100;
	spec->align = false;
	spec->vehicle = true;
	spec->energyConsume = Energy::kW(50);
	spec->consumeChemical = true;
	spec->store = true;
	spec->capacity = Mass::kg(5000);
	spec->storeSetUpper = true;
	spec->forceDelete = true;
	spec->costGreedy = 1.3;
	spec->clearance = 1.5;

	spec->materials = {
		{ Item::byName("electric-motor")->id, 2 },
		{ Item::byName("steel-sheet")->id, 2 },
		{ Item::byName("gear-wheel")->id, 2 },
	};

	tech = new Tech(Tech::next(), "truck-haulers");
	tech->tags = {"vehicles"};
	tech->cost = Currency::k(10);
	tech->licenseSpecs.insert(Spec::byName("truck-hauler"));

	spec = new Spec("truck-stop");
	spec->licensed = true;
	spec->health = 10;
	spec->collision = {0, 0, 0, 3, 0.1, 3};
	spec->selection = spec->collision;
	spec->parts = {
		(new Part(Thing("models/truck-stop.stl")))->paint(0x662222ff),
	};
	spec->pivot = true;
	spec->named = true;
	spec->vehicleStop = true;

	auto thingDroneChassis = Thing("models/drone-chassis-hd.stl", "models/drone-chassis-ld.stl");
	auto thingDroneSpars = Thing("models/drone-spars-hd.stl", "models/drone-spars-ld.stl");
	auto thingDroneRotor = Thing("models/drone-rotor-hd.stl", "models/drone-rotor-ld.stl");

	spec = new Spec("drone");
	spec->licensed = true;
	spec->select = false;
	spec->build = false;
	spec->health = 10;
	spec->collision = {0, 0, 0, 1, 1, 1};
	spec->selection = spec->collision;
	spec->parts = {
		(new Part(thingDroneChassis))->paint(0x990000ff),
		(new Part(thingDroneSpars))->paint(0x444444ff)->rotate(Point::Up, 45),
		(new PartSpinner(thingDroneRotor, 45))->paint(0x999999ff)->translate(0.3,0.02,0.3),
		(new PartSpinner(thingDroneRotor, 45))->paint(0x999999ff)->translate(0.3,0.02,-0.3),
		(new PartSpinner(thingDroneRotor, 45))->paint(0x999999ff)->translate(-0.3,0.02,0.3),
		(new PartSpinner(thingDroneRotor, 45))->paint(0x999999ff)->translate(-0.3,0.02,-0.3),
	};
	spec->align = false;
	spec->drone = true;
	spec->droneSpeed = 0.1f;
	spec->collideBuild = false;

	spec = new Spec("arm");
	spec->licensed = true;
	spec->health = 10;
	spec->collision = {0, 0, 0, 1, 2, 1};
	spec->selection = spec->collision;
	spec->arm = true;
	spec->enable = true;
	spec->armOffset = 1.0f;
	spec->armSpeed = 1.0f/60.0f;
	spec->rotate = true;
	spec->parts = {
		(new Part(Thing("models/arm-base-hd.stl", "models/arm-base-ld.stl")))->translate(0,-1.0,0)->paint(0x4169E1ff),
		(new Part(Thing("models/arm-pillar-hd.stl", "models/arm-pillar-ld.stl")))->translate(0,-1.0,0)->paint(0x4169E1ff),
		(new Part(Thing("models/arm-telescope1-hd.stl", "models/arm-telescope1-ld.stl")))->translate(0,-1.0,0)->paint(0xC0C0C0ff)->gloss(8),
		(new Part(Thing("models/arm-telescope2-hd.stl", "models/arm-telescope2-ld.stl")))->translate(0,-1.0,0)->paint(0xC0C0C0ff)->gloss(8),
		(new Part(Thing("models/arm-telescope3-hd.stl", "models/arm-telescope3-ld.stl")))->translate(0,-1.0,0)->paint(0xC0C0C0ff)->gloss(8),
		(new Part(Thing("models/arm-grip-hd.stl", "models/arm-grip-ld.stl")))->translate(0,-1.0,0)->paint(0x4169E1ff),
	};
	spec->consumeElectricity = true;
	spec->energyConsume = Energy::kW(10);
	spec->energyDrain = Energy::W(100);
	spec->materials = {
		{ Item::byName("steel-sheet")->id, 1 },
		{ Item::byName("circuit-board")->id, 1 },
	};

	// Arm states:
	// 0-359: rotation
	// 360-n: parking

	{
		Mat4 state0 = Mat4::identity;

		for (uint i = 0; i < 360; i++) {
			Mat4 state = Mat4::rotateY((float)i*DEG2RAD);

			float theta = (float)i;

			float a = sin(theta*DEG2RAD)*1.0;
			float b = cos(theta*DEG2RAD)*0.4;
			float r = 1.0*0.4 / std::sqrt(a*a + b*b);

			Point t1 = Point::North * (0.0f*r) - Point::North * 0.2f;
			Point t2 = Point::North * (0.3f*r) - Point::North * 0.2f;
			Point t3 = Point::North * (0.6f*r) - Point::North * 0.2f;
			Point g  = Point::North * (0.6f*r) + Point::North * 0.2f;

			Mat4 extend1 = Mat4::translate(t1.x, t1.y, t1.z) * state;
			Mat4 extend2 = Mat4::translate(t2.x, t2.y, t2.z) * state;
			Mat4 extend3 = Mat4::translate(t3.x, t3.y, t3.z) * state;
			Mat4 extendG = Mat4::translate(g.x, g.y, g.z) * state;

			spec->states.push_back({
				state0,
				state,
				extend1,
				extend2,
				extend3,
				extendG,
			});
		}
	}

	{
		Mat4 state0 = Mat4::identity;

		for (uint i = 0; i < 90; i+=10) {
			Mat4 state = state0;

			float theta = (float)i;

			float a = sin(theta*DEG2RAD)*1.0;
			float b = cos(theta*DEG2RAD)*0.1;
			float r = 1.0*0.1 / std::sqrt(a*a + b*b);

			Point t1 = Point::North * (0.0f*r) - Point::North * 0.2f;
			Point t2 = Point::North * (0.3f*r) - Point::North * 0.2f;
			Point t3 = Point::North * (0.6f*r) - Point::North * 0.2f;
			Point g  = Point::North * (0.6f*r) + Point::North * 0.2f;

			Mat4 extend1 = Mat4::translate(t1.x, t1.y, t1.z) * state;
			Mat4 extend2 = Mat4::translate(t2.x, t2.y, t2.z) * state;
			Mat4 extend3 = Mat4::translate(t3.x, t3.y, t3.z) * state;
			Mat4 extendG = Mat4::translate(g.x, g.y, g.z) * state;

			spec->states.push_back({
				state0,
				state,
				extend1,
				extend2,
				extend3,
				extendG,
			});
		}
	}

	spec = new Spec("long-arm");
	spec->licensed = true;
	spec->collision = {0, 0, 0, 1, 2, 1};
	spec->selection = spec->collision;
	spec->arm = true;
	spec->enable = true;
	spec->armOffset = 2.0f;
	spec->armSpeed =1.0f/60.0f;
	spec->rotate = true;
	spec->health = 10;
	spec->parts = {
		(new Part(Thing("models/arm-base-hd.stl", "models/arm-base-ld.stl")))->translate(0,-1.0,0)->paint(0xFF4500ff),
		(new Part(Thing("models/arm-pillar-hd.stl", "models/arm-pillar-ld.stl")))->translate(0,-1.0,0)->paint(0xFF4500ff),
		(new Part(Thing("models/arm-telescope1-hd.stl", "models/arm-telescope1-ld.stl")))->translate(0,-1.0,0)->paint(0xC0C0C0ff)->gloss(8),
		(new Part(Thing("models/arm-telescope2-hd.stl", "models/arm-telescope2-ld.stl")))->translate(0,-1.0,0)->paint(0xC0C0C0ff)->gloss(8),
		(new Part(Thing("models/arm-telescope3-hd.stl", "models/arm-telescope3-ld.stl")))->translate(0,-1.0,0)->paint(0xC0C0C0ff)->gloss(8),
		(new Part(Thing("models/arm-telescope4-hd.stl", "models/arm-telescope4-ld.stl")))->translate(0,-1.0,0)->paint(0xC0C0C0ff)->gloss(8),
		(new Part(Thing("models/arm-telescope5-hd.stl", "models/arm-telescope5-ld.stl")))->translate(0,-1.0,0)->paint(0xC0C0C0ff)->gloss(8),
		(new Part(Thing("models/arm-grip-hd.stl", "models/arm-grip-ld.stl")))->translate(0,-1.0,0)->paint(0xFF4500ff),
	};
	spec->consumeElectricity = true;
	spec->energyConsume = Energy::kW(10);
	spec->energyDrain = Energy::W(300);
	spec->materials = {
		{ Item::byName("steel-sheet")->id, 1 },
		{ Item::byName("circuit-board")->id, 1 },
	};

	// Arm states:
	// 0-359: rotation
	// 360-n: parking

	{
		Mat4 state0 = Mat4::identity;

		for (uint i = 0; i < 360; i++) {
			Mat4 state = Mat4::rotateY((float)i*DEG2RAD);

			float theta = (float)i;

			float a = sin(theta*DEG2RAD)*1.0;
			float b = cos(theta*DEG2RAD)*0.4;
			float r = 1.0*0.4 / std::sqrt(a*a + b*b);

			Point t1 = Point::North * (0.0f*r) - Point::North * 0.2f;
			Point t2 = Point::North * (0.4f*r) - Point::North * 0.2f;
			Point t3 = Point::North * (0.8f*r) - Point::North * 0.2f;
			Point t4 = Point::North * (1.2f*r) - Point::North * 0.2f;
			Point t5 = Point::North * (1.6f*r) - Point::North * 0.2f;
			Point g  = Point::North * (1.6f*r) + Point::North * 0.2f;

			Mat4 extend1 = Mat4::translate(t1.x, t1.y, t1.z) * state;
			Mat4 extend2 = Mat4::translate(t2.x, t2.y, t2.z) * state;
			Mat4 extend3 = Mat4::translate(t3.x, t3.y, t3.z) * state;
			Mat4 extend4 = Mat4::translate(t4.x, t4.y, t4.z) * state;
			Mat4 extend5 = Mat4::translate(t5.x, t5.y, t5.z) * state;
			Mat4 extendG = Mat4::translate(g.x, g.y, g.z) * state;

			spec->states.push_back({
				state0,
				state,
				extend1,
				extend2,
				extend3,
				extend4,
				extend5,
				extendG,
			});
		}
	}

	{
		Mat4 state0 = Mat4::identity;

		for (uint i = 0; i < 90; i+=10) {
			Mat4 state = state0;

			float theta = (float)i;

			float a = sin(theta*DEG2RAD)*1.0;
			float b = cos(theta*DEG2RAD)*0.1;
			float r = 1.0*0.1 / std::sqrt(a*a + b*b);

			Point t1 = Point::North * (0.0f*r) - Point::North * 0.2f;
			Point t2 = Point::North * (0.4f*r) - Point::North * 0.2f;
			Point t3 = Point::North * (0.8f*r) - Point::North * 0.2f;
			Point t4 = Point::North * (1.2f*r) - Point::North * 0.2f;
			Point t5 = Point::North * (1.6f*r) - Point::North * 0.2f;
			Point g  = Point::North * (1.6f*r) + Point::North * 0.2f;

			Mat4 extend1 = Mat4::translate(t1.x, t1.y, t1.z) * state;
			Mat4 extend2 = Mat4::translate(t2.x, t2.y, t2.z) * state;
			Mat4 extend3 = Mat4::translate(t3.x, t3.y, t3.z) * state;
			Mat4 extend4 = Mat4::translate(t4.x, t4.y, t4.z) * state;
			Mat4 extend5 = Mat4::translate(t5.x, t5.y, t5.z) * state;
			Mat4 extendG = Mat4::translate(g.x, g.y, g.z) * state;

			spec->states.push_back({
				state0,
				state,
				extend1,
				extend2,
				extend3,
				extend4,
				extend5,
				extendG,
			});
		}
	}

	auto steamEnginewheel = Thing("models/steam-engine-wheel-hd.stl", "models/steam-engine-wheel-ld.stl");

	spec = new Spec("boiler");
	spec->licensed = true;
	spec->collision = {0, 0, 0, 3, 2, 2};
	spec->selection = spec->collision;
	spec->pipe = true;
	spec->pipeCapacity = Liquid::l(1000);
	spec->pipeConnections = {
		{1.0f, -0.5f, 1.0f},
		{1.0f, -0.5f, -1.0f},
	};
	spec->pipeOutputConnections = {
		{-1.5f, -0.5f, 0.5f},
		{-1.5f, -0.5f, -0.5f},
	};

	spec->parts = {
		(new Part(Thing("models/boiler-chassis-hd.stl", "models/boiler-chassis-ld.stl")))->paint(0xB0C4DEff),
		(new Part(Thing("models/boiler-firebox-hd.stl", "models/boiler-firebox-ld.stl")))->gloss(16)->paint(0x666666ff),
		(new Part(Thing("models/boiler-stack-hd.stl", "models/boiler-stack-ld.stl")))->gloss(16)->paint(0xda8a67ff),
		(new Part(Thing("models/boiler-stack-cover-hd.stl", "models/boiler-stack-cover-ld.stl")))->paint(0x000000ff),
		(new PartSmoke(1200, 10, 0.005, 0.25f, 0.01f, 0.005f, 0.05f, 0.99f, 60, 180))->translate(0.6,1,0),
	};

	{
		auto thingRivet = Thing("models/boiler-rivet-hd.stl");

		for (int i = -2; i <= 2; i++) {
			Mat4 trx = Mat4::rotate(Point::South, -90.0f*DEG2RAD);
			trx = trx * Mat4::translate(Point::Up*0.5f);
			trx = trx * Mat4::rotate(Point::East, (float)i*30.0f*DEG2RAD);
			trx = trx * Mat4::translate(Point::East*1.31f);
			auto part = (new Part(thingRivet))->ld(false)->paint(0xB0C4DEff);
			part->transform = part->transform * trx;
			part->tsrt = part->transform * part->srt;
			spec->parts.push_back(part);
		}

		for (int i = -2; i <= 2; i++) {
			Mat4 trx = Mat4::rotate(Point::South, 90.0f*DEG2RAD);
			trx = trx * Mat4::translate(Point::Up*0.5f);
			trx = trx * Mat4::rotate(Point::East, (float)i*30.0f*DEG2RAD);
			trx = trx * Mat4::translate(Point::East*-1.31f);
			auto part = (new Part(thingRivet))->ld(false)->paint(0xB0C4DEff);
			part->transform = part->transform * trx;
			part->tsrt = part->transform * part->srt;
			spec->parts.push_back(part);
		}
	}

	for (uint i = 0; i < 10; i++) {
		float fi = (float)i;
		spec->states.push_back({
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::scale(0.1f*fi, 0.1f*fi, 0.1f*fi),
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
		});
	}

	for (uint i = 0; i < 80; i++) {
		spec->states.push_back({
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
		});
	}

	for (uint i = 0; i < 10; i++) {
		float fi = (float)(9-i);
		spec->states.push_back({
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::scale(0.1f*fi, 0.1f*fi, 0.1f*fi),
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
			Mat4::identity,
		});
	}

	spec->health = 10;
	spec->align = true;
	spec->rotate = true;
	spec->consumeChemical = true;
	spec->energyConsume = Energy::MW(2);
	spec->energyDrain = Energy::kW(60);
	spec->crafter = true;
	spec->crafterProgress = true;
	spec->recipeTags = {"boiling"};
	spec->materials = {
		{ Item::byName("brick")->id, 3 },
		{ Item::byName("copper-sheet")->id, 3 },
	};

	recipe = new Recipe(Recipe::next(), "boiling");
	recipe->energyUsage = Energy::MJ(20);
	recipe->tags = {"boiling"};
	recipe->inputFluids = {
		{ Fluid::byName("water")->id, 1000 },
	};
	recipe->outputFluids = {
		{ Fluid::byName("steam")->id, 1000 },
	};
	recipe->licensed = true;

	auto steamDroplet = new Part(droplet);
	steamDroplet->color = Fluid::byName("steam")->color;

	recipe->parts = {steamDroplet};

	spec = new Spec("steam-engine");
	spec->licensed = true;
	spec->collision = {0, 0, 0, 4, 4, 5};
	spec->selection = spec->collision;
	spec->electrical = { .area = Area(5,6), .rate = Energy::MW(1) };
	spec->pipe = true;
	spec->pipeCapacity = Liquid::l(100);
	spec->pipeConnections = {{-0.5f, -1.5f, 2.5f}, {0.5f, -1.5f, 2.5f}};
	spec->parts = {
		(new Part(Thing("models/steam-engine-boiler-hd.stl", "models/steam-engine-boiler-ld.stl")))->gloss(16)->paint(0x708090ff)->translate(0,-2,0),
		(new Part(Thing("models/steam-engine-saddle-hd.stl", "models/steam-engine-saddle-ld.stl")))->gloss(16)->paint(0x004225ff)->translate(0,-2,0),
		(new Part(Thing("models/steam-engine-foot-hd.stl", "models/steam-engine-foot-ld.stl")))->gloss(16)->paint(0x004225ff)->translate(0,-2,0),
		(new Part(Thing("models/steam-engine-axel-hd.stl", "models/steam-engine-axel-ld.stl")))->gloss(16)->paint(0x666666ff)->translate(0,1,1),
		(new Part(steamEnginewheel))->gloss(8)->paint(0x666666ff)->translate(-1.75,1,1)->rotate(Point::South, 90),
		(new Part(steamEnginewheel))->gloss(8)->paint(0x666666ff)->translate( 1.75,1,1)->rotate(Point::South, 90),
	};
	spec->health = 10;
	spec->align = true;
	spec->rotate = true;
	spec->enable = true;
	spec->consumeThermalFluid = true;
	spec->generateElectricity = true;
	spec->energyGenerate = Energy::MW(1);
	spec->materials = {
		{ Item::byName("steel-sheet")->id, 5 },
		{ Item::byName("copper-sheet")->id, 5 },
	};

	{
		Mat4 state0 = Mat4::identity;

		for (int i = 0; i < 720; i++) {
			Mat4 state1 = Mat4::rotateY((float)i*DEG2RAD*5.0f);

			spec->states.push_back({
				state0,
				state0,
				state0,
				state0,
				state1,
				state1,
			});
		}
	}

	spec = new Spec("turret");
	spec->health = 100;
	spec->collision = {0, 0, 0, 1, 1, 1};
	spec->selection = spec->collision;
	spec->parts = {
		(new Part(Thing("models/turret-chassis.stl")))->paint(0x444444ff)->translate(0,-0.5,0),
		(new Part(Thing("models/turret-dome.stl")))->paint(0x0044ccff)->translate(0,-0.5,0)->pivots(),
		(new Part(Thing("models/turret-barrel.stl")))->paint(0x444444ff)->translate(0,-0.5,0)->pivots(),
	};
	spec->align = true;
	spec->rotate = true;
	spec->turret = true;
	spec->turretRange = 50;
	spec->turretPivot = 0.1;
	spec->turretCooldown = 10;
	spec->turretBulletSpec = "bullet";

	spec = new Spec("bullet");
	spec->build = false;
	spec->explodes = true;
	spec->explosionSpec = "bullet-impact1";
	spec->health = 0;
	spec->collision = {0, 0, 0, 0.1, 0.1, 0.1};
	spec->selection = spec->collision;
	spec->parts = {
		(new Part(Thing("models/bullet.stl")))->paint(0x660000ff),
	};
	spec->align = false;
	spec->missile = true;
	spec->missileSpeed = 1.0;
	spec->missileBallistic = true;

	spec = new Spec("bullet-impact1");
	spec->build = false;
	spec->align = false;
	spec->explosion = true;
	spec->explosionDamage = 10;
	spec->explosionRadius = 0.1;
	spec->explosionRate = 0.01;

	tech = new Tech(Tech::next(), "turrets");
	tech->tags = {"defence"};
	tech->cost = Currency::k(10);
	tech->licenseSpecs.insert(Spec::byName("turret"));

	spec = new Spec("missile");
	spec->explodes = true;
	spec->explosionSpec = "missile-explosion1";
	spec->health = 100;
	spec->collision = {0, 0, 0, 1, 1, 2};
	spec->selection = spec->collision;
	spec->parts = {
		(new Part(Thing("models/missile-chassis.stl")))->paint(0x660000ff),
	};
	spec->align = false;
	spec->missile = true;

	spec = new Spec("missile-explosion1");
	spec->align = false;
	spec->build = false;
	spec->explosion = true;
	spec->explosionDamage = 100;
	spec->explosionRadius = 10;
	spec->explosionRate = 0.5;

	tech = new Tech(Tech::next(), "missiles");
	tech->tags = {"defence"};
	tech->cost = Currency::k(10);
	tech->licenseSpecs.insert(Spec::byName("missile"));

	spec = new Spec("teleporter");
	spec->licensed = true;
	spec->health = 10;
	spec->store = true;
	spec->capacity = Mass::kg(10000);
	spec->loadPriority = true;
	spec->rotate = true;
	spec->crafter = true;
	spec->crafterProgress = false;
	spec->enable = true;
	spec->recipeTags = {"teleporting"};
	spec->consumeElectricity = true;
	spec->energyConsume = Energy::MW(10);
	spec->energyDrain = Energy::kW(300);
	spec->collision = {0, 0, 0, 8, 8, 8};
	spec->selection = spec->collision;
	spec->parts = {
		(new Part(Thing("models/teleporter-base-hd.stl")))->paint(0x4682B4ff)->translate(0,-2.5,0),
		(new Part(Thing("models/teleporter-ring1-hd.stl")))->paint(0xFFD700ff)->gloss(8),
		(new Part(Thing("models/teleporter-ring2-hd.stl")))->paint(0xFFD700ff)->gloss(8),
		(new Part(Thing("models/teleporter-ring3-hd.stl")))->paint(0xFFD700ff)->gloss(8),
	};
	spec->materials = {
		{ Item::byName("steel-sheet")->id, 5 },
	};

	{
		Mat4 state0 = Mat4::identity;

		for (int i = 0; i < 360; i+=2) {
			Mat4 state1 = Mat4::rotateY((float)i*DEG2RAD);
			Mat4 state2 = Mat4::rotateX((float)i*DEG2RAD);
			Mat4 state3 = Mat4::rotateZ((float)i*DEG2RAD);

			spec->states.push_back({
				state0,
				state1,
				state2,
				state3,
			});
		}
	}

	spec = new Spec("computer");
	spec->health = 10;
	spec->rotate = true;
	spec->computer = true;
	spec->consumeElectricity = true;
	spec->energyDrain = Energy::W(100);
	spec->collision = {0, 0, 0, 1, 2, 1};
	spec->selection = spec->collision;
	spec->parts = {
		(new Part(Thing("models/computer-rack-hd.stl", "models/computer-rack-ld.stl")))->paint(0x888888ff)->translate(0,-1,0),
	};
	spec->materials = {
		{ Item::byName("steel-sheet")->id, 1 },
		{ Item::byName("copper-wire")->id, 10 },
		{ Item::byName("circuit-board")->id, 10 },
	};

	tech = new Tech(Tech::next(), "computers");
	tech->tags = {"computation"};
	tech->cost = Currency::k(10);
	tech->licenseSpecs.insert(Spec::byName("computer"));

	spec = new Spec("projector");
	spec->health = 10;
	spec->projector = true;
	spec->collision = {0, 0, 0, 1, 0.1, 1};
	spec->selection = spec->collision;
	spec->parts = {
		(new Part(Thing("models/projector.stl")))->paint(0x888888ff),
		(new PartSmoke(1000, 100, 0.0025, 0.25f, 0.05f, 0.005f, 0.1f, 0.99f, 5, 10))->paint(0xeeeeeeff),
	};

	recipe = new Recipe(Recipe::next(), "sell-steel-ingots1");
	recipe->energyUsage = Energy::MJ(100);
	recipe->tags = {"teleporting"};
	recipe->inputItems = {
		{ Item::byName("steel-ingot")->id, 1000 },
	};
	recipe->outputCurrency = 1000;
	recipe->parts = Item::byName("steel-ingot")->parts;
	recipe->licensed = true;

	recipe = new Recipe(Recipe::next(), "sell-steel-sheet1");
	recipe->energyUsage = Energy::MJ(100);
	recipe->tags = {"teleporting"};
	recipe->inputItems = {
		{ Item::byName("steel-sheet")->id, 1000 },
	};
	recipe->outputCurrency = 1000;
	recipe->parts = Item::byName("steel-sheet")->parts;
	recipe->licensed = true;

	recipe = new Recipe(Recipe::next(), "sell-copper-ingots1");
	recipe->energyUsage = Energy::MJ(100);
	recipe->tags = {"teleporting"};
	recipe->inputItems = {
		{ Item::byName("copper-ingot")->id, 1000 },
	};
	recipe->outputCurrency = 1000;
	recipe->parts = Item::byName("copper-ingot")->parts;
	recipe->licensed = true;

	recipe = new Recipe(Recipe::next(), "sell-copper-sheet1");
	recipe->energyUsage = Energy::MJ(100);
	recipe->tags = {"teleporting"};
	recipe->inputItems = {
		{ Item::byName("copper-sheet")->id, 1000 },
	};
	recipe->outputCurrency = 1000;
	recipe->parts = Item::byName("copper-sheet")->parts;
	recipe->licensed = true;
}

int main(int argc, char const *argv[]) {
	//nvidia-settings --query=fsaa --verbose
	//putenv((char*)"__GL_FSAA_MODE=9");

	bool loadSave = true;

	for (int i = 1; i < argc; i++) {
		auto arg = std::string(argv[i]);

		if (arg == "--new") {
			loadSave = false;
			continue;
		}

		fatalf("unexpected argument: %s", arg.c_str());
	}

	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_WINDOW_ALWAYS_RUN|FLAG_MSAA_4X_HINT|FLAG_VSYNC_HINT);
	InitWindow(1920,1080,"factropy");
	//SetTargetFPS(60);
	SetExitKey(0);

	SiteCamera *camSec = new SiteCamera(
		{50,50,50},
		{-1,-1,-1}
	);

	MainCamera *camera = new MainCamera(
		{-50,50,-50},
		{1,-1,1}
	);

  //float fogDensity = 0.004f;
  float fogDensity = 0.003f;
	Vector4 fogColor = ColorNormalize(SKYBLUE);
	Vector4 ambient = { 0.6f, 0.6f, 0.3f, 1.0f };

	Shader shader = LoadShader(
		FormatText("shaders/glsl%i/base_lighting.vs", GLSL_VERSION),
		FormatText("shaders/glsl%i/fog.fs", GLSL_VERSION)
	);

	shader.locs[LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
	shader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
	shader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

  SetShaderValue(shader, GetShaderLocation(shader, "fogDensity"), &fogDensity, UNIFORM_FLOAT);
  SetShaderValue(shader, GetShaderLocation(shader, "fogColor"), &fogColor, UNIFORM_VEC4);
	SetShaderValue(shader, GetShaderLocation(shader, "ambient"), &ambient, UNIFORM_VEC4);

	Shader pshader = LoadShader(
		FormatText("shaders/glsl%i/base_lighting_instanced.vs", GLSL_VERSION),
		FormatText("shaders/glsl%i/fog.fs", GLSL_VERSION)
	);

	pshader.locs[LOC_MATRIX_MVP] = GetShaderLocation(pshader, "mvp");
	pshader.locs[LOC_MATRIX_MODEL] = GetShaderLocationAttrib(pshader, "instance");
	pshader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(pshader, "viewPos");

  SetShaderValue(pshader, GetShaderLocation(pshader, "fogDensity"), &fogDensity, UNIFORM_FLOAT);
  SetShaderValue(pshader, GetShaderLocation(pshader, "fogColor"), &fogColor, UNIFORM_VEC4);
	SetShaderValue(pshader, GetShaderLocation(pshader, "ambient"), &ambient, UNIFORM_VEC4);

	Shader gshader = LoadShader(
		FormatText("shaders/glsl%i/base_lighting_instanced.vs", GLSL_VERSION),
		FormatText("shaders/glsl%i/fog_ghosts.fs", GLSL_VERSION)
	);

	gshader.locs[LOC_MATRIX_MVP] = GetShaderLocation(gshader, "mvp");
	gshader.locs[LOC_MATRIX_MODEL] = GetShaderLocationAttrib(gshader, "instance");
	gshader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(gshader, "viewPos");

  SetShaderValue(gshader, GetShaderLocation(gshader, "fogDensity"), &fogDensity, UNIFORM_FLOAT);
  SetShaderValue(gshader, GetShaderLocation(gshader, "fogColor"), &fogColor, UNIFORM_VEC4);
	SetShaderValue(gshader, GetShaderLocation(gshader, "ambient"), &ambient, UNIFORM_VEC4);

	Part::shader = pshader;
	Part::ghostShader = gshader;
	Part::material = LoadMaterialDefault();

	Shader particleShader = LoadShader(
		FormatText("shaders/glsl%i/base_lighting_particles.vs", GLSL_VERSION),
		FormatText("shaders/glsl%i/fog.fs", GLSL_VERSION)
	);

	particleShader.locs[LOC_MATRIX_MVP] = GetShaderLocation(particleShader, "mvp");
	particleShader.locs[LOC_MATRIX_MODEL] = GetShaderLocationAttrib(particleShader, "particle");
	particleShader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(particleShader, "viewPos");

  SetShaderValue(particleShader, GetShaderLocation(particleShader, "fogDensity"), &fogDensity, UNIFORM_FLOAT);
  SetShaderValue(particleShader, GetShaderLocation(particleShader, "fogColor"), &fogColor, UNIFORM_VEC4);
	SetShaderValue(particleShader, GetShaderLocation(particleShader, "ambient"), &ambient, UNIFORM_VEC4);

	Part::particleShader = particleShader;

	Part::shaderSpecShine = GetShaderLocation(Part::shader, "specShine");
	Part::particleShaderSpecShine = GetShaderLocation(Part::particleShader, "specShine");

	/*Light lightA =*/ CreateLight(LIGHT_DIRECTIONAL, Point(-1, 1, 0), Point::Zero, WHITE, shader);
	/*Light lightB =*/ CreateLight(LIGHT_DIRECTIONAL, Point(-1, 1, 0), Point::Zero, WHITE, pshader);
	/*Light lightB =*/ CreateLight(LIGHT_DIRECTIONAL, Point(-1, 1, 0), Point::Zero, WHITE, particleShader);

	Model cube = LoadModelFromMesh(GenMeshCube(1.0f,1.0f,1.0f));
	cube.materials[0].shader = shader;

	View::waterCube = LoadModelFromMesh(GenMeshCube(1.0f,1.0f,1.0f));
	View::waterCube.materials[0].shader = pshader;
	View::waterCube.materials[0].maps[MAP_DIFFUSE].color = GetColorSRGB(0x010190FF);

	View::redCube = LoadModelFromMesh(GenMeshCube(0.5f,0.5f,0.5f));
	View::redCube.materials[0].shader = pshader;
	View::redCube.materials[0].maps[MAP_DIFFUSE].color = GetColorSRGB(0xff0000FF);

	View::greenCube = LoadModelFromMesh(GenMeshCube(0.5f,0.5f,0.5f));
	View::greenCube.materials[0].shader = pshader;
	View::greenCube.materials[0].maps[MAP_DIFFUSE].color = GetColorSRGB(0x00ff00FF);

	View::gridSquareLand = (new Part(Thing("models/grid-square.stl")))->scale(0.001, 0.001, 0.001)->paint(0xccccccFF);

	Chunk::material = LoadMaterialDefault();
	Chunk::material.shader = shader;
	Chunk::material.maps[MAP_DIFFUSE].color = WHITE;

	Mod* mod = new ModDuktape("base");
	mod->load();

	scenario();

	Popup* popup = nullptr;
	StatsPopup2* statsPopup = new StatsPopup2(camera);
	EntityPopup2* entityPopup = new EntityPopup2(camera);
	RecipePopup* recipePopup = new RecipePopup(camera);

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)rlWindowHandle(), true);
	ImGui_ImplOpenGL3_Init(nullptr);

	ImPlot::CreateContext();

	auto& imGuiIO = ImGui::GetIO();
	auto& imGuiStyle = ImGui::GetStyle();

	imGuiStyle.WindowRounding = 0.0f;

	//auto font =
	imGuiIO.Fonts->AddFontFromFileTTF("font/Roboto-Regular.ttf", 24);

	imGuiIO.IniFilename = nullptr;

	//imGuiStyle.ScaleAllSizes(1.5f);
	//imGuiIO.FontGlobalScale = 1.5f;

	MessagePopup *status = new MessagePopup(camera);
	status->text = std::string(quotes[(uint)std::time(nullptr)%(sizeof(quotes)/sizeof(char*))]);

	std::function<void(void)> loadingScreen = [&]() {
		ensure(!WindowShouldClose());

		BeginDrawing();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ClearBackground(BLACK);

			int x = 0, y = 0;

			std::function<void(void)> advance = [&]() {
				x += 128;
				if (x > GetScreenWidth()-128) {
					y += 128;
					x = 0;
				}
				if (y > GetScreenHeight()-128) {
					y = 0;
				}
			};

			for (auto [_,item]: Item::names) {
				if (!item->texture.id) break;
				DrawTextureEx(item->texture.texture, (Vector2){(float)x, (float)y}, 0.0f, 1.0, WHITE);
				advance();
			}

			for (auto [_,fluid]: Fluid::names) {
				if (!fluid->texture.id) break;
				DrawTextureEx(fluid->texture.texture, (Vector2){(float)x, (float)y}, 0.0f, 1.0, WHITE);
				advance();
			}

			for (auto [_,recipe]: Recipe::names) {
				if (!recipe->texture.id) break;
				DrawTextureEx(recipe->texture.texture, (Vector2){(float)x, (float)y}, 0.0f, 0.5, WHITE);
				advance();
			}

			for (auto [_,spec]: Spec::all) {
				if (!spec->texture.id) break;
				DrawTextureEx(spec->texture.texture, (Vector2){(float)x, (float)y}, 0.0f, 0.5, WHITE);
				advance();
			}

			status->draw();

			rlglDraw();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		EndDrawing();
	};

	loadingScreen();

	for (auto [name,item]: Item::names) {

		item->texture = LoadRenderTexture(128, 128);

		BeginTextureMode(item->texture);

			ClearBackground(GetColor(0x0));

			BeginMode3D((Camera3D){
				.position = (Vector3){0.9,1,0.9},
				.target =   (Vector3){0,0.25,0},
				.up =       -Point::Up,
				.fovy =     45.0f,
				.type =     CAMERA_PERSPECTIVE,
			});

			for (uint i = 0; i < item->parts.size(); i++) {
				item->parts[i]->draw(Mat4::identity);
			}

			EndMode3D();

		EndTextureMode();

		item->image = GetTextureData(item->texture.texture);
		loadingScreen();
	}

	loadingScreen();

	auto droplet = Thing("models/fluid.stl");

	for (auto [name,fluid]: Fluid::names) {

		fluid->texture = LoadRenderTexture(128, 128);

		BeginTextureMode(fluid->texture);

			ClearBackground(GetColor(0x0));

			BeginMode3D((Camera3D){
				.position = (Vector3){0.9,1,0.9},
				.target =   (Vector3){0,0.25,0},
				.up =       -Point::Up,
				.fovy =     45.0f,
				.type =     CAMERA_PERSPECTIVE,
			});

			auto drop = new Part(droplet);
			drop->color = fluid->color;
			drop->draw(Mat4::identity);
			fluid->droplet = drop;

			EndMode3D();

		EndTextureMode();

		fluid->image = GetTextureData(fluid->texture.texture);
		loadingScreen();
	}

	loadingScreen();

	for (auto [name,recipe]: Recipe::names) {
		notef("rendering recipe %s", name);
		recipe->texture = LoadRenderTexture(256, 256);

		BeginTextureMode(recipe->texture);

			ClearBackground(GetColor(0x0));

			BeginMode3D((Camera3D){
				.position = (Vector3){0.9,1,0.9},
				.target =   (Vector3){0,0.25,0},
				.up =       -Point::Up,
				.fovy =     45.0f,
				.type =     CAMERA_PERSPECTIVE,
			});

			for (Part* part: recipe->parts) {
				part->draw(Mat4::identity);
			}

			EndMode3D();

		EndTextureMode();

		recipe->image = GetTextureData(recipe->texture.texture);
		loadingScreen();
	}

	loadingScreen();

	for (auto [name,spec]: Spec::all) {
		notef("rendering spec %s", name);
		spec->texture = LoadRenderTexture(256, 256);

		BeginTextureMode(spec->texture);

			ClearBackground(GetColor(0x0));

			BeginMode3D((Camera3D){
				.position = Point(1.0f, 1.0f, 1.0f) * spec->collision.dimensions().length(),
				.target =   Point(0.0f, spec->collision.h/4.0f, 0.0f),
				.up =       -Point::Up,
				.fovy =     45.0f,
				.type =     CAMERA_PERSPECTIVE,
			});

			for (Part* part: spec->parts) {
				part->draw(Mat4::identity);
			}

			EndMode3D();

		EndTextureMode();

		spec->image = GetTextureData(spec->texture.texture);
		loadingScreen();
	}

	loadingScreen();

	if (loadSave && std::filesystem::exists("autosave")) {

		Sim::load("autosave");

	}	else {

		int horizon = 4;
		for (int cy = -horizon; cy < horizon; cy++) {
			for (int cx = -horizon; cx < horizon; cx++) {
				loadingScreen();
				Chunk::request(cx,cy);
			}
		}

		Entity& en = Entity::create(Entity::next(), Spec::byName("truck-engineer")).floor(0).materialize();

		en.store().insert({Item::byName("coal")->id, 50});
		en.store().insert({Item::byName("steel-sheet")->id, 100});
		en.store().insert({Item::byName("copper-sheet")->id, 100});
		en.store().insert({Item::byName("brick")->id, 50});
		en.store().insert({Item::byName("circuit-board")->id, 50});
		en.store().insert({Item::byName("gear-wheel")->id, 100});
		en.store().insert({Item::byName("pipe")->id, 50});

		for (auto stack: en.store().stacks) {
			en.store().levelSet(stack.iid, stack.size, stack.size);
		}

		camera->directing = new GuiEntity(en.id);
	}

	RenderTexture secondary = LoadRenderTexture(400-imGuiStyle.WindowPadding.x*2, 270-imGuiStyle.WindowPadding.y*2);

	std::atomic<bool> running = true;

	auto chunkGenerator = std::thread([&]() {
		Sim::reseedThread();

		while (running) {

			std::vector<Chunk*> generate;

			Chunk::mutex.lock();
			for (auto& xy: Chunk::requested) {
				ensure(!Chunk::all.count(xy));
				generate.push_back(new Chunk(xy.x, xy.y));
			}
			Chunk::requested.clear();
			Chunk::mutex.unlock();

			for (auto& chunk: generate) {
				chunk->generate();
			}

			std::vector<Chunk*> check;

			Sim::locked([&]() {
				for (auto& chunk: generate) {
					chunk->findHills();
					chunk->generated = true;
					chunk->regenerate();
					for (auto& fn: Chunk::generators) {
						fn(chunk);
					}
				}
				for (auto& [_,chunk]: Chunk::all) {
					check.push_back(chunk);
				}
			});

			for (auto& chunk: check) {
				chunk->backgroundgenerate();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	});

	while (!WindowShouldClose()) {
		Sim::locked([&]() {
			Sim::update();
			mod->update();

			for (auto& [_,chunk]: Chunk::all)
				chunk->autoload();
		});

		if (popup && !popup->visible) {
			popup = nullptr;
		}

		bool popupVisible = popup && popup->visible;
		bool popupFocused = popup && (popup->mouseOver || popup->inputFocused);

		bool worldFocused = !popupVisible && !popupFocused;

		camera->update(worldFocused);
		camSec->update(worldFocused);

		if (worldFocused) {

			if (IsKeyReleased(KEY_F9)) {
				camSec->pos = camera->position;
				camSec->dir = camera->direction;
			}

			if (IsKeyReleased(KEY_Q)) {
				if (camera->placing) delete camera->placing;
				camera->placing = nullptr;

				if (camera->selecting && camera->selected.size()) {
					Sim::locked([&]() {
						for (auto se: camera->selected) {
							if (se->spec->junk) continue;

							if (Entity::exists(se->id)) {
								Entity& en = Entity::get(se->id);
								auto ge = new GuiFakeEntity(se->spec);
								ge->dir = en.dir;
								ge->move(en.pos);
								ge->getConfig(en);

								if (!camera->placing)
									camera->placing = new Plan(camera->selected[0]->pos);
								camera->placing->add(ge);
							}
						}
					});
					camera->selection = {Point::Zero, Point::Zero};
					camera->selecting = false;
				}
				else
				if (camera->hovering && !camera->hovering->spec->junk) {
					Sim::locked([&]() {
						auto se = camera->hovering;
						if (Entity::exists(se->id)) {
							Entity& en = Entity::get(se->id);
							auto ge = new GuiFakeEntity(se->spec->pipette ? se->spec->pipette: se->spec);
							ge->dir = en.dir;
							if (en.spec->pipette && en.spec->conveyor) {
								ge->dir = en.spec->conveyorOutput.transform(en.dir.rotation());
							}
							ge->move(en.pos);
							//ge->getConfig(en);

							if (!camera->placing)
								camera->placing = new Plan(camera->hovering->pos);
							camera->placing->add(ge);
						}
					});
				}
			}

			if (IsKeyReleased(KEY_R)) {
				if (camera->placing) {
					camera->placing->rotate();
				}
				else
				if (camera->hovering) {
					Sim::locked([&]() {
						if (Entity::exists(camera->hovering->id)) {
							Entity::get(camera->hovering->id)
								.rotate();
						}
					});
				}
			}

			if (IsKeyReleased(KEY_C)) {
				if (camera->placing) {
					camera->placing->cycle();
				}
			}

			if (IsKeyReleased(KEY_T)) {
				if (camera->hovering) {
					Sim::locked([&]() {
						if (Entity::exists(camera->hovering->id)) {
							Entity::get(camera->hovering->id)
								.toggle();
						}
					});
				}
			}

			if (IsKeyReleased(KEY_F)) {
				if (camera->hovering && camera->hovering->spec->pipe) {
					Sim::locked([&]() {
						if (Entity::exists(camera->hovering->id)) {
							Pipe::get(camera->hovering->id).flush();
						}
					});
				}
			}

			if (IsKeyReleased(KEY_O)) {
				if (camera->selecting) {
					Sim::locked([&]() {
						for (auto se: camera->selected) {
							if (Entity::exists(se->id)) {
								auto& en = Entity::get(se->id);
								en.setEnabled(!en.isEnabled());
							}
						}
					});
				}
			}

			if (IsKeyReleased(KEY_G)) {
				camera->showGrid = !camera->showGrid;
			}

			if (camera->mouse.left.clicked && IsKeyDown(KEY_LEFT_CONTROL)) {
				if (camera->hovering && camera->hovering->spec->vehicle) {
					delete camera->directing;
					camera->directing = new GuiEntity(camera->hovering->id);
				}
			}

			if (camera->hovering && camera->hovering->spec->ropeway && IsKeyReleased(KEY_J)) {
				delete camera->connecting;
				camera->connecting = new GuiEntity(camera->hovering->id);
			}

			if (camera->mouse.right.clicked && IsKeyDown(KEY_LEFT_CONTROL)) {
				Sim::locked([&]() {
					if (camera->directing && camera->directing->spec->vehicle && Entity::exists(camera->directing->id)) {
						RayHitInfo spot = GetCollisionRayGround(camera->mouse.ray, 0);
						Entity::get(camera->directing->id).vehicle().addWaypoint(Point(spot.position));
					}
				});
			}

			if (camera->mouse.left.down && camera->placing && !IsKeyDown(KEY_LEFT_CONTROL)) {
				bool force = IsKeyDown(KEY_LEFT_SHIFT);
				Sim::locked([&]() {
					if (force || camera->placing->fits()) {
						for (uint i = 0; i < camera->placing->entities.size(); i++) {
							auto te = camera->placing->entities[i];
							// Plan will fit over existing entities in the right positions, but don't double up
							if (Entity::fits(te->spec, te->pos, te->dir)) {
								Entity& en = Entity::create(Entity::next(), te->spec)
									.construct()
									.look(te->dir)
									.move(te->pos);
								te->setConfig(en);

								if (camera->connecting && camera->connecting->connectable(te)) {
									en.ropeway().connect(camera->connecting->id);
									delete camera->connecting;
									camera->connecting = new GuiEntity(en.id);
								}

							} else {
								uint eid = Entity::at(te->pos);
								if (eid) {
									Entity& en = Entity::get(eid);
									if (en.spec == te->spec) {
										en.look(te->dir);
										te->setConfig(en);
									}
								}
							}
						}
						if (camera->placing->entities.size() == 1) {
							auto ge = camera->placing->entities[0];

							if (ge->spec->pipette && ge->spec->conveyor) {
								auto pe = new GuiFakeEntity(ge->spec->pipette);
								pe->dir = ge->spec->conveyorOutput.transform(ge->dir.rotation());
								pe->move(ge->pos);
								camera->placing->entities[0] = pe;
								delete ge;
							}
						}
					}
				});
			}

			if (camera->mouse.left.clicked
				&& !IsKeyDown(KEY_LEFT_CONTROL)
				&& !camera->placing
				&& camera->hovering
				&& camera->hovering->spec->operable()
			){
				if (popup) popup->show(false);
				popup = entityPopup;
				popup->show(true);
				entityPopup->useEntity(camera->hovering->id);
			}

			if (IsKeyReleased(KEY_DELETE)) {
				if (camera->selected.size()) {
					Sim::locked([&]() {
						for (auto te: camera->selected) {
							uint id = te->id;
							if (!Entity::exists(id)) continue;
							if (te->spec->forceDelete && !IsKeyDown(KEY_LEFT_SHIFT)) continue;
							if (camera->directing && camera->directing->id == id) continue;
							Entity::get(id).deconstruct();
						}
					});
					camera->selection = {Point::Zero, Point::Zero};
					camera->selecting = false;
				}
				if (camera->hovering) {
					Sim::locked([&]() {
						int id = camera->hovering->id;
						if (Entity::exists(id)) {
							Entity::get(id).deconstruct();
						}
					});
				}
			}

			if (camera->hovering && IsKeyReleased(KEY_F11)) {
				Sim::locked([&]() {
					int id = camera->hovering->id;
					if (Entity::exists(id)) {
						Entity& en = Entity::get(id);
						if (en.spec->conveyor) {
							en.conveyor().insert(Item::byName("pipe")->id);
						}
					}
				});
			}

			if (IsKeyReleased(KEY_F10)) {
				Sim::locked([&]() {
					Point groundZero = camera->groundTarget(0.0f);
					Chunk* chunk = Chunk::tryGet(groundZero);
					if (chunk) chunk->regenerate();
				});
			}
		}

		if (IsKeyReleased(KEY_F1)) {
			if (popup == statsPopup) {
				popup->show(false);
				popup = nullptr;
			} else {
				if (popup) popup->show(false);
				popup = statsPopup;
				popup->show(true);
			}
		}

		if (IsKeyReleased(KEY_E)) {
			[&]() {
				if (popup && popup != recipePopup) return;
				if (popup && popup->inputFocused) return;

				if (popup) {
					popup->show(false);
					popup = nullptr;
					return;
				}

				if (!popup) {
					popup = recipePopup;
					popup->show(true);
					return;
				}
			}();
		}

		if (IsKeyReleased(KEY_ESCAPE)) {
			if (popup) {
				popup->show(false);
				popup = nullptr;
			}
			else
			if (camera->placing) {
				delete camera->placing;
				camera->placing = nullptr;
			}
			else
			if (camera->selecting) {
				camera->selection = {Point::Zero, Point::Zero};
				camera->selecting = false;
			}
			else
			if (camera->directing) {
				delete camera->directing;
				camera->directing = nullptr;
			}
		}

		if (IsKeyReleased(KEY_F5)) {
			Sim::save("autosave");
		}

		for (auto part: Part::all) {
			part->update();
		}

		BeginDrawing();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			Point cameraTarget = camSec->pos;
			SetShaderValue(shader, shader.locs[LOC_VECTOR_VIEW], &cameraTarget.x, UNIFORM_VEC3);
			SetShaderValue(pshader, pshader.locs[LOC_VECTOR_VIEW], &cameraTarget.x, UNIFORM_VEC3);
			SetShaderValue(gshader, gshader.locs[LOC_VECTOR_VIEW], &cameraTarget.x, UNIFORM_VEC3);
			SetShaderValue(particleShader, particleShader.locs[LOC_VECTOR_VIEW], &cameraTarget.x, UNIFORM_VEC3);

			camSec->draw(secondary);

			cameraTarget = camera->position;
			SetShaderValue(shader, shader.locs[LOC_VECTOR_VIEW], &cameraTarget.x, UNIFORM_VEC3);
			SetShaderValue(pshader, pshader.locs[LOC_VECTOR_VIEW], &cameraTarget.x, UNIFORM_VEC3);
			SetShaderValue(gshader, gshader.locs[LOC_VECTOR_VIEW], &cameraTarget.x, UNIFORM_VEC3);
			SetShaderValue(particleShader, particleShader.locs[LOC_VECTOR_VIEW], &cameraTarget.x, UNIFORM_VEC3);

			camera->draw();

			ImGui::Begin("hudV", nullptr, ImGuiWindowFlags_NoDecoration);
			ImGui::SetWindowSize({(float)400,0.0f}, ImGuiCond_Always);
			ImGui::SetWindowPos({(float)(GetScreenWidth()-400),0}, ImGuiCond_Always);

			ImGui::Print(fmt("%d FPS, %u meshes", GetFPS(), camera->objects));

			ImGui::Image(secondary.texture.id, ImVec2(secondary.texture.width, secondary.texture.height), ImVec2(0,1), ImVec2(1,0));

			ImGui::Print("Electricity Network Load"); ImGui::SameLine();
			ImGui::PrintRight(Entity::electricityDemand.formatRate());
			ImGui::OverflowBar(Entity::electricitySupply.portion(Entity::electricityCapacityReady));

			ImGui::Print(fmtc("electricityDemand %s", Entity::electricityDemand.formatRate()));
			ImGui::Print(fmtc("electricitySupply %s", Entity::electricitySupply.formatRate()));
			ImGui::Print(fmtc("electricityCapacity %s", Entity::electricityCapacity.formatRate()));
			ImGui::Print(fmtc("electricityCapacityReady %s", Entity::electricityCapacityReady.formatRate()));

			ImGui::Print(fmt("Ledger::balance %s", Ledger::balance.format()));

			if (camera->hovering) {
				GuiEntity* ge = camera->hovering;

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Print(ge->spec->name);

				ImGui::GetWindowDrawList()->AddRectFilled(
					ImGui::GetCursorScreenPos(),
					ImVec2(ImGui::GetCursorScreenPos().x + ImGui::GetWindowContentRegionWidth(), ImGui::GetCursorScreenPos().y + ge->spec->texture.texture.height),
					ImColor(0.5f, 0.5f, 0.5f, 1.0f)
				);

				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - (float)ge->spec->texture.texture.width) * 0.5f);
				ImGui::Image(ge->spec->texture.texture.id,
					ImVec2(ge->spec->texture.texture.width, ge->spec->texture.texture.height),
					ImVec2(0,0)
				);

				ImGui::Print(fmtc("Health: %d/%d", ge->health, ge->spec->health));
				ImGui::LevelBar(std::max(0.01, ge->spec->health ? (float)ge->health / (float)ge->spec->health: 1.0));

				if (ge->spec->consumeChemical) {
					ImGui::Print("Fuel");
					ImGui::LevelBar(ge->burner.energy.portion(ge->burner.buffer));
				}

				if (ge->spec->consumeElectricity) {
					if (ge->spec->energyConsume && ge->spec->energyDrain) {
						ImGui::Print(fmtc("Electricity: %s (%s)",
							ge->spec->energyConsume.formatRate(),
							ge->spec->energyDrain.formatRate())
						);
					}
					else
					if (ge->spec->energyConsume) {
						ImGui::Print(fmtc("Electricity: %s",
							ge->spec->energyConsume.formatRate())
						);
					}
					else
					if (ge->spec->energyDrain) {
						ImGui::Print(fmtc("Electricity: %s",
							ge->spec->energyDrain.formatRate())
						);
					}
				}

				if (ge->spec->crafter) {
					Recipe* recipe = ge->crafter.recipe;

					if (ge->spec->recipeTags.count("mining")) {
						if (recipe && recipe->mine) {
							ImGui::Print(fmtc("Mining: %s(%d) %0.1f", Item::get(recipe->mine)->name, Chunk::countMine(ge->miningBox(), recipe->mine), Recipe::miningRate));
						} else {
							ImGui::Print("Mining: (nothing)");
						}
						ImGui::LevelBar(ge->crafter.progress);

						for (Stack stack: Chunk::minables(ge->box())) {
							ImGui::Print(fmtc("%s(%d)", Item::get(stack.iid)->name, stack.size));
						}
					}

					if (ge->spec->recipeTags.count("smelting")) {
						ImGui::Print(fmtc("Smelting: %s", recipe && recipe->tags.count("smelting") ? recipe->name: "(nothing)"));
						ImGui::LevelBar(recipe && recipe->tags.count("smelting") ? ge->crafter.progress: 0.0f);
					}

					if (ge->spec->recipeTags.count("teleporting")) {
						ImGui::Print(fmtc("Teleporting: %s", recipe && recipe->tags.count("teleporting") ? recipe->name: "(nothing)"));
						ImGui::LevelBar(recipe && recipe->tags.count("teleporting") ? ge->crafter.progress: 0.0f);
						ImGui::Print("Shipment");
						ImGui::LevelBar(ge->crafter.inputsProgress);
					}

					if (ge->spec->recipeTags.count("crafting")) {
						ImGui::Print(fmtc("Crafting: %s", recipe && recipe->tags.count("crafting") ? recipe->name: "(nothing)"));
						ImGui::LevelBar(recipe && recipe->tags.count("crafting") ? ge->crafter.progress: 0.0f);
					}

					ImGui::Print(fmtc("Products completed: %d", ge->crafter.completed));
				}

				if (ge->spec->store) {
					ImGui::Print("Storage");
					ImGui::LevelBar(ge->store.usage.portion(ge->store.limit));
				}

				if (ge->spec->pipeCapacity) {
					ImGui::Print(fmtc("Fluid: %s %s",
						ge->pipe.fid ? Fluid::get(ge->pipe.fid)->name: "(none)",
						ge->pipe.fid ? Liquid((uint)((float)ge->spec->pipeCapacity.value * ge->pipe.level)).format(): "0l"
					));
					ImGui::LevelBar(ge->pipe.level);
				}
			}

			if (camera->placing) {
				for (auto ge: camera->placing->entities) {
					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Print(ge->spec->name);

					if (ge->spec->recipeTags.count("mining")) {
						auto minables = Chunk::minables(ge->box());
						for (Stack stack: minables) {
							ImGui::Print(fmtc(fmtc("%s(%d)", Item::get(stack.iid)->name, stack.size)));
						}
					}
				}
			}

			ImGui::End();

			if (popup && popup->visible) {
				popup->draw();
			}

			rlglDraw();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		EndDrawing();

		camera->statsFrame.set(camera->frame, GetFrameTime()*1000.0f);
		camera->statsFrame.update(camera->frame);
		camera->frame++;
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	running = false;
	chunkGenerator.join();

	UnloadRenderTexture(secondary);

	UnloadShader(shader);
	UnloadShader(pshader);
	UnloadShader(particleShader);

	UnloadModel(cube);
	UnloadModel(View::waterCube);

	delete status;
	delete statsPopup;
	delete entityPopup;

	delete mod;
	delete camera;
	delete camSec;

	Entity::reset();
	Chunk::reset();
	Spec::reset();
	Tech::reset();
	Part::reset();
	Recipe::reset();
	Item::reset();

	CloseWindow();
	return 0;
}
