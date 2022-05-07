#pragma once

#include "person.hpp"
#include "tile.hpp"
#include "sdl2.hpp"
#include "building.hpp"

#include <SDL.h>

#include <functional>
#include <random>
#include <string>
#include <vector>
#include <set>

class Base
{
public:
    static Base& get();

public:
    Base(Base const&) = delete;
    void operator=(Base const&) = delete;

    void set_building_dim();

public:
    void display_resources();
	void display_scene();
    void display_shop();

    void handle_mouse_pressed(int x, int y);
    void handle_mouse_dragged(int x, int y);
	void handle_mouse_released(int x, int y);
	
private:
    Base();

    // 0 - yes, 1 - occupied, 2 - out of bounds
    int can_place_building(Building const& building) const;
    void update_base_buildings(Building const& building);
    void display_farmers();
    void display_buildings();

public:
	int gold, wheat, wood, stone, gems;
	int level, exp, troph;
	bool edit_mode;

    int const TILES_X, TILES_Y;

    enum class ShopState
    {
        HIDDEN,
        VISIBLE,
        APPEARING,
        DISAPPEARING
    } shop_state;

    enum class PlaceState
    {
        STATIONERY,
        FOLLOW_MOUSE
    };

private:
    std::vector<std::vector<Tile>> tiles;
    std::vector<Person> farmers;
    std::vector<Building> shop_buildings;

	std::set<Building> base_buildings;
    std::set<Building>::iterator place;
    PlaceState place_state;

    int gold_production, wheat_production, wood_production, stone_production;

    sdl2::Text text_build;
};
