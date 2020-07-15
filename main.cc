
#define GRAPHICS_API_OPENGL_33
#define GLSL_VERSION 330
#include "raylib.h"
#include "raymath.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#include <cstdio>
#include <cstdlib>

#define notef(...) { fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); }
#define fatalf(...) { notef(__VA_ARGS__); exit(EXIT_FAILURE); }

#define ensure(cond,...) if (!(cond)) { exit(EXIT_FAILURE); }
#define ensuref(cond,...) if (!(cond)) { notef(__VA_ARGS__); exit(EXIT_FAILURE); }

#include "gui.h"
#include "panel.h"
#include "mod.h"
#include "sim.h"
#include "chunk.h"
#include "spec.h"
#include "entity.h"

#include <vector>

Model LoadPart(const char *path, Shader shader, Color color) {
	notef("LoadPart %s", path);
	Model model = LoadModel(path);
	ensuref(model.materialCount == 1, "multi material model");
	ensuref(model.materials[0].maps[MAP_DIFFUSE].texture.id == GetTextureDefault().id, "textured model");
	UnloadMaterial(model.materials[0]);
	model.materials[0] = LoadMaterialDefault();
	model.materials[0].shader = shader;
	model.materials[0].maps[MAP_DIFFUSE].color = color;
	return model;
}

