
#define GRAPHICS_API_OPENGL_33
#define GLSL_VERSION 330
#include "raylib.h"
#include "raymath.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#include "common.h"
#include "gui.h"
#include "panel.h"
#include "mod.h"
#include "sim.h"
#include "chunk.h"
#include "spec.h"
#include "entity.h"

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

	Part::shader = shader;

	Light light = CreateLight(LIGHT_DIRECTIONAL, (Vector3){ -1, 1, 0 }, Vector3Zero(), WHITE, shader);

	Sim::seed(879600773);

	Spec *spec = new Spec("provider-container");
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
		new Part("models/container.stl", GetColor(0x009900ff)),
	};
	spec->align = true;
	spec->rotate = false;
	spec->rotateGhost = true;

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

	spec->animations[North] = spec->animations[South];
	spec->animations[East] = spec->animations[South];
	spec->animations[West] = spec->animations[South];

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
	}

	Model cube = LoadModelFromMesh(GenMeshCube(1.0f,1.0f,1.0f));
	cube.materials[0].shader = shader;

	Model waterCube = LoadModelFromMesh(GenMeshCube(1.0f,1.0f,1.0f));
	waterCube.materials[0].maps[MAP_DIFFUSE].color = GetColor(0x010190FF);

	if (loadSave) {
		Sim::load("autosave");
	}

	if (!loadSave) {
		for (int cy = -5; cy < 5; cy++) {
			for (int cx = -5; cx < 5; cx++) {
				Chunk::get(cx,cy);
			}
		}
	}

	Panels::init();
	Gui::buildPopup = new BuildPopup(800, 800);
	Gui::entityPopup = new EntityPopup(800, 800);

	Mod* mod = new Mod("base");
	mod->load();

	while (!WindowShouldClose()) {
		
		Sim::locked(Sim::update);
		mod->update();

		Gui::updateMouseState();
		Gui::updateCamera();

		UpdateLightValues(shader, light);

		float cameraPos[3] = { Gui::camera.position.x, Gui::camera.position.y, Gui::camera.position.z };
		SetShaderValue(shader, shader.locs[LOC_VECTOR_VIEW], cameraPos, UNIFORM_VEC3);

		if (!Gui::popup || !Gui::popup->contains(Gui::mouse.x, Gui::mouse.y)) {
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
						Entity::get(Gui::hovering->id)
							.rotate();
					});
				}
			}

			if (IsKeyReleased(KEY_E)) {
				Gui::popup = (Gui::popup && Gui::popup == Gui::buildPopup) ? NULL: Gui::buildPopup;
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

			if (Gui::mouse.left && Gui::mouse.leftChanged && !Gui::placing && Gui::hovering) {
				Gui::popup = Gui::entityPopup;
				Sim::locked([&]() {
					GuiEntity *ge = new GuiEntity(Gui::hovering->id);
					Gui::entityPopup->useEntity(ge);
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

			if (IsKeyReleased(KEY_F5)) {
				Sim::save("autosave");
			}
		}

		if (Gui::popup) {
			Gui::popup->update();
		}

		BeginDrawing();
			ClearBackground(BLANK);

			BeginMode3D(Gui::camera);
				DrawGrid(64,1);

				float size = (float)Chunk::size;
				float e = -(size/2.0f)-0.5f;
				for (auto pair: Chunk::all) {
					Chunk *chunk = pair.second;
					if (!chunk->heightmap.meshes) {
						chunk->genHeightMap();
						chunk->heightmap.materials[MAP_DIFFUSE].shader = shader;
					}
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
					Matrix t = ge->transform();
					for (auto part: ge->spec->parts) {
						part->draw(t);
					}
				}

				if (Gui::hovering) {
					Spec::Animation* animation = &Gui::hovering->spec->animations[Gui::hovering->dir];
					Vector3 bounds = (Vector3){animation->w, animation->h, animation->d};
					DrawCubeWiresV(Gui::hovering->pos.vec(), Vector3AddValue(bounds, 0.01), WHITE);
				}

				if (Gui::placing) {
					BeginBlendMode(BLEND_ADDITIVE);
						Matrix t = Gui::placing->transform();
						for (auto part: Gui::placing->spec->parts) {
							part->draw(t);
						}
					EndBlendMode();
				}

			EndMode3D();

			if (Gui::popup) {
				Gui::popup->draw();
			}

			DrawFPS(10,10);
		EndDrawing();
	}

//	for (auto [name, spec]: Spec::all) {
//		notef("Spec unload model: %s %s", name.c_str(), spec->obj.c_str());
//		UnloadModel(spec->model);
//	}

	UnloadModel(cube);
	UnloadModel(waterCube);
	UnloadShader(shader);

	for (auto pair: Chunk::all) {
		Chunk *chunk = pair.second;
		chunk->dropHeightMap();
	}

	delete mod;
	delete Gui::buildPopup;

	Gui::reset();
	Entity::reset();
	Chunk::reset();
	Spec::reset();

	Part::test();

	CloseWindow();
	return 0;
}