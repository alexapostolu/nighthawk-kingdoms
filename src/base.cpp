#include "base.hpp"
#include "screen.hpp"
#include "person.hpp"
#include "tile.hpp"
#include "building.hpp"
#include "sdl2.hpp"

#include <cassert>
#include <iostream>
#include <iomanip>
#include <random>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

Base& Base::get()
{
	static Base instance;
	return instance;
}

Base::Base()
	: gold(250), wheat(250), wood(500), stone(0), gems(10)
	, level(1), exp(0), troph(0)
	, edit_mode(false), shop_state(ShopState::HIDDEN)
	, TILES_X(58), TILES_Y(23)
	, tiles(TILES_Y, std::vector<Tile>(TILES_X, Tile{ TileState::GRASS }))
	, place(nullptr)
	, text_build("BUILD", Screen::get().SCREEN_WIDTH - 20, Screen::get().SCREEN_HEIGHT - 65, sdl2::Align::RIGHT)
{
	farmers.push_back(Person{ { TILES_X / 2, TILES_Y / 2 }, { TILES_X / 2.f * 20 + 5, TILES_Y / 2.f * 20 + 60 } });
	farmers.push_back(Person{ { TILES_X / 2 - 5, TILES_Y / 2 + 7 }, { (TILES_X / 2.f - 5 ) * 20 + 5, (TILES_Y / 2.f + 7) * 20 + 60 } });
}

void Base::set_building_dim()
{
	auto [x1, y1] = Screen::get().get_img_dim("farmhouse.png");
	shop_buildings.push_back(std::make_unique<ProdBuilding>(
		"farmhouse.png",
		sdl2::Dimension{200, Screen::get().SCREEN_HEIGHT - 150,
			(int)(x1 * 0.25), (int)(y1 * 0.25) },
		2, 100, 0, 0, 0,
		ProdType::WHEAT, 3, 6, 100
	));

	auto [x2, y2] = Screen::get().get_img_dim("lumbermill.png");
	shop_buildings.push_back(std::make_unique<ProdBuilding>(
		"lumbermill.png",
		sdl2::Dimension{ 600, Screen::get().SCREEN_HEIGHT - 150,
			(int)(x2 * 0.25), (int)(y2 * 0.25) },
		0, 100, 0, 0, 0,
		ProdType::WOOD, 3, 10, 100
	));

	auto [x3, y3] = Screen::get().get_img_dim("road.png");
	shop_buildings.push_back(std::make_unique<Building>(
		"road.png",
		sdl2::Dimension{ 1000, Screen::get().SCREEN_HEIGHT - 150,
			(int)(x3 * 0.15), (int)(y3 * 0.25) },
		0, 10, 0, 0, 0
	));
}

void Base::display_resources()
{
	Screen::get().fill(sdl2::clr_black);
	Screen::get().stroke(sdl2::clr_clear);
	Screen::get().rect(0, 0, Screen::get().SCREEN_WIDTH, 50);

	Screen::get().fill(sdl2::clr_yellow);
	Screen::get().rect(0, 50, Screen::get().SCREEN_WIDTH, 5);

	std::string str[] = {
		"Gold: " + std::to_string(Base::get().gold),
		"Wheat: " + std::to_string(Base::get().wheat),
		"Wood: " + std::to_string(Base::get().wood),
		"Gems: " + std::to_string(Base::get().gems),
	};

	std::string imgs[] = { "gold.png", "wheat.png", "wood.png", "gems.png" };

	int pos[] = {
		0,
		(Screen::get().SCREEN_WIDTH / 4),
		(Screen::get().SCREEN_WIDTH / 4 * 2),
		(Screen::get().SCREEN_WIDTH / 4 * 3)
	};

	sdl2::font_ptr ttf_font(TTF_OpenFont(sdl2::str_brygada.c_str(), 24));
	int text_w, text_h;
	TTF_SizeText(ttf_font.get(), str[3].c_str(), &text_w, &text_h);
	int margin = (Screen::get().SCREEN_WIDTH - (Screen::get().SCREEN_WIDTH / 4 * 3 + text_w)) / 2;

	Screen::get().fill(sdl2::clr_yellow);
	for (int i = 0; i < sizeof(str) / sizeof(str[0]); ++i)
	{
		Screen::get().image(imgs[i],
			(Screen::get().SCREEN_WIDTH / 4 * i) + margin - 10, 4, 40, 40, sdl2::Align::RIGHT);

		Screen::get().text(str[i], sdl2::str_brygada, 24,
			(Screen::get().SCREEN_WIDTH / 4 * i) + margin , 10, sdl2::Align::LEFT);
	}
}

void Base::display_scene(bool second)
{
	display_farmers();

	if (place != nullptr)
		display_grid();

	display_base_buildings();
	manage_resources(second);

	if (place != nullptr)
		place->display_placement_options();

	not_enough_resources();

	
	Screen::get().stroke(sdl2::clr_black);
	Screen::get().fill(sdl2::clr_white);
	Screen::get().trig(200, 200, 250, 150, 250, 250,
		sdl2::Align::CENTER, sdl2::TrigQuad::MIDDLE);
}