int main(int argc, char const *argv[]) {
	//nvidia-settings --query=fsaa --verbose
	//putenv((char*)"__GL_FSAA_MODE=9");

	Sim::seed(879600773);

	Spec *spec = new Spec("provider-container");
	spec->animations[South].w = 2;
	spec->animations[South].h = 2;
	spec->animations[South].d = 5;
	spec->animations[East].w = 5;
	spec->animations[East].h = 2;
	spec->animations[East].d = 2;
	spec->obj = "models/container.obj";
	spec->color = GetColor(0x990000ff);
	spec->align = true;
	spec->rotate = false;
	spec->rotateGhost = true;

	spec->animations[North] = spec->animations[South];
	spec->animations[West] = spec->animations[East];

	spec = new Spec("requester-container");
	spec->animations[South].w = 2;
	spec->animations[South].h = 2;
	spec->animations[South].d = 5;
	spec->animations[East].w = 5;
	spec->animations[East].h = 2;
	spec->animations[East].d = 2;
	spec->obj = "models/container.obj";
	spec->color = GetColor(0x0044ccff);
	spec->align = true;
	spec->rotate = false;
	spec->rotateGhost = true;

	spec->animations[North] = spec->animations[South];
	spec->animations[West] = spec->animations[East];

	spec = new Spec("buffer-container");
	spec->animations[South].w = 2;
	spec->animations[South].h = 2;
	spec->animations[South].d = 5;
	spec->animations[East].w = 5;
	spec->animations[East].h = 2;
	spec->animations[East].d = 2;
	spec->obj = "models/container.obj";
	spec->color = GetColor(0x009900ff);
	spec->align = true;
	spec->rotate = false;
	spec->rotateGhost = true;

	spec->animations[North] = spec->animations[South];
	spec->animations[West] = spec->animations[East];

	Entity::create(Entity::next(), Spec::byName("provider-container")).floor(0);
	Entity::create(Entity::next(), Spec::byName("requester-container")).move((Point){3,0,0}).floor(0);
	Entity::create(Entity::next(), Spec::byName("buffer-container")).move((Point){-3,0,0}).floor(0);

	SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_VSYNC_HINT|FLAG_MSAA_4X_HINT);
	InitWindow(1920,1080,"test9");
	SetTargetFPS(60);
	SetExitKey(0);

	Gui::camera = {
		.position = (Vector3){5,5,5},
		.target = (Vector3){0,0,0},
		.up = (Vector3){0,1,0},
		.fovy = 45,
		.type = CAMERA_PERSPECTIVE,
	};

	Shader shader = LoadShader(
		FormatText("shaders/glsl%i/base_lighting.vs", GLSL_VERSION),
		FormatText("shaders/glsl%i/lighting.fs", GLSL_VERSION)
	);

	shader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
	shader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	int ambientLoc = GetShaderLocation(shader, "ambient");
	float ambientCol[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
	SetShaderValue(shader, ambientLoc, &ambientCol, UNIFORM_VEC4);

	Light light = CreateLight(LIGHT_DIRECTIONAL, (Vector3){ -1, 1, 0 }, Vector3Zero(), WHITE, shader);

	for (auto [name, spec]: Spec::all) {
		notef("Spec model: %s %s", name.c_str(), spec->obj.c_str());
		spec->model = LoadPart(spec->obj.c_str(), shader, spec->color);
	}

	Model cube = LoadModelFromMesh(GenMeshCube(1.0f,1.0f,1.0f));
	cube.materials[0].shader = shader;

	Model waterCube = LoadModelFromMesh(GenMeshCube(1.0f,1.0f,1.0f));
	waterCube.materials[0].maps[MAP_DIFFUSE].color = GetColor(0x010190FF);

	for (int cy = -5; cy < 5; cy++) {
		for (int cx = -5; cx < 5; cx++) {
			Chunk::get(cx,cy);
		}
	}

	for (int cy = -5; cy < 5; cy++) {
		for (int cx = -5; cx < 5; cx++) {
			Chunk::get(cx,cy)->genHeightMap();
			Chunk::get(cx,cy)->heightmap.materials[MAP_DIFFUSE].shader = shader;
		}
	}

	Panels::init();
	Gui::buildPopup = new BuildPopup(600, 600);

	Mod* mod = new Mod("base");
	mod->load();

	while (!WindowShouldClose()) {
		mod->update();

		Gui::updateMouseState();
		Gui::updateCamera();

		UpdateLightValues(shader, light);

		float cameraPos[3] = { Gui::camera.position.x, Gui::camera.position.y, Gui::camera.position.z };
		SetShaderValue(shader, shader.locs[LOC_VECTOR_VIEW], cameraPos, UNIFORM_VEC3);

		if (Gui::popup && Gui::popup->contains(Gui::mouse.x, Gui::mouse.y)) {
			Gui::popup->input();
		}
		else {

			if (IsKeyReleased(KEY_Q)) {
				Gui::build(Gui::hovering ? Gui::hovering->spec: NULL);
			}

			if (IsKeyReleased(KEY_R)) {
				if (Gui::placing) {
					Gui::placing->rotate();
				}
				else
				if (Gui::hovering) {
					Sim::locked([&]() {
						Entity::load(Gui::hovering->id)
							.rotate();
					});
				}
			}

			if (IsKeyReleased(KEY_E)) {
				Gui::popup = Gui::buildPopup;
			}

			if (Gui::mouse.left && Gui::mouse.leftChanged && Gui::placing) {
				Sim::locked([&]() {
					Entity::create(Entity::next(), Gui::placing->spec)
						.setGhost(true)
						.face(Gui::placing->dir)
						.move(Gui::placing->pos)
						.setGhost(false);
				});
			}

			if (IsKeyReleased(KEY_ESCAPE)) {
				if (Gui::popup) {
					Gui::popup = NULL;
				}
				else
				if (Gui::placing) {
					delete Gui::placing;
					Gui::placing = NULL;
				}
			}
		}

		if (Gui::popup) {
			Gui::popup->center();
			Gui::popup->update();
		}

		BeginDrawing();
			ClearBackground(BLANK);

			BeginMode3D(Gui::camera);
				DrawGrid(20,1);

				float size = (float)Chunk::size;
				float e = -(size/2.0f)-0.5f;
				for (auto pair: Chunk::all) {
					Chunk *chunk = pair.second;
					float x = chunk->x;
					float y = chunk->y;
					DrawModel(chunk->heightmap, Vector3Zero(), 1.0f, WHITE);
					DrawModel(
						waterCube,
						(Vector3){
							size*x + (size/2.0f),
							e,
							size*y + (size/2.0f)
						},
						1.0f*size,
						WHITE
					);
				}

				Gui::findEntities();

				for (auto ge: Gui::entities) {
					DrawModelEx(
						ge->spec->model,
						ge->pos.vec(),
						(Vector3){0,1,0},
						Directions::degrees(ge->dir),
						(Vector3){1,1,1},
						WHITE
					);
				}

				if (Gui::hovering) {
					Spec::Animation* animation = &spec->animations[Gui::hovering->dir];
					Vector3 bounds = (Vector3){animation->w, animation->h, animation->d};
					DrawCubeWiresV(Gui::hovering->pos.vec(), Vector3AddValue(bounds, 0.01), WHITE);
				}

				if (Gui::placing) {
					BeginBlendMode(BLEND_ADDITIVE);
						DrawModelEx(
							Gui::placing->spec->model,
							Gui::placing->pos.vec(),
							(Vector3){0,1,0},
							Directions::degrees(Gui::placing->dir),
							(Vector3){1,1,1},
							WHITE
						);
					EndBlendMode();
				}

			EndMode3D();

			if (Gui::popup) {
				Gui::popup->draw();
			}

			DrawFPS(10,10);
		EndDrawing();
	}

	for (auto [name, spec]: Spec::all) {
		notef("Spec unload model: %s %s", name.c_str(), spec->obj.c_str());
		UnloadModel(spec->model);
	}

	for (auto pair: Chunk::all) {
		Chunk *chunk = pair.second;
		chunk->dropHeightMap();
	}

	UnloadModel(cube);
	UnloadModel(waterCube);

	delete mod;
	delete Gui::buildPopup;

	UnloadShader(shader);
	CloseWindow();
	return 0;
}