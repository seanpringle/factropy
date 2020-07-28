
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
#include <ctime>

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
	Vector4 ambient = { 0.3f, 0.3f, 0.3f, 1.0f };

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

	Sim::reset();
	Sim::seed(879600773);
	//Sim::seed(4);

	Item* item = new Item(Item::next(), "log");
	item->part = (new Part(Thing("models/wood.stl")))->scale(0.5f, 0.5f, 0.5f)->paint(0xCD853Fff);

	item = new Item(Item::next(), "iron-ore");
	item->part = (new Part(Thing("models/iron-ore.stl")))->scale(0.5f, 0.5f, 0.5f)->paint(0xB7410Eff);
	Item::mining.insert(item->id);

	item = new Item(Item::next(), "copper-ore");
	item->part = (new Part(Thing("models/copper-ore.stl")))->scale(0.5f, 0.5f, 0.5f)->paint(0x529f88ff);
	Item::mining.insert(item->id);

	item = new Item(Item::next(), "coal");
	item->part = (new Part(Thing("models/coal.stl")))->scale(0.5f, 0.5f, 0.5f)->paint(0x222222ff);
	Item::mining.insert(item->id);

	item = new Item(Item::next(), "stone");
	item->part = (new Part(Thing("models/stone.stl")))->scale(0.5f, 0.5f, 0.5f)->paint(0x999999ff);
	Item::mining.insert(item->id);

	auto thingIngot = Thing("models/ingot.stl");

	item = new Item(Item::next(), "iron-ingot");
	item->part = (new Part(thingIngot))->paint(0x686969ff);

	item = new Item(Item::next(), "copper-ingot");
	item->part = (new Part(thingIngot))->paint(0xDC7F64ff);

	auto thingContainer = Thing("models/container.stl");
	auto thingFan = Thing("models/fan.stl");
	auto thingGear = Thing("models/gear.stl");
	auto thingAssembler = Thing("models/assembler.stl");
	auto thingFurnace = Thing("models/furnace.stl");
	auto thingMiner = Thing("models/miner.stl");
	auto thingTruckChassisEngineer = Thing("models/truck-chassis-engineer.stl");
	auto thingTruckChassisHauler = Thing("models/truck-chassis-hauler.stl");
	auto thingTruckWheel = Thing("models/truck-wheel.stl");

	Recipe* recipe = new Recipe(Recipe::next(), "mining1");
	recipe->tags = {"mining"};
	recipe->mining = true;
	recipe->parts = {
		(new Part(thingMiner))->paint(0xB7410Eff)->scale(0.1f,0.1f,0.1f),
		(new Part(thingGear))->paint(0xccccccff)->scale(0.3f,0.3f,0.3f)->rotate(Point::East(), 90)->translate(0,0,0.15),
	};

	Spec* spec = new Spec("provider-container");
	spec->w = 2;
	spec->h = 2;
	spec->d = 5;
	spec->parts = {
		(new Part(thingContainer))->paint(0x990000ff),
		(new PartSpinner(thingFan, 4))->paint(0xccccccff)->translate(0,1.1,0),
	};
	spec->store = true;
	spec->enableSetLower = true;
	spec->rotate = true;

	spec = new Spec("requester-container");
	spec->w = 2;
	spec->h = 2;
	spec->d = 5;
	spec->parts = {
		(new Part(thingContainer))->paint(0x0044ccff),
	};
	spec->store = true;
	spec->enableSetLower = true;
	spec->enableSetUpper = true;
	spec->rotate = true;

	spec = new Spec("buffer-container");
	spec->w = 2;
	spec->h = 2;
	spec->d = 5;
	spec->parts = {
		(new Part(thingContainer))->paint(0x006600ff),
	};
	spec->store = true;
	spec->rotate = true;

	spec = new Spec("assembler");
	spec->w = 6;
	spec->h = 3;
	spec->d = 6;
	spec->parts = {
		(new Part(thingAssembler))->paint(0x009900ff),
	};
	spec->store = true;
	spec->rotate = true;

	spec = new Spec("furnace");
	spec->w = 4;
	spec->h = 4;
	spec->d = 4;
	spec->parts = {
		(new Part(thingFurnace))->paint(0xcc6600ff),
	};
	spec->store = true;
	spec->rotate = true;

	spec = new Spec("miner");
	spec->w = 5;
	spec->h = 5;
	spec->d = 10;
	spec->parts = {
		(new Part(thingMiner))->paint(0xB7410Eff),
		(new PartSpinner(thingGear, 1))->paint(0xccccccff)->scale(3,3,3)->rotate(Point::East(), 90)->translate(0,0,3.75),
	};
	spec->store = true;
	spec->rotate = true;
	spec->place = Spec::Hill;

	spec->crafter = true;
	spec->recipeTags = {"mining"};

	spec = new Spec("belt");
	spec->w = 1;
	spec->h = 1;
	spec->d = 1;
	spec->rotate = true;
	spec->belt = true;

	spec->parts = {
		(new Part(Thing("models/belt-base.stl")))->paint(0xcccc00ff)->translate(0,-0.5,0),
		(new Part(Thing("models/belt-surface.stl")))->paint(0x000000ff)->translate(0,-0.5,0),
		(new PartCycle(Thing("models/belt-ridge.stl"), 2))->paint(0xcccc00ff)->translate(0,-0.5,0),
	};

	std::vector<Spec*> rocks;

	for (int i = 1; i < 4; i++) {
		auto name = "rock" + std::to_string(i);
		auto part = "models/" + name + ".stl";

		spec = new Spec(name);
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
	spec->w = 2;
	spec->h = 5;
	spec->d = 2;
	spec->pivot = true;
	spec->parts = {
		(new Part(Thing("models/tree1.stl").smooth()))->paint(0x224400ff)->translate(0,-2.5,0),
	};

	trees.push_back(spec);

	spec = new Spec("tree2");
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
	spec->w = 3;
	spec->h = 0.1;
	spec->d = 3;
	spec->parts = {
		(new Part(Thing("models/truck-stop.stl")))->paint(0x662222ff),
	};
	spec->pivot = true;

	spec = new Spec("camera-drone");
	spec->w = 1;
	spec->h = 1;
	spec->d = 1;
	spec->parts = {
		(new Part(Thing("models/drone-chassis.stl")))->paint(0x660000ff),
	};
	spec->align = false;
	spec->drone = true;

	spec = new Spec("arm");
	spec->w = 2;
	spec->h = 3;
	spec->d = 2;
	spec->arm = true;
	spec->rotate = true;
	spec->parts = {
		(new Part(Thing("models/arm-base.stl")))->translate(0,-1.5,0)->paint(0x660000ff),
		(new Part(Thing("models/arm-pillar.stl")))->translate(0,-1.5,0)->paint(0x006600ff),
		(new Part(Thing("models/arm-telescope1.stl")))->translate(0,-1.5,0)->paint(0x666666ff),
		(new Part(Thing("models/arm-telescope2.stl")))->translate(0,-1.5,0)->paint(0x666666ff),
		(new Part(Thing("models/arm-telescope3.stl")))->translate(0,-1.5,0)->paint(0x666666ff),
		(new Part(Thing("models/arm-grip.stl")))->translate(0,-1.5,0)->paint(0x000066ff),
	};

	{
		Matrix state0 = MatrixIdentity();

		//x(theta) = rx cos(theta)
		//y(theta) = ry sin(theta)

		for (uint i = 0; i < 360; i++) {
			Matrix state = MatrixRotateY((float)i*DEG2RAD);

			float theta = (float)i;

			float a3 = sin(theta*DEG2RAD)*1.0;
			float b3 = cos(theta*DEG2RAD)*0.60;
			float r3 = 1.0*0.60 / std::sqrt(a3*a3 + b3*b3);

			float a4 = sin(theta*DEG2RAD)*1.0;
			float b4 = cos(theta*DEG2RAD)*0.25;
			float r4 = 1.0*0.25 / std::sqrt(a4*a4 + b4*b4);

			float a5 = sin(theta*DEG2RAD)*1.0;
			float b5 = cos(theta*DEG2RAD)*0.25;
			float r5 = 1.0*0.25 / std::sqrt(a5*a5 + b5*b5);

			Point t3 = Point::South() * (1.0f-r3);
			Point t4 = Point::South() * (1.0f-r4);
			Point t5 = Point::South() * (1.0f-r5);

			Matrix extend3 = MatrixMultiply(MatrixTranslate(t3.x, t3.y, t3.z), state);
			Matrix extend4 = MatrixMultiply(MatrixTranslate(t4.x, t4.y, t4.z), state);
			Matrix extend5 = MatrixMultiply(MatrixTranslate(t5.x, t5.y, t5.z), state);

			spec->states.push_back({
				state0,
				state,
				state,
				extend3,
				extend4,
				extend5,
			});
		}
	}

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
	Chunk::material.maps[MAP_DIFFUSE].color = WHITE;

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

		Entity& en = Entity::create(Entity::next(), Spec::byName("truck-engineer")).floor(0);
		en.store().insert({Item::byName("log")->id, 10});
	}

	Panels::init();
	camera->buildPopup = new BuildPopup(camera, 814, 800);
	camera->entityPopup = new EntityPopup(camera, 800, 800);
	camera->recipePopup = new RecipePopup(camera, 800, 800);
	camera->itemPopup = new ItemPopup(camera, 800, 800);
	camera->statsPopup = new StatsPopup(camera, 800, 800);

	Mod* mod = new Mod("base");
	mod->load();

	MessagePopup *status = new MessagePopup(800, 150);
	status->text = std::string(quotes[(uint)std::time(NULL)%(sizeof(quotes)/sizeof(char*))]);

	std::function<void(void)> loadingScreen = [&]() {
		ensure(!WindowShouldClose());

		BeginDrawing();

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

			for ([[maybe_unused]] auto [name,item]: Item::names) {
				if (!item->texture.id) break;
				DrawTextureEx(item->texture.texture, (Vector2){(float)x, (float)y}, 0.0f, 1.0, WHITE);
				advance();
			}

			for ([[maybe_unused]] auto [name,recipe]: Recipe::names) {
				if (!recipe->texture.id) break;
				DrawTextureEx(recipe->texture.texture, (Vector2){(float)x, (float)y}, 0.0f, 0.5, WHITE);
				advance();
			}

			for ([[maybe_unused]] auto [name,spec]: Spec::all) {
				if (!spec->texture.id) break;
				DrawTextureEx(spec->texture.texture, (Vector2){(float)x, (float)y}, 0.0f, 0.5, WHITE);
				advance();
			}

			status->update();
			status->draw();

		EndDrawing();
	};

	loadingScreen();

	for (auto [name,item]: Item::names) {

		item->texture = LoadRenderTexture(128, 128);

		BeginTextureMode(item->texture);

			ClearBackground(GetColor(0x0));

			BeginMode3D((Camera3D){
				position: (Vector3){0.9,1,0.9},
				target:   (Vector3){0,0.1,0},
				up:       -Point::Up(),
				fovy:     45.0f,
				type:     CAMERA_PERSPECTIVE,
			});

			item->part->draw(MatrixIdentity());

			EndMode3D();

		EndTextureMode();

		item->image = GetTextureData(item->texture.texture);
		loadingScreen();
	}

	loadingScreen();

	for (auto [name,recipe]: Recipe::names) {
		notef("rendering recipe %s", name);
		recipe->texture = LoadRenderTexture(256, 256);

		BeginTextureMode(recipe->texture);

			ClearBackground(GetColor(0x0));

			BeginMode3D((Camera3D){
				position: (Vector3){0.9,1,0.9},
				target:   (Vector3){0,0.1,0},
				up:       -Point::Up(),
				fovy:     45.0f,
				type:     CAMERA_PERSPECTIVE,
			});

			for (Part* part: recipe->parts) {
				part->draw(MatrixIdentity());
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
				position: Point(1.0f, 1.0f, 1.0f) * Point(spec->w, spec->h, spec->d).length(),
				target:   Point(0.0f, 0.0f, 0.0f),
				up:       -Point::Up(),
				fovy:     45.0f,
				type:     CAMERA_PERSPECTIVE,
			});

			for (Part* part: spec->parts) {
				part->draw(MatrixIdentity());
			}

			EndMode3D();

		EndTextureMode();

		spec->image = GetTextureData(spec->texture.texture);
		loadingScreen();
	}

	loadingScreen();

	for (auto pair: Chunk::all) {
		Chunk *chunk = pair.second;
		if (!chunk->generated) {
			chunk->genHeightMap();
			chunk->generated = true;
		}
	}

	RenderTexture secondary = LoadRenderTexture(GetScreenWidth()/4, GetScreenHeight()/4);

	while (!WindowShouldClose()) {
		Sim::locked(Sim::update);
		mod->update();

		camera->update();
		camSec->update();

		if (camera->worldFocused) {

			if (IsKeyReleased(KEY_ONE)) {
				camera->resource = 0;
			}
			if (IsKeyReleased(KEY_TWO)) {
				camera->resource = 1;
			}
			if (IsKeyReleased(KEY_THREE)) {
				camera->resource = 2;
			}
			if (IsKeyReleased(KEY_FOUR)) {
				camera->resource = 3;
			}

			if (IsKeyReleased(KEY_F1)) {
				camera->popup = camera->statsPopup;
			}

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
							.construct()
							.look(camera->placing->dir)
							.move(camera->placing->pos)
							.materialize();
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
					}
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

		camera->statsFrame.set(camera->frame, GetFrameTime());
		camera->statsFrame.update(camera->frame);
		camera->frame++;
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
	delete camera->itemPopup;
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