void Base::display_shop()
{
	static int shop_y = Screen::get().SCREEN_HEIGHT;
	const int shop_h = Screen::get().SCREEN_HEIGHT - 300;
	const int shop_spd = 15;

	switch (shop_state)
	{
	case ShopState::HIDDEN: {
		if (place == nullptr)
		{
			Screen::get().fill(sdl2::clr_white);
			Screen::get().text("BUILD", sdl2::str_brygada, 45,
				text_build.dim.x, text_build.dim.y, text_build.align);
		}

		shop_y = Screen::get().SCREEN_HEIGHT;

		break;
	}
	case ShopState::APPEARING: {
		Screen::get().fill(sdl2::clr_black);
		Screen::get().stroke(sdl2::clr_clear);
		Screen::get().rect(0, shop_y, Screen::get().SCREEN_WIDTH, Screen::get().SCREEN_HEIGHT - shop_h);

		shop_y -= shop_spd;
		if (shop_y <= shop_h)
		{
			shop_state = ShopState::VISIBLE;
			shop_y = shop_h;
		}

		break;
	}
	case ShopState::VISIBLE: {
		Screen::get().fill(sdl2::clr_black);
		Screen::get().stroke(sdl2::clr_clear);
		Screen::get().rect(0, shop_h, Screen::get().SCREEN_WIDTH, Screen::get().SCREEN_HEIGHT - shop_h);

		Screen::get().fill(sdl2::clr_white);
		Screen::get().text("CLOSE", sdl2::str_brygada, 35,
			Screen::get().SCREEN_WIDTH - 15, shop_h - 40, sdl2::Align::RIGHT);

		for (auto const& building : shop_buildings)
			building->display_building(false);

		break;
	}
	case ShopState::DISAPPEARING: {
		Screen::get().fill(sdl2::clr_black);
		Screen::get().stroke(sdl2::clr_clear);
		Screen::get().rect(0, shop_y, Screen::get().SCREEN_WIDTH, Screen::get().SCREEN_HEIGHT - shop_h);

		shop_y += shop_spd;
		if (shop_y >= Screen::get().SCREEN_HEIGHT)
		{
			shop_state = ShopState::HIDDEN;
			shop_y = Screen::get().SCREEN_HEIGHT;
		}

		break;
	}
	}
}

void Base::handle_mouse_pressed(int x, int y)
{
	if (shop_state == ShopState::HIDDEN)
	{
		if (place == nullptr && text_build.clicked_on(x, y))
		{
			shop_state = ShopState::APPEARING;
		}
		else if (place != nullptr)
		{
			auto const building = *place.get();
			auto const& [img, dim, height_d, cost_g, cost_w, cost_s, cost_i, id] = building;
			int const base = dim.y - (dim.h / 2) - 30;
			
			if (std::sqrt(std::pow(x - (dim.x - 40), 2) + std::pow(y - base, 2)) <= 20 &&
				can_place_building(building) == 0)
			{
				if (!building.can_buy(gold, wood, stone, iron))
				{
					if (resources_msg.empty() || (dim.y - 30) - resources_msg.back().second.dim.y > 10)
					{
						resources_msg.push_back({
							dim.y - 130,
							sdl2::Text("Not enough resources", dim.x, dim.y - 30, sdl2::Align::CENTER)
						});
					}

					return;
				}

				int x1 = ((dim.x - (dim.w / 2)) - 5) / 20;
				int x2 = ((dim.x + (dim.w / 2)) - 5) / 20;
				int y1 = ((dim.y - (dim.h / 2) + (height_d * 20)) - 60) / 20;
				int y2 = ((dim.y + (dim.h / 2)) - 60) / 20;

				for (int i = y1; i <= y2; ++i)
				{
					for (int j = x1; j <= x2; ++j)
						tiles[i][j].state = img == "road.png" ? TileState::PATH : TileState::OCCUPIED;
				}

				gold -= cost_g;
				wood -= cost_w;
				stone -= cost_s;
				place = nullptr;
				place_state = PlaceState::STATIONERY;
			}
			else if (std::sqrt(std::pow(x - (dim.x + 40), 2) + std::pow(y - base, 2)) <= 20)
			{
				base_buildings.erase(place);
				place = nullptr;

				shop_state = ShopState::APPEARING;
			}
		}
		else if (place == nullptr)
		{
			for (auto const& building : base_buildings)
			{
				if (building->is_item_pressed(x, y))
					building->collect_item(gold, wheat, wood, stone, iron);
			}
		}
	}
	else if (shop_state == ShopState::VISIBLE)
	{
		if (x >= 1055 && x <= 1165 && y >= 190 && y <= 220)
		{
			shop_state = ShopState::DISAPPEARING;
			return;
		}
	}
}

