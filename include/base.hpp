#pragma once

#include "person.hpp"
#include "tile.hpp"
#include "sdl2.hpp"
#include "building.hpp"

#include <SDL.h>

#include <random>
#include <string>
#include <vector>

class Base
{
public:
    static Base& get();

public:
    Base(Base const&) = delete;
    void operator=(Base const&) = delete;

public:
    void display_resources();
	void display_scene();
    void display_shop();

    void handle_mouse_pressed(int x, int y);
	void handle_mouse_released(int x, int y);
	
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

private:
    Base();

    std::vector<std::vector<Tile>> tiles;
    std::vector<Person> farmers;
    std::vector<Building> shop_buildings;
    std::vector<Building> base_buildings;
    // index of building being placed
    int place;

    sdl2::Text text_build;
};
