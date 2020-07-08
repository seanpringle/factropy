
#define GRAPHICS_API_OPENGL_33
#define GLSL_VERSION 330
#include "raylib.h"
#include "raymath.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#include <cstdio>

#define notef(...) { fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); }
#define fatalf(...) { notef(__VA_ARGS__); exit(EXIT_FAILURE); }

#define ensure(cond,...) if (!(cond)) { exit(EXIT_FAILURE); }
#define ensuref(cond,...) if (!(cond)) { notef(__VA_ARGS__); exit(EXIT_FAILURE); }

#include "spec.h"
#include "entity.h"

typedef struct {
	int x, y, dx, dy, wheel;
	bool left, leftChanged;
	bool middle, middleChanged;
	bool right, rightChanged;
} MouseState;

MouseState GetMouseState(MouseState last) {
	Vector2 pos = GetMousePosition();
	bool left = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
	bool middle = IsMouseButtonDown(MOUSE_MIDDLE_BUTTON);
	bool right = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
	return (MouseState) {
		.x = (int)pos.x,
		.y = (int)pos.y,
		.dx = (int)pos.x - last.x,
		.dy = (int)pos.y - last.y,
		.wheel = GetMouseWheelMove(),
		.left = left,
		.leftChanged = left != last.left,
		.middle = middle,
		.middleChanged = middle != last.middle,
		.right = right,
		.rightChanged = right != last.right,
	};
}

static float speedH = 0.005;
static float speedV = 0.005;

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

	Spec *spec = new Spec("container");
	spec->w = 2;
	spec->h = 2;
	spec->d = 5;
	spec->obj = "models/container.obj";

	Entities::create(Entities::next(), Specs::byName("container"));
	Entities::create(Entities::next(), Specs::byName("container")).move((Point){3,0,0});
	Entities::create(Entities::next(), Specs::byName("container")).move((Point){-3,0,0});

	SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_VSYNC_HINT|FLAG_MSAA_4X_HINT);
	InitWindow(1920,1080,"test8");
	SetTargetFPS(60);

	Camera3D camera = {
		.position = (Vector3){5,5,5},
		.target = (Vector3){0,0,0},
		.up = (Vector3){0,1,0},
		.fovy = 45,
		.type = CAMERA_PERSPECTIVE,
	};

	MouseState mouse = { 0 };

	Shader shader = LoadShader(
		FormatText("shaders/glsl%i/base_lighting.vs", GLSL_VERSION),
		FormatText("shaders/glsl%i/lighting.fs", GLSL_VERSION)
	);

	shader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
	shader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	int ambientLoc = GetShaderLocation(shader, "ambient");
	float ambientCol[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	SetShaderValue(shader, ambientLoc, &ambientCol, UNIFORM_VEC4);

	Light light = CreateLight(LIGHT_DIRECTIONAL, (Vector3){ -1, 1, 0 }, Vector3Zero(), WHITE, shader);

	for (auto [name, spec]: Specs::all) {
		notef("Spec model: %s %s", name.c_str(), spec->obj.c_str());
		spec->model = LoadPart(spec->obj.c_str(), shader, (Color){127,0,0,255});
	}

	while (!WindowShouldClose()) {
		mouse = GetMouseState(mouse);

		if (mouse.right) {
			float rH = speedH * mouse.dx;
			float rV = speedV * mouse.dy;
			Vector3 direction = Vector3Subtract(camera.position, camera.target);
			Vector3 right = Vector3CrossProduct(direction, camera.up);
			Matrix rotateH = MatrixRotate(camera.up, -rH);
			Matrix rotateV = MatrixRotate(right, rV);
			camera.position = Vector3Transform(camera.position, rotateH);
			camera.position = Vector3Transform(camera.position, rotateV);
		}

		if (mouse.wheel) {
			Vector3 delta = Vector3Scale(camera.position, mouse.wheel < 0 ? 0.25: -0.25);
			camera.position = Vector3Add(camera.position, delta);
		}

		SetCameraMode(camera, CAMERA_CUSTOM);

		UpdateLightValues(shader, light);

		float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
		SetShaderValue(shader, shader.locs[LOC_VECTOR_VIEW], cameraPos, UNIFORM_VEC3);

		BeginDrawing();
			ClearBackground(BLACK);
			BeginMode3D(camera);
				DrawGrid(10,1);
				for (auto en: Entities::all) {
					DrawModel(en.spec->model, en.pos.vec(), 1, WHITE);
				}
			EndMode3D();
		EndDrawing();
	}

	for (auto [name, spec]: Specs::all) {
		notef("Spec unload model: %s %s", name.c_str(), spec->obj.c_str());
		UnloadModel(spec->model);
	}

	UnloadShader(shader);
	CloseWindow();
	return 0;
}