void Base::handle_mouse_dragged(int x, int y)
{
	Screen::get().fill(sdl2::clr_green);
	Screen::get().stroke(sdl2::clr_clear);
	Screen::get().rhom(x, y, 100, 50);

	if (shop_state == ShopState::VISIBLE)
	{
		assert(place == nullptr);

		for (auto const& building : shop_buildings)
		{
			if (building->is_pressed(x, y))
			{
				update_base_buildings(building.get(), true);

				place_state = PlaceState::FOLLOW_MOUSE;
				shop_state = ShopState::HIDDEN;
				break;
			}
		}

		return;
	}

	if (place != nullptr)
	{
		if (place->is_pressed(x, y))
		{
			place_state = PlaceState::FOLLOW_MOUSE;
			place_offset = { x - place->dim.x, y - place->dim.y };
		}

		if (shop_state == ShopState::HIDDEN && place_state == PlaceState::FOLLOW_MOUSE)
		{
			if (can_place_building(*place) != 2)
				update_base_buildings(place.get(), false, x, y);
		}
	}
}

void Base::handle_mouse_released(int x, int y)
{
	place_state = PlaceState::STATIONERY;
}

int Base::can_place_building(Building const& b) const
{
	int x1 = ((b.dim.x - (b.dim.w / 2)) - 5) / 20;
	int x2 = ((b.dim.x + (b.dim.w / 2)) - 5) / 20;
	int y1 = ((b.dim.y - (b.dim.h / 2) + (b.height_d * 20)) - 60) / 20;
	int y2 = ((b.dim.y + (b.dim.h / 2)) - 60) / 20;

	bool can_place = true;
	bool out = false;
	for (int i = x1; i <= x2; ++i)
	{
		for (int j = y1; j <= y2; ++j)
		{
			if (i < 0 || i >= tiles[j].size() || j < 0 || j >= tiles.size())
			{
				out = true;
				break;
			}

			if (tiles[j][i].state == TileState::OCCUPIED)
				can_place = false;
		}
	}

	return out ? 2
			   : !can_place;
}

void Base::update_base_buildings(Building* building, bool shrink, int x, int y)
{
	if (place != nullptr)
		base_buildings.erase(place);

	place = building->create_building(shrink, x, y);
	base_buildings.insert(place);
}

void Base::display_farmers()
{
	float const spd = 0.5f;
	static int step_size = 20 / spd;

	for (auto& farmer : farmers)
	{
		Screen::get().image(
			"farmer.png",
			(int)farmer.actual_pos.x, (int)farmer.actual_pos.y, 100, 60,
			sdl2::Align::CENTER);

		if (farmer.path.empty())
			farmer.generate_path(tiles);

		auto const& dest = farmer.path[0];
		if (step_size == 0)
		{
			farmer.path_pos = farmer.path[0];
			farmer.path.pop_front();

			step_size = 20 / spd;
		}
		else
		{
			if (farmer.path_pos.x < dest.x)
				farmer.actual_pos.x += spd;
			else if (farmer.path_pos.x > dest.x)
				farmer.actual_pos.x -= spd;
			else if (farmer.path_pos.y < dest.y)
				farmer.actual_pos.y += spd;
			else if (farmer.path_pos.y > dest.y)
				farmer.actual_pos.y -= spd;

			step_size--;
		}
	}
}

void Base::display_base_buildings()
{
	for (auto& building : base_buildings)
	{
		if (building == place)
		{
			int can_place = can_place_building(*building);
			building->display_backdrop(!can_place ? sdl2::clr_green : sdl2::clr_red);
		}

		building->display_building(place != nullptr && building != place);
		building->display_item_collect();
	}
}

void Base::manage_resources(bool second)
{
	for (auto& building : base_buildings)
	{
		if (place == building)
			continue;

		if (second)
			building->add_resources();

		if (building->is_item_cap())
			building->display_item();
	}
}

void Base::not_enough_resources()
{
	for (auto it = resources_msg.begin(); it != resources_msg.end();)
	{
		auto const& [end, txt] = *it;

		float alpha = 1 - (end - txt.dim.y) / 100.0;

		Screen::get().fill(255, 255, 255, (uint8_t)(255 * alpha));
		Screen::get().text(txt.text, sdl2::str_brygada, 20,
			txt.dim.x, txt.dim.y, txt.align);

		it->second.dim.y--;

		if (txt.dim.y <= end + 5)
			it = resources_msg.erase(it);
		else
			++it;
	}
}

void Base::display_grid()
{
	int const sides = 9;
	int const w = 70;
	int const h = 50;

	int const x_base = Screen::get().SCREEN_WIDTH / 2;
	int const y_base = 60;

	for (int i = 0; i < sides + 1; ++i)
	{
		int const x0_off = i * (w / 2);
		int const y0_off = i * (h / 2);
		int const x1_off = ((w / 2) * sides) - (i * w / 2);
		int const y1_off = ((h / 2) * sides) + (i * h / 2);

		Screen::get().stroke(sdl2::clr_white);

		Screen::get().line(
			x_base - x0_off, y_base + y0_off,
			x_base + x1_off, y_base + y1_off);

		Screen::get().line(
			x_base + x0_off, y_base + y0_off,
			x_base - x1_off, y_base + y1_off);
	}
}

bool Base::shared_ptr_comp::operator() (std::shared_ptr<Building> const& a, std::shared_ptr<Building> const& b) const
{
	return *a < *b;
}