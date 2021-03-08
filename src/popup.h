#pragma once

// ImGUI popup windows.

#include "view.h"
#include "tech.h"
#include "recipe.h"
#include "item.h"
#include "fluid.h"

struct Popup {
	MainCamera* camera = nullptr;
	bool visible = false;
	bool opened = false;
	bool mouseOver = false;
	bool inputFocused = false;
	Popup(MainCamera* c);
	virtual ~Popup();
	virtual void draw();
	void show(bool state = true);
	void center(int w, int h);
	void bottomLeft(int w, int h);
};

struct MessagePopup : Popup {
	std::string text;
	MessagePopup(MainCamera* c);
	~MessagePopup();
	void draw();
};

struct StatsPopup2 : Popup {
	StatsPopup2(MainCamera* c);
	~StatsPopup2();
	void draw();
};

struct EntityPopup2 : Popup {
	uint eid = 0;
	char name[50];
	EntityPopup2(MainCamera* c);
	~EntityPopup2();
	void draw();
	void useEntity(uint eid);
};

struct RecipePopup : Popup {
	bool showUnavailable = false;

	struct {
		Item* item = nullptr;
		Fluid* fluid = nullptr;
		Recipe* recipe = nullptr;
		Spec* spec = nullptr;
		Tech* tech = nullptr;
	} locate;

	struct {
		std::map<Item*,bool> item;
		std::map<Fluid*,bool> fluid;
		std::map<Recipe*,bool> recipe;
		std::map<Spec*,bool> spec;
		std::map<Tech*,bool> tech;
	} highlited;

	struct {
		std::map<Item*,bool> item;
		std::map<Fluid*,bool> fluid;
		std::map<Recipe*,bool> recipe;
		std::map<Spec*,bool> spec;
		std::map<Tech*,bool> tech;
	} expanded;

	struct {
		std::vector<Item*> items;
		std::vector<Fluid*> fluids;
		std::vector<Recipe*> recipes;
		std::vector<Spec*> specs;
		std::vector<Tech*> techs;
	} sorted;

	RecipePopup(MainCamera* c);
	~RecipePopup();
	void draw();
	void drawItem(Item* item);
	void drawItemButton(Item* item, int count);
	void drawFluid(Fluid* fluid);
	void drawFluidButton(Fluid* fluid, int count);
	void drawRecipe(Recipe* recipe);
	void drawRecipeButton(Recipe* recipe);
	void drawSpec(Spec* spec);
	void drawSpecButton(Spec* spec);
	void drawTech(Tech* tech);
	void drawTechButton(Tech* tech);
};
