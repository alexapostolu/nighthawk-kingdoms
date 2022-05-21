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
	void display_scene(bool second);
    void display_shop();

    void handle_mouse_pressed(int x, int y);
    void handle_mouse_dragged(int x, int y);
	void handle_mouse_released(int x, int y);
	
private:
    Base();

    // 0 - yes, 1 - occupied, 2 - out of bounds
    int can_place_building(Building const& b) const;
    void update_base_buildings(Building* building, bool shrink = false, int x = -1, int y = -1);
    void display_farmers();
    void display_base_buildings();
    void manage_resources(bool second);
    void display_grid();

public:
	int gold, wheat, wood, stone, iron, gems;
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
    struct shared_ptr_comp
    {
        bool operator() (std::shared_ptr<Building> const& a,
                         std::shared_ptr<Building> const& b) const;
    };

private:
    std::vector<std::vector<Tile>> tiles;
    std::vector<Person> farmers;
    std::vector<std::unique_ptr<Building>> shop_buildings;

	std::set<std::shared_ptr<Building>, shared_ptr_comp> base_buildings;
    std::shared_ptr<Building> place;
    PlaceState place_state;
    SDL_Point place_offset; // so when mouse dragged it doesn't teleport to mouse

    sdl2::Text text_build;
    int not_enough_resources;
    sdl2::Text text_not_enough_resources;
};
