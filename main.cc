
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

	SetTraceLogLevel(LOG_WARNING);
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

	float ambientCol[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

	Shader shader = LoadShader(
		FormatText("shaders/glsl%i/base_lighting.vs", GLSL_VERSION),
		FormatText("shaders/glsl%i/lighting.fs", GLSL_VERSION)
	);

	shader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
	shader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	SetShaderValue(shader, GetShaderLocation(shader, "ambient"), &ambientCol, UNIFORM_VEC4);

	Shader pshader = LoadShader(
		FormatText("shaders/glsl%i/base_lighting_instanced.vs", GLSL_VERSION),
		FormatText("shaders/glsl%i/lighting.fs", GLSL_VERSION)
	);

	pshader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(pshader, "model");
	pshader.locs[LOC_MATRIX_VIEW] = GetShaderLocation(pshader, "view");
	pshader.locs[LOC_MATRIX_PROJECTION] = GetShaderLocation(pshader, "projection");
	pshader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(pshader, "viewPos");

	SetShaderValue(pshader, GetShaderLocation(pshader, "ambient"), &ambientCol, UNIFORM_VEC4);

	Part::shader = pshader;
	Part::material = LoadMaterialDefault();

	Light lightA = CreateLight(LIGHT_DIRECTIONAL, (Vector3){ -1, 1, 0 }, Vector3Zero(), WHITE, shader);
	Light lightB = CreateLight(LIGHT_DIRECTIONAL, (Vector3){ -1, 1, 0 }, Vector3Zero(), WHITE, pshader);

	//Sim::seed(879600773);
	Sim::seed(1);

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
	spec->vehicle = false;

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
								0.5,
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

	Model cube = LoadModelFromMesh(GenMeshCube(1.0f,1.0f,1.0f));
	cube.materials[0].shader = shader;

	Model waterCube = LoadModelFromMesh(GenMeshCube(1.0f,1.0f,1.0f));
	waterCube.materials[0].shader = pshader;
	waterCube.materials[0].maps[MAP_DIFFUSE].color = GetColor(0x010190FF);

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

		UpdateLightValues(shader, lightA);
		UpdateLightValues(pshader, lightB);

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

			if (IsKeyReleased(KEY_T) && Gui::hovering) {
				Gui::camera.target = Gui::hovering->pos.vec();
			}

			if (IsKeyReleased(KEY_E)) {
				Gui::popup = (Gui::popup && Gui::popup == Gui::buildPopup) ? NULL: Gui::buildPopup;
			}

			if (IsKeyReleased(KEY_G)) {
				Gui::showGrid = !Gui::showGrid;
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

			if (IsKeyReleased(KEY_DELETE) && Gui::hovering) {
				Sim::locked([&]() {
					int id = Gui::hovering->id;
					if (Entity::exists(id)) {
						Entity::get(id).destroy();
					};
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

		for (auto spec: Spec::all) {
			for (auto part: spec.second->parts) {
				part->update();
			}
		}

		BeginDrawing();
			ClearBackground(SKYBLUE);

			BeginMode3D(Gui::camera);

				if (Gui::showGrid) {
					Vector3 groundZero = {
						std::floor(Gui::camera.target.x),
						std::floor(0),
						std::floor(Gui::camera.target.z),
					};

					Gui::drawGrid(groundZero, 64, 1);
				}

				std::vector<Matrix> water;

				float size = (float)Chunk::size;
				for (auto pair: Chunk::all) {
					Chunk *chunk = pair.second;
					if (!chunk->heightmap.meshes) {
						chunk->genHeightMap();
						chunk->heightmap.materials[MAP_DIFFUSE].shader = shader;
					}
					float x = chunk->x;
					float y = chunk->y;
					DrawModel(chunk->heightmap, (Vector3){0,-21.81,0}, 1.0f, WHITE);
					water.push_back(
						MatrixMultiply(
							MatrixTranslate(x+0.5f, -0.52f, y+0.5f),
							MatrixScale(size,size,size)
						)
					);
				}

				rlDrawMeshInstanced(waterCube.meshes[0], waterCube.materials[0], water.size(), water.data());

				Gui::findEntities();

				std::map<Part*,std::vector<Matrix>> batches;

				for (auto ge: Gui::entities) {
					Matrix t = ge->transform();
					for (auto part: ge->spec->parts) {
						batches[part].push_back(t);
					}
				}

				for (auto pair: batches) {
					Part *part = pair.first;
					part->drawInstanced(pair.second.size(), pair.second.data());
				}

				if (Gui::hovering) {
					Spec::Animation* animation = &Gui::hovering->spec->animations[Gui::hovering->dir];
					Vector3 bounds = (Vector3){animation->w, animation->h, animation->d};
					DrawCubeWiresV(Gui::hovering->pos.vec(), Vector3AddValue(bounds, 0.01), WHITE);
				}

				if (Gui::placing) {
					Matrix t = Gui::placing->transform();
					for (auto part: Gui::placing->spec->parts) {
						part->drawGhost(t);
					}
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

	CloseWindow();
	return 0;
}