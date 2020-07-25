
#define GRAPHICS_API_OPENGL_33
#define GLSL_VERSION 330
#include "raylib.h"
#include "raymath.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#include "common.h"
#include "panel.h"
#include "mod.h"
#include "sim.h"
#include "chunk.h"
#include "spec.h"
#include "entity.h"
#include "view.h"
#include "item.h"
#include "recipe.h"

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

	//SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_VSYNC_HINT|FLAG_MSAA_4X_HINT);
	InitWindow(1920,1080,"test9");
	SetTargetFPS(60);
	SetExitKey(0);

	SiteCamera *camSec = new SiteCamera(
		{50,50,50},
		{-1,-1,-1}
	);

	MainCamera *camera = new MainCamera(
		{10,10,10},
		{-1,-1,-1}
	);

  float fogDensity = 0.004f;
	Vector4 fogColor = ColorNormalize(SKYBLUE);
	Vector4 ambient = { 0.2f, 0.2f, 0.2f, 1.0f };

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
	pshader.locs[LOC_MATRIX_MODEL] = GetShaderAttribLocation(pshader, "instance");
	pshader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(pshader, "viewPos");

  SetShaderValue(pshader, GetShaderLocation(pshader, "fogDensity"), &fogDensity, UNIFORM_FLOAT);
  SetShaderValue(pshader, GetShaderLocation(pshader, "fogColor"), &fogColor, UNIFORM_VEC4);
	SetShaderValue(pshader, GetShaderLocation(pshader, "ambient"), &ambient, UNIFORM_VEC4);

	Part::shader = pshader;
	Part::material = LoadMaterialDefault();

	Light lightA = CreateLight(LIGHT_DIRECTIONAL, Point(-1, 1, 0), Point::Zero(), WHITE, shader);
	Light lightB = CreateLight(LIGHT_DIRECTIONAL, Point(-1, 1, 0), Point::Zero(), WHITE, pshader);

	Sim::seed(879600773);
	//Sim::seed(4);

	Item* item = new Item(Item::next(), "log");
	item->image = LoadImage("icons/none.png");

	item = new Item(Item::next(), "iron-ore");
	item->image = LoadImage("icons/none.png");
	Item::mining.insert(item);

	Recipe* recipe = new Recipe(Recipe::next(), "mining1");
	recipe->image = LoadImage("icons/none.png");
	recipe->tags = {"mining"};
	recipe->mining = true;

	auto thingContainer = Thing("models/container.stl");
	auto thingFan = Thing("models/fan.stl");
	auto thingGear = Thing("models/gear.stl");
	auto thingAssembler = Thing("models/assembler.stl");
	auto thingFurnace = Thing("models/furnace.stl");
	auto thingMiner = Thing("models/miner.stl");
	auto thingBeltBase = Thing("models/belt-base.stl");
	auto thingBeltChevron = Thing("models/belt-chevron.stl");
	auto thingBeltRoller = Thing("models/belt-roller.stl");
	auto thingTruckChassisEngineer = Thing("models/truck-chassis-engineer.stl");
	auto thingTruckChassisHauler = Thing("models/truck-chassis-hauler.stl");
	auto thingTruckWheel = Thing("models/truck-wheel.stl");

	Spec* spec = new Spec("provider-container");
	spec->image = LoadImage("icons/provider-container.png");
	spec->w = 2;
	spec->h = 2;
	spec->d = 5;
	spec->parts = {
		(new Part(thingContainer))->paint(0x990000ff),
		(new PartSpinner(thingFan, 4))->paint(0xccccccff)->translate(0,1.1,0),
	};
	spec->store = true;
	spec->rotate = true;

	spec = new Spec("requester-container");
	spec->image = LoadImage("icons/requester-container.png");
	spec->w = 2;
	spec->h = 2;
	spec->d = 5;
	spec->parts = {
		(new Part(thingContainer))->paint(0x0044ccff),
	};
	spec->store = true;
	spec->rotate = true;

	spec = new Spec("buffer-container");
	spec->image = LoadImage("icons/buffer-container.png");
	spec->w = 2;
	spec->h = 2;
	spec->d = 5;
	spec->parts = {
		(new Part(thingContainer))->paint(0x006600ff),
	};
	spec->store = true;
	spec->rotate = true;

	spec = new Spec("assembler");
	spec->image = LoadImage("icons/none.png");
	spec->w = 5;
	spec->h = 3;
	spec->d = 5;
	spec->parts = {
		(new Part(thingAssembler))->paint(0x009900ff),
	};
	spec->store = true;
	spec->rotate = true;

	spec = new Spec("furnace");
	spec->image = LoadImage("icons/none.png");
	spec->w = 4;
	spec->h = 4;
	spec->d = 4;
	spec->parts = {
		(new Part(thingFurnace))->paint(0xcc6600ff),
	};
	spec->store = true;
	spec->rotate = true;

	spec = new Spec("miner");
	spec->image = LoadImage("icons/none.png");
	spec->w = 5;
	spec->h = 5;
	spec->d = 5;
	spec->parts = {
		(new Part(thingMiner))->paint(0xB7410Eff),
		(new PartSpinner(thingGear, 1))->paint(0xccccccff)->scale(3,3,3)->rotate(Point::East(), 90)->translate(0,0,1.5),
	};
	spec->store = true;
	spec->rotate = true;
	spec->place = Spec::Hill;

	spec->crafter = true;
	spec->recipeTags = {"mining"};

	spec = new Spec("belt");
	spec->image = LoadImage("icons/none.png");
	spec->w = 1;
	spec->h = 0.5;
	spec->d = 1;
	spec->rotate = true;

	spec->parts = {
		(new Part(thingBeltBase))->paint(0xcccc00ff),
		(new Part(thingBeltChevron))->paint(0x0000ccff)->translate(0,0.23,0),
		(new Part(thingBeltRoller))->paint(0xccccccff)->translate(0,0.25,0),
	};

	for (int i = 1; i < 4; i++) {
		spec->parts.push_back(
			(new Part(thingBeltRoller))->paint(0xccccccff)->translate(0,0.25,0.14f*i)
		);
		spec->parts.push_back(
			(new Part(thingBeltRoller))->paint(0xccccccff)->translate(0,0.25,0.14f*-i)
		);
	}

	std::vector<Spec*> rocks;

	for (int i = 1; i < 4; i++) {
		auto name = "rock" + std::to_string(i);
		auto part = "models/" + name + ".stl";

		spec = new Spec(name);
		spec->image = LoadImage("icons/none.png");
		spec->w = 2;
		spec->h = 1;
		spec->d = 2;
		spec->pivot = true;
		spec->parts = {
			(new Part(Thing(part)))->paint(0x666666ff),
		};

		rocks.push_back(spec);
	}

	Chunk::generator([&](Chunk *chunk) {
		for (int y = 0; y < Chunk::size; y++) {
			for (int x = 0; x < Chunk::size; x++) {
				Box bounds = {(float)chunk->x*Chunk::size+x, 0.0f, (float)chunk->y*Chunk::size+y, 2.0f, 1.0f, 2.0f};
				if (Chunk::isLand(bounds.grow(1.0f))) {
					double n = Sim::noise2D(chunk->x*Chunk::size+x + 1000000, chunk->y*Chunk::size+y + 1000000, 8, 0.6, 0.015);
					if (n < 0.4 && Sim::random() < 0.04) {
						Spec *spec = rocks[Sim::choose(rocks.size())];
						Entity::create(Entity::next(), spec)
							.setGhost(true)
							.look(Point::South().randomHorizontal())
							.move((Point){
								(float)(chunk->x*Chunk::size+x),
								spec->h/2.0f,
								(float)(chunk->y*Chunk::size+y),
							})
							.setGhost(false)
						;
					}
				}
			}
		}
	});

	std::vector<Spec*> trees;

	spec = new Spec("tree1");
	spec->image = LoadImage("icons/none.png");
	spec->w = 2;
	spec->h = 5;
	spec->d = 2;
	spec->pivot = true;
	spec->parts = {
		(new Part(Thing("models/tree1.stl").smooth()))->paint(0x224400ff)->translate(0,-2.5,0),
	};

	trees.push_back(spec);

	spec = new Spec("tree2");
	spec->image = LoadImage("icons/none.png");
	spec->w = 2;
	spec->h = 6;
	spec->d = 2;
	spec->pivot = true;
	spec->parts = {
		(new Part(Thing("models/tree2.stl").smooth()))->paint(0x006600ff)->translate(-5,-3,0),
	};

	trees.push_back(spec);

	Chunk::generator([&](Chunk *chunk) {
		for (int y = 0; y < Chunk::size; y++) {
			for (int x = 0; x < Chunk::size; x++) {
				float e = chunk->tiles[y][x].elevation;
				if (e > -0.01) {
					double n = Sim::noise2D(chunk->x*Chunk::size+x + 2000000, chunk->y*Chunk::size+y + 2000000, 8, 0.6, 0.015);
					if (n < 0.4 && Sim::random() < 0.04) {
						Spec *spec = trees[Sim::choose(trees.size())];
						Entity::create(Entity::next(), spec)
							.setGhost(true)
							.look(Point::South().randomHorizontal())
							.move((Point){
								(float)(chunk->x*Chunk::size+x),
								(e*100.0f) + spec->h/2.0f,
								(float)(chunk->y*Chunk::size+y),
							})
							.setGhost(false)
						;
					}
				}
			}
		}
	});

	spec = new Spec("truck-engineer");
	spec->image = LoadImage("icons/none.png");
	spec->w = 2;
	spec->h = 2;
	spec->d = 3;
	spec->parts = {
		(new Part(thingTruckChassisEngineer))->paint(0xff6600ff)->translate(0,0.3,0),
		(new Part(thingTruckWheel))->paint(0x444444ff)->translate(-0.8,-0.75,-1),
		(new Part(thingTruckWheel))->paint(0x444444ff)->translate(-0.8,-0.75,0),
		(new Part(thingTruckWheel))->paint(0x444444ff)->translate(-0.8,-0.75,1),
		(new Part(thingTruckWheel))->paint(0x444444ff)->translate(0.8,-0.75,-1),
		(new Part(thingTruckWheel))->paint(0x444444ff)->translate(0.8,-0.75,0),
		(new Part(thingTruckWheel))->paint(0x444444ff)->translate(0.8,-0.75,1),
	};
	spec->align = false;
	spec->pivot = true;
	spec->vehicle = true;
	spec->store = true;

	spec->costGreedy = 1.3;
	spec->clearance = 1.5;

	spec = new Spec("truck-hauler");
	spec->image = LoadImage("icons/none.png");
	spec->w = 2;
	spec->h = 2;
	spec->d = 3;
	spec->parts = {
		(new Part(thingTruckChassisHauler))->paint(0xffcc00ff)->translate(0,0.3,0),
		Spec::byName("truck-engineer")->parts[1],
		Spec::byName("truck-engineer")->parts[2],
		Spec::byName("truck-engineer")->parts[3],
		Spec::byName("truck-engineer")->parts[4],
		Spec::byName("truck-engineer")->parts[5],
		Spec::byName("truck-engineer")->parts[6],
	};
	spec->align = false;
	spec->vehicle = true;
	spec->store = true;

	spec = new Spec("truck-stop");
	spec->image = LoadImage("icons/none.png");
	spec->w = 3;
	spec->h = 0.1;
	spec->d = 3;
	spec->parts = {
		(new Part(Thing("models/truck-stop.stl")))->paint(0x662222ff),
	};
	spec->pivot = true;

	spec = new Spec("camera-drone");
	spec->image = LoadImage("icons/none.png");
	spec->w = 1;
	spec->h = 1;
	spec->d = 1;
	spec->parts = {
		(new Part(Thing("models/drone-chassis.stl")))->paint(0x660000ff),
	};
	spec->align = false;
	spec->drone = true;

	Model cube = LoadModelFromMesh(GenMeshCube(1.0f,1.0f,1.0f));
	cube.materials[0].shader = shader;

	View::waterCube = LoadModelFromMesh(GenMeshCube(1.0f,1.0f,1.0f));
	View::waterCube.materials[0].shader = pshader;
	View::waterCube.materials[0].maps[MAP_DIFFUSE].color = GetColor(0x010190FF);

	View::redCube = LoadModelFromMesh(GenMeshCube(0.5f,0.5f,0.5f));
	View::redCube.materials[0].shader = pshader;
	View::redCube.materials[0].maps[MAP_DIFFUSE].color = GetColor(0xff0000FF);

	View::greenCube = LoadModelFromMesh(GenMeshCube(0.5f,0.5f,0.5f));
	View::greenCube.materials[0].shader = pshader;
	View::greenCube.materials[0].maps[MAP_DIFFUSE].color = GetColor(0x00ff00FF);

	Chunk::material = LoadMaterialDefault();
	Chunk::material.shader = shader;
	Chunk::material.maps[MAP_DIFFUSE].color = GetColor(0xffffffff);

	if (loadSave) {
		Sim::load("autosave");
	}

	if (!loadSave) {
		int horizon = 4;
		for (int cy = -horizon; cy < horizon; cy++) {
			for (int cx = -horizon; cx < horizon; cx++) {
				Chunk::get(cx,cy);
			}
		}

		Entity::create(Entity::next(), Spec::byName("truck-engineer")).floor(0)
			.store()
				.insert((Stack){Item::byName("log")->id, 10})
		;

		Entity::create(Entity::next(), Spec::byName("camera-drone"))
			.move((Point){0,20,0})
		;
	}

	Panels::init();
	camera->buildPopup = new BuildPopup(camera, 800, 800);
	camera->entityPopup = new EntityPopup(camera, 800, 800);
	camera->recipePopup = new RecipePopup(camera, 800, 800);

	Mod* mod = new Mod("base");
	mod->load();

	RenderTexture secondary = LoadRenderTexture(GetScreenWidth()/4, GetScreenHeight()/4);

	while (!WindowShouldClose()) {
		Sim::locked(Sim::update);
		mod->update();

		camera->update();
		camSec->update();

		if (camera->worldFocused) {

			if (IsKeyReleased(KEY_Q)) {
				camera->build(camera->hovering ? camera->hovering->spec: NULL);
			}

			if (IsKeyReleased(KEY_R)) {
				if (camera->placing) {
					camera->placing->rotate();
				}
				else
				if (camera->hovering) {
					Sim::locked([&]() {
						Entity::get(camera->hovering->id)
							.rotate();
					});
				}
			}

			if (IsKeyReleased(KEY_E)) {
				camera->popup = (camera->popup && camera->popup == camera->buildPopup) ? NULL: camera->buildPopup;
			}

			if (IsKeyReleased(KEY_G)) {
				camera->showGrid = !camera->showGrid;
			}

			if (camera->mouse.left.clicked && IsKeyDown(KEY_LEFT_SHIFT)) {
				Sim::locked([&]() {
					for (Entity& en: Entity::all) {
						if (en.spec->vehicle) {
							RayHitInfo spot = GetCollisionRayGround(camera->mouse.ray, 0);
							en.vehicle().addWaypoint(Point(spot.position));
							break;
						}
					}
				});
			}

			if (camera->mouse.left.clicked && camera->placing) {
				Sim::locked([&]() {
					if (Entity::fits(camera->placing->spec, camera->placing->pos, camera->placing->dir)) {
						Entity::create(Entity::next(), camera->placing->spec)
							.setGhost(true)
							.look(camera->placing->dir)
							.move(camera->placing->pos)
							.setGhost(false);
					}
				});
			}

			if (camera->mouse.left.clicked && !camera->placing && camera->hovering) {
				camera->popup = camera->entityPopup;
				Sim::locked([&]() {
					camera->entityPopup->useEntity(camera->hovering->id);
				});
			}

			if (IsKeyReleased(KEY_DELETE) && camera->hovering) {
				Sim::locked([&]() {
					int id = camera->hovering->id;
					if (Entity::exists(id)) {
						Entity::get(id).remove();
					};
				});
			}

			if (IsKeyReleased(KEY_ESCAPE)) {
				if (camera->popup) {
					camera->popup = NULL;
				}
				else
				if (camera->placing) {
					delete camera->placing;
					camera->placing = NULL;
				}
			}

			if (IsKeyReleased(KEY_F5)) {
				Sim::save("autosave");
			}
		}

		if (camera->popup) {
			camera->popup->update();
		}

		for (auto spec: Spec::all) {
			for (auto part: spec.second->parts) {
				part->update();
			}
		}

		BeginDrawing();

			UpdateLightValues(shader, lightA);
			UpdateLightValues(pshader, lightB);

			Point cameraTarget = camSec->groundTarget(0);
			SetShaderValue(shader, shader.locs[LOC_VECTOR_VIEW], &cameraTarget.x, UNIFORM_VEC3);
			SetShaderValue(pshader, pshader.locs[LOC_VECTOR_VIEW], &cameraTarget.x, UNIFORM_VEC3);

			camSec->draw(secondary);

			cameraTarget = camera->groundTarget(0);
			SetShaderValue(shader, shader.locs[LOC_VECTOR_VIEW], &cameraTarget.x, UNIFORM_VEC3);
			SetShaderValue(pshader, pshader.locs[LOC_VECTOR_VIEW], &cameraTarget.x, UNIFORM_VEC3);

			camera->draw();

			DrawTexturePro(secondary.texture,
				(Rectangle){0,0,(float)secondary.texture.width,(float)-secondary.texture.height},
				(Rectangle){0,0,(float)secondary.texture.width,(float)secondary.texture.height},
				(Vector2){0,0}, 0.0f, WHITE
			);

			DrawFPS(10,10);
		EndDrawing();
	}


	UnloadRenderTexture(secondary);

	UnloadShader(shader);
	UnloadShader(pshader);

	UnloadModel(cube);
	UnloadModel(View::waterCube);

	for (auto pair: Chunk::all) {
		Chunk *chunk = pair.second;
		chunk->dropHeightMap();
	}

	delete mod;
	delete camera->buildPopup;
	delete camera->entityPopup;
	delete camera->recipePopup;
	delete camera;
	delete camSec;

	Entity::reset();
	Chunk::reset();
	Spec::reset();
	Part::reset();
	Recipe::reset();
	Item::reset();

	CloseWindow();
	return 0;
}