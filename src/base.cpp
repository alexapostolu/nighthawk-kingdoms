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

Base& Base::get()
{
	static Base instance;
	return instance;
}

Base::Base()
	: gold(750), wheat(250), wood(500), stone(0), gems(10)
	, level(1), exp(0), troph(0)
	, edit_mode(false), shop_state(ShopState::HIDDEN)
	, TILES_X(58), TILES_Y(23)
	, tiles(TILES_Y, std::vector<Tile>(TILES_X, Tile{ TileState::GRASS }))
	, place(-1)
	, text_build(Screen::get().SCREEN_WIDTH - 20, Screen::get().SCREEN_HEIGHT - 65, sdl2::Align::RIGHT)
{
	farmers.push_back(Person{ { TILES_X / 2, TILES_Y / 2 }, { TILES_X / 2.f * 20 + 5, TILES_Y / 2.f * 20 + 60 } });
	farmers.push_back(Person{ { TILES_X / 2 - 5, TILES_Y / 2 + 7 }, { (TILES_X / 2.f - 5 ) * 20 + 5, (TILES_Y / 2.f + 7) * 20 + 60 } });
}

void Base::set_building_dim()
{
	auto [x1, y1] = Screen::get().get_img_dim("farmhouse.png");
	shop_buildings.push_back({ "farmhouse.png",  { 200, Screen::get().SCREEN_HEIGHT - 150,
		(int)(x1 * 0.25), (int)(y1 * 0.25) }, 3, 100, 0, 0 });

	auto [x2, y2] = Screen::get().get_img_dim("lumbermill.png");
	shop_buildings.push_back({ "lumbermill.png", { 600, Screen::get().SCREEN_HEIGHT - 150,
		(int)(x2 * 0.25), (int)(y2 * 0.25) }, 0, 100, 0, 0});

	auto [x3, y3] = Screen::get().get_img_dim("road.png");
	shop_buildings.push_back({ "road.png",		 { 1000, Screen::get().SCREEN_HEIGHT - 150,
		(int)(x3 * 0.25), (int)(y3 * 0.25) }, 0, 10, 0, 0});
}

void Base::display_resources()
{
	Screen::get().rect(0, 0, Screen::get().SCREEN_WIDTH, 50, sdl2::clr_black, sdl2::clr_clear);
	Screen::get().rect(0, 50, Screen::get().SCREEN_WIDTH, 5, sdl2::clr_yellow, sdl2::clr_clear);

	std::string str[] = {
		"Gold: " + std::to_string(Base::get().gold),
		"Wheat: " + std::to_string(Base::get().wheat),
		"Wood: " + std::to_string(Base::get().wood),
		"Gems: " + std::to_string(Base::get().gems),
	};

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

	for (int i = 0; i < sizeof(str) / sizeof(str[0]); ++i)
	{
		Screen::get().text(str[i], sdl2::clr_yellow, sdl2::str_brygada, 24,
			(Screen::get().SCREEN_WIDTH / 4 * i) + margin, 10, sdl2::Align::LEFT);
	}
}

