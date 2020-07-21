
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
	SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_VSYNC_HINT|FLAG_MSAA_4X_HINT);
	InitWindow(1920,1080,"test9");
	SetTargetFPS(60);
	SetExitKey(0);

	SiteCamera *camSec = new SiteCamera(
		(Vector3){50,50,50},
		(Vector3){-1,-1,-1}
	);

	MainCamera *camera = new MainCamera(
		(Vector3){5,5,5},
		(Vector3){0,0,0}
	);

	float ambientCol[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

	Shader shader = LoadShader(
		FormatText("shaders/glsl%i/base_lighting.vs", GLSL_VERSION),
		FormatText("shaders/glsl%i/lighting.fs", GLSL_VERSION)
	);

	shader.locs[LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
	shader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");

	SetShaderValue(shader, GetShaderLocation(shader, "ambient"), &ambientCol, UNIFORM_VEC4);

	Shader pshader = LoadShader(
		FormatText("shaders/glsl%i/base_lighting_instanced.vs", GLSL_VERSION),
		FormatText("shaders/glsl%i/lighting.fs", GLSL_VERSION)
	);

	pshader.locs[LOC_MATRIX_MVP] = GetShaderLocation(pshader, "mvp");
	pshader.locs[LOC_MATRIX_MODEL] = GetShaderAttribLocation(pshader, "instance");

	SetShaderValue(pshader, GetShaderLocation(pshader, "ambient"), &ambientCol, UNIFORM_VEC4);

	Part::shader = pshader;
	Part::material = LoadMaterialDefault();

	Light lightA = CreateLight(LIGHT_DIRECTIONAL, (Vector3){ -1, 1, 0 }, Vector3Zero(), WHITE, shader);
	Light lightB = CreateLight(LIGHT_DIRECTIONAL, (Vector3){ -1, 1, 0 }, Vector3Zero(), WHITE, pshader);

	//Sim::seed(879600773);
	Sim::seed(2);

	Item* item = new Item(Item::next(), "log");
	item->image = LoadImage("icons/none.png");

	Spec* spec = new Spec("provider-container");
	spec->image = LoadImage("icons/provider-container.png");
	spec->animations[South].w = 2;
	spec->animations[South].h = 2;
	spec->animations[South].d = 5;
	spec->animations[East].w = 5;
	spec->animations[East].h = 2;
	spec->animations[East].d = 2;
	spec->parts = {
		new Part("models/container.stl", GetColor(0x990000ff)),
		(new PartSpinner("models/fan.stl", GetColor(0xccccccff)))->translate(0,1.1,0),
	};
	spec->align = true;
	spec->rotate = false;
	spec->rotateGhost = true;
	spec->vehicle = false;
	spec->store = true;

	spec->animations[North] = spec->animations[South];
	spec->animations[West] = spec->animations[East];

	spec = new Spec("requester-container");
	spec->image = LoadImage("icons/requester-container.png");
	spec->animations[South].w = 2;
	spec->animations[South].h = 2;
	spec->animations[South].d = 5;
	spec->animations[East].w = 5;
	spec->animations[East].h = 2;
	spec->animations[East].d = 2;
	spec->parts = {
		new Part("models/container.stl", GetColor(0x0044ccff)),
	};
	spec->align = true;
	spec->rotate = false;
	spec->rotateGhost = true;
	spec->vehicle = false;
	spec->store = true;

	spec->animations[North] = spec->animations[South];
	spec->animations[West] = spec->animations[East];

	spec = new Spec("buffer-container");
	spec->image = LoadImage("icons/buffer-container.png");
	spec->animations[South].w = 2;
	spec->animations[South].h = 2;
	spec->animations[South].d = 5;
	spec->animations[East].w = 5;
	spec->animations[East].h = 2;
	spec->animations[East].d = 2;
	spec->parts = {
		new Part("models/container.stl", GetColor(0x006600ff)),
	};
	spec->align = true;
	spec->rotate = false;
	spec->rotateGhost = true;
	spec->vehicle = false;
	spec->store = true;

	spec->animations[North] = spec->animations[South];
	spec->animations[West] = spec->animations[East];

	spec = new Spec("assembler");
	spec->image = LoadImage("icons/none.png");
	spec->animations[South].w = 5;
	spec->animations[South].h = 3;
	spec->animations[South].d = 5;
	spec->parts = {
		new Part("models/assembler.stl", GetColor(0x009900ff)),
	};
	spec->align = true;
	spec->rotate = true;
	spec->rotateGhost = true;
	spec->vehicle = false;
	spec->store = true;

	spec->animations[North] = spec->animations[South];
	spec->animations[East] = spec->animations[South];
	spec->animations[West] = spec->animations[South];

	spec = new Spec("belt");
	spec->image = LoadImage("icons/none.png");
	spec->animations[South].w = 1;
	spec->animations[South].h = 0.5;
	spec->animations[South].d = 1;

	spec->parts = {
		new Part("models/belt-base.stl", GetColor(0xcccc00ff)),
		(new Part("models/belt-chevron.stl", GetColor(0x0000ccff)))->translate(0,0.23,0),
		(new PartRoller("models/belt-roller.stl", GetColor(0xccccccff)))->translate(0,0.25,0),
	};

	for (int i = 1; i < 4; i++) {
		spec->parts.push_back(
			(new PartRoller("models/belt-roller.stl", GetColor(0xccccccff)))->translate(0,0.25,0.14f*i)
		);
		spec->parts.push_back(
			(new PartRoller("models/belt-roller.stl", GetColor(0xccccccff)))->translate(0,0.25,0.14f*-i)
		);
	}

	spec->align = true;
	spec->rotate = true;
	spec->rotateGhost = true;
	spec->vehicle = false;

	spec->animations[North] = spec->animations[South];
	spec->animations[East] = spec->animations[South];
	spec->animations[West] = spec->animations[South];

	std::vector<Spec*> rocks;

	for (int i = 1; i < 4; i++) {
		auto name = "rock" + std::to_string(i);
		auto part = "models/" + name + ".stl";

		spec = new Spec(name);
		spec->image = LoadImage("icons/none.png");
		spec->animations[South].w = 2;
		spec->animations[South].h = 1;
		spec->animations[South].d = 2;
		spec->parts = {
			new PartFacer(part, GetColor(0x666666ff)),
		};
		spec->align = false;
		spec->rotate = false;
		spec->rotateGhost = false;
		spec->vehicle = false;

		rocks.push_back(spec);
	}

	Chunk::generator([&](Chunk *chunk) {
		for (int y = 0; y < Chunk::size; y++) {
			for (int x = 0; x < Chunk::size; x++) {
				float e = chunk->tiles[y][x].elevation;
				if (e < 0.01 && e > -0.01) {
					double n = Sim::noise2D(chunk->x*Chunk::size+x + 1000000, chunk->y*Chunk::size+y + 1000000, 8, 0.6, 0.015);
					if (n < 0.4 && Sim::random() < 0.04) {
						Spec *spec = rocks[Sim::choose(rocks.size())];
						Entity::create(Entity::next(), spec)
							.setGhost(true)
							.move((Point){
								(float)(chunk->x*Chunk::size+x),
								spec->animations[South].h/2.0f,
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
	spec->animations[South].w = 2;
	spec->animations[South].h = 5;
	spec->animations[South].d = 2;
	spec->parts = {
		(new PartFacer("models/tree1.stl", GetColor(0x224400ff)))->translate(0,-2.5,0),
	};
	spec->align = false;
	spec->rotate = false;
	spec->rotateGhost = false;
	spec->vehicle = false;

	trees.push_back(spec);

	spec = new Spec("tree2");
	spec->image = LoadImage("icons/none.png");
	spec->animations[South].w = 2;
	spec->animations[South].h = 6;
	spec->animations[South].d = 2;
	spec->parts = {
		(new PartFacer("models/tree2.stl", GetColor(0x006600ff)))->translate(-5,-3,0),
	};
	spec->align = false;
	spec->rotate = false;
	spec->rotateGhost = false;
	spec->vehicle = false;

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
							.move((Point){
								(float)(chunk->x*Chunk::size+x),
								(e*100.0f) + spec->animations[South].h/2.0f,
								(float)(chunk->y*Chunk::size+y),
							})
							.setGhost(false)
						;
					}
				}
			}
		}
	});

	spec = new Spec("engineer-truck");
	spec->image = LoadImage("icons/none.png");
	spec->animations[South].w = 2;
	spec->animations[South].h = 2;
	spec->animations[South].d = 3;
	spec->parts = {
		(new Part("models/truck-chassis.stl", GetColor(0xcccc00ff)))->translate(0,0.3,0),
		(new PartWheel("models/truck-wheel.stl", GetColor(0x444444ff)))->speed(1)->steer(20)->translate(-0.8,-0.75,-1),
		(new PartWheel("models/truck-wheel.stl", GetColor(0x444444ff)))->translate(-0.8,-0.75,0),
		(new PartWheel("models/truck-wheel.stl", GetColor(0x444444ff)))->translate(-0.8,-0.75,1),
		(new PartWheel("models/truck-wheel.stl", GetColor(0x444444ff)))->translate(0.8,-0.75,-1),
		(new PartWheel("models/truck-wheel.stl", GetColor(0x444444ff)))->translate(0.8,-0.75,0),
		(new PartWheel("models/truck-wheel.stl", GetColor(0x444444ff)))->translate(0.8,-0.75,1),
	};
	spec->align = false;
	spec->rotate = false;
	spec->rotateGhost = false;
	spec->vehicle = true;
	spec->store = true;

	spec = new Spec("camera-drone");
	spec->image = LoadImage("icons/none.png");
	spec->animations[South].w = 1;
	spec->animations[South].h = 1;
	spec->animations[South].d = 1;
	spec->parts = {
		(new Part("models/drone-chassis.stl", GetColor(0x660000ff))),
	};
	spec->align = false;
	spec->rotate = false;
	spec->rotateGhost = false;
	spec->drone = true;

	Model cube = LoadModelFromMesh(GenMeshCube(1.0f,1.0f,1.0f));
	cube.materials[0].shader = shader;

	View::waterCube = LoadModelFromMesh(GenMeshCube(1.0f,1.0f,1.0f));
	View::waterCube.materials[0].shader = pshader;
	View::waterCube.materials[0].maps[MAP_DIFFUSE].color = GetColor(0x010190FF);

	Chunk::material = LoadMaterialDefault();
	Chunk::material.shader = shader;

	if (loadSave) {
		Sim::load("autosave");
	}

	if (!loadSave) {
		int horizon = 15;
		for (int cy = -horizon; cy < horizon; cy++) {
			for (int cx = -horizon; cx < horizon; cx++) {
				Chunk::get(cx,cy);
			}
		}

		Entity::create(Entity::next(), Spec::byName("engineer-truck")).floor(0)
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

	Mod* mod = new Mod("base");
	mod->load();

	RenderTexture secondary = LoadRenderTexture(GetScreenWidth()/4, GetScreenHeight()/4);

	while (!WindowShouldClose()) {
		Sim::locked(Sim::update);
		mod->update();

		camera->update();
		camSec->update();

		UpdateLightValues(shader, lightA);
		UpdateLightValues(pshader, lightB);

		float cameraPos[3] = { camera->position.x, camera->position.y, camera->position.z };
		SetShaderValue(shader, shader.locs[LOC_VECTOR_VIEW], cameraPos, UNIFORM_VEC3);

		if (!camera->popup || !camera->popup->contains(camera->mouse.x, camera->mouse.y)) {

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

			if (IsKeyReleased(KEY_T) && camera->hovering) {
				camera->lookAt(camera->hovering->pos.floor(0));
			}

			if (IsKeyReleased(KEY_E)) {
				camera->popup = (camera->popup && camera->popup == camera->buildPopup) ? NULL: camera->buildPopup;
			}

			if (IsKeyReleased(KEY_G)) {
				camera->showGrid = !camera->showGrid;
			}

			if (camera->mouse.left && camera->mouse.leftChanged && camera->placing) {
				Sim::locked([&]() {
					Entity::create(Entity::next(), camera->placing->spec)
						.setGhost(true)
						.face(camera->placing->dir)
						.move(camera->placing->pos)
						.setGhost(false);
				});
			}

			if (camera->mouse.left && camera->mouse.leftChanged && !camera->placing && camera->hovering) {
				camera->popup = camera->entityPopup;
				Sim::locked([&]() {
					GuiEntity *ge = new GuiEntity(camera->hovering->id);
					camera->entityPopup->useEntity(ge);
				});
			}

			if (IsKeyReleased(KEY_DELETE) && camera->hovering) {
				Sim::locked([&]() {
					int id = camera->hovering->id;
					if (Entity::exists(id)) {
						Entity::get(id).destroy();
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

			camSec->draw(secondary);

			camera->draw();

			DrawTexturePro(secondary.texture,
				(Rectangle){0,0,(float)secondary.texture.width,(float)-secondary.texture.height},
				(Rectangle){0,0,(float)secondary.texture.width,(float)secondary.texture.height},
				(Vector2){0,0}, 0.0f, WHITE
			);

			DrawFPS(10,10);
		EndDrawing();
	}

	delete camSec;

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

	Entity::reset();
	Chunk::reset();
	Spec::reset();
	Part::reset();
	Item::reset();

	CloseWindow();
	return 0;
}