void Base::display_scene()
{
	static int tick = 0;

	float spd = 0.5f;
	static int step_size = 20 / spd;

	for (auto& farmer : farmers)
	{
		Screen::get().image("farmer.png",
			(int)farmer.actual_pos.x, (int)farmer.actual_pos.y, 100, 60, sdl2::Align::CENTER);

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

	// display tile grid when placing buildings
	if (place != -1)
	{
		for (int i = 0; i < TILES_Y; ++i)
		{
			for (int j = 0; j < TILES_X; ++j)
				Screen::get().rect((j * 20) + 5, (i * 20) + 60, 20, 20,
					tiles[i][j].state == TileState::OCCUPIED ? sdl2::clr_black : sdl2::clr_clear, sdl2::clr_white);
		}
	}

	for (std::size_t i = 0; i < base_buildings.size(); ++i)
	{
		if (i == place)
			continue;

		auto const& [img, dim, height_d, cost_g, cost_w, cost_s] = base_buildings[i];
		Screen::get().image(img, dim.x, dim.y, dim.w, dim.h, sdl2::Align::CENTER);

		if (img == "lumbermill.png" && tick > 500)
		{
			gold++;
			wood++;
			tick = 0;
		}
		tick++;
	}

	if (place != -1)
	{
		auto& [img, dim, height_d, cost_g, cost_w, cost_s] = base_buildings[place];

		int prev_x = dim.x;
		int prev_y = dim.y;

		if (img == "road.png")
		{
			dim.x += 10;
			dim.y += 10;
		}

		int x1 = ((dim.x - (dim.w / 2)) - 5) / 20;
		int x2 = ((dim.x + (dim.w / 2)) - 5) / 20;
		int y1 = ((dim.y - (dim.h / 2) + (height_d * 20)) - 60) / 20;
		int y2 = ((dim.y + (dim.h / 2)) - 60) / 20;

		bool can_place = true;
		bool out = false;
		for (int i = x1; i <= x2; ++i)
		{
			for (int j = y1; j <= y2; ++j)
			{
				if (j < 0 || j >= tiles.size() || i < 0 || i >= tiles[j].size())
				{
					out = true;
					break;
				}
				if (tiles[j][i].state == TileState::OCCUPIED)
				{
					can_place = false;
					break;
				}
			}
		}

		if (out)
		{
			dim.x = prev_x;
			dim.y = prev_y;
			can_place = false;
		}

		int rect_w = std::ceil(dim.w / 20.0);
		rect_w = (rect_w + (rect_w % 2)) * 20;
		
		int rect_h = std::ceil(dim.h / 20.0);
		rect_h = (rect_h + (rect_h % 2)) * 20;

		int h = height_d * 20;
		Screen::get().rect(dim.x, dim.y + (h / 2), rect_w, rect_h - h, can_place ? sdl2::clr_green : sdl2::clr_red,
			sdl2::clr_clear, sdl2::Align::CENTER);

		Screen::get().image(img, dim.x, dim.y, dim.w, dim.h, sdl2::Align::CENTER);

		int const base = dim.y + (dim.h / 2);
		Screen::get().image("checkmark.png", dim.x - 40, base + 30, 40, 40, sdl2::Align::CENTER);
		Screen::get().image("x.png",		 dim.x + 40, base + 30, 40, 40, sdl2::Align::CENTER);
	}
}

void Base::display_shop()
{
	static int shop_y = Screen::get().SCREEN_HEIGHT;
	const int shop_h = Screen::get().SCREEN_HEIGHT - 300;
	const int shop_spd = 15;

	switch (shop_state)
	{
	case ShopState::HIDDEN: {
		if (place == -1)
		{
			Screen::get().text("BUILD", sdl2::clr_white, sdl2::str_brygada, 45,
				text_build.dim.x, text_build.dim.y, text_build.align);

			shop_y = Screen::get().SCREEN_HEIGHT;
		}

		shop_y = Screen::get().SCREEN_HEIGHT;

		break;
	}
	case ShopState::APPEARING: {
		Screen::get().rect(0, shop_y, Screen::get().SCREEN_WIDTH, Screen::get().SCREEN_HEIGHT - shop_h, sdl2::clr_black, sdl2::clr_clear);

		shop_y -= shop_spd;
		if (shop_y <= shop_h)
		{
			shop_state = ShopState::VISIBLE;
			shop_y = shop_h;
		}

		break;
	}
	case ShopState::VISIBLE: {
		Screen::get().rect(0, shop_h, Screen::get().SCREEN_WIDTH, Screen::get().SCREEN_HEIGHT - shop_h, sdl2::clr_black, sdl2::clr_clear);

		Screen::get().text("CLOSE", sdl2::clr_white, sdl2::str_brygada, 35,
			Screen::get().SCREEN_WIDTH - 15, shop_h - 40, sdl2::Align::RIGHT);

		for (auto const& building : shop_buildings)
		{
			auto const& [img, dim, height_d, cost_g, cost_w, cost_s] = building;
			Screen::get().image(img, dim.x, dim.y, dim.w, dim.h, sdl2::Align::CENTER);
		}

		break;
	}
	case ShopState::DISAPPEARING: {
		Screen::get().rect(0, shop_y, Screen::get().SCREEN_WIDTH, Screen::get().SCREEN_HEIGHT - shop_h, sdl2::clr_black, sdl2::clr_clear);

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
		if (text_build.clicked_on(x, y) && place == -1)
		{
			shop_state = ShopState::APPEARING;
		}
		else if (place != -1)
		{
			auto& [img, dim, height_d, cost_g, cost_w, cost_s] = base_buildings[place];
			int const base = dim.y + (dim.h / 2);

			if (std::sqrt(std::pow(x - (dim.x - 40), 2) + std::pow(y - (base+ 30), 2)) <= 20)
			{
				if (img == "road.png")
				{
					tiles[(dim.y - 60) / 20][(dim.x - 5) / 20].state = TileState::PATH;

					gold -= cost_g;
					wood -= cost_w;
					stone -= cost_s;
					place = -1;
					return;
				}

				int x1 = ((dim.x - (dim.w / 2)) - 5) / 20;
				int x2 = ((dim.x + (dim.w / 2)) - 5) / 20;
				int y1 = ((dim.y - (dim.h / 2) + (height_d * 20)) - 60) / 20;
				int y2 = ((dim.y + (dim.h / 2)) - 60) / 20;

				bool can_place = true;
				for (int i = x1; i <= x2; ++i)
				{
					for (int j = y1; j <= y2; ++j)
					{
						if (i < 0 || i >= tiles[j].size() || j < 0 || j >= tiles.size())
						{
							can_place = false;
							break;
						}
						if (tiles[j][i].state == TileState::OCCUPIED)
						{
							can_place = false;
							break;
						}
					}
				}

				if (can_place)
				{
					for (int i = x1; i <= x2; ++i)
					{
						for (int j = y1; j <= y2; ++j)
							tiles[j][i].state = TileState::OCCUPIED;
					}

					gold -= cost_g;
					wood -= cost_w;
					stone -= cost_s;
					place = -1;
					place_state = PlaceState::STATIONERY;
				}
			}
			else if (std::sqrt(std::pow(x - (dim.x + 40), 2) + std::pow(y - (base + 30), 2)) <= 20)
			{
				base_buildings.pop_back();

				place = -1;
				shop_state = ShopState::APPEARING;
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
	if (shop_state == ShopState::VISIBLE)
	{
		for (auto const& building : shop_buildings)
		{
			auto const& [img, dim, height_d, cost_g, cost_w, cost_s] = building;
			if (x >= dim.x - (dim.w / 2) && x <= dim.x + (dim.w / 2) &&
				y >= dim.y - (dim.h / 2) && y <= dim.y + (dim.h / 2))
			{
				base_buildings.push_back({ img, { dim.x, dim.y, (int)(dim.w * 0.6), (int)(dim.h * 0.6) }, height_d, cost_g, cost_w, cost_s });
				place = base_buildings.size() - 1;

				place_state = PlaceState::FOLLOW_MOUSE;
				shop_state = ShopState::HIDDEN;
				break;
			}
		}

		return;
	}

	if (place != -1)
	{
		auto const& [img, dim, height_d, cost_g, cost_w, cost_s] = base_buildings[place];
		if (x >= dim.x - (dim.w / 2) && x <= dim.x + (dim.w / 2) &&
			y >= dim.y - (dim.h / 2) && y <= dim.y + (dim.h / 2))
		{
			place_state = PlaceState::FOLLOW_MOUSE;
		}

		if (shop_state == ShopState::HIDDEN && place_state == PlaceState::FOLLOW_MOUSE)
		{
			auto& [img, dim, height_d, cost_g, cost_w, cost_s] = base_buildings[place];

			dim.x = ((x - 5) / 20) * 20 + 5;
			dim.y = (y / 20) * 20;
		}
	}
}

void Base::handle_mouse_released(int x, int y)
{
	place_state = PlaceState::STATIONERY;
}
