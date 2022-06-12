#pragma once

#include "sdl2.hpp"
#include "item.hpp"

#include <memory>
#include <string>
#include <vector>
#include <set>

class Building
{
public:
	Building(std::string const& _img, sdl2::Dimension const _dim,
		int const _height_d, int const _cost_gold, int const _cost_wood,
		int const _cost_stone, int const _cost_iron);

public:
	void display_building(bool const transparent) const;
	void display_backdrop(SDL_Color const& clr) const;
	void display_placement_options() const;
	bool is_pressed(int x, int y) const;
	bool can_buy(int gold, int wood, int stone, int iron) const;

	virtual void add_resources();
	virtual void display_item() const;
	virtual void collect_item(int& gold, int& wheat, int& wood, int& stone, int& iron);
	virtual void display_item_collect();
	virtual bool is_item_cap() const;
	virtual bool is_item_pressed(int mx, int my) const;

public:
	virtual std::shared_ptr<Building> create_building(bool shrink, int x, int y) const;
	
public:
	bool operator < (Building const& _building) const;

public:
	std::string img;
	sdl2::Dimension dim;
	int height_d;
	int cost_gold, cost_wood, cost_stone, cost_iron;

public:
	int id;
	static int inc;
};

enum class ProdType
{
	GOLD,
	WHEAT,
	WOOD,
	STONE,
	IRON
};

class ProdBuilding : public Building
{
public:
	ProdBuilding(std::string const& _img, sdl2::Dimension const _dim,
		int const _height_d, int const _cost_gold, int const _cost_wood, int const _cost_stone,
		int const _cost_iron, ProdType const _type, int const _rate, int const _display_cap,
		int const _storage_cap);

public:
	void add_resources() override;
	void display_item() const override;
	void collect_item(int& gold, int& wheat, int& wood, int& stone, int& iron) override;
	void display_item_collect() override;
	bool is_item_cap() const override;
	bool is_item_pressed(int mx, int my) const override;

public:
	std::shared_ptr<Building> create_building(bool shrink, int x, int y) const override;

private:
	ProdType type;
	int rate;
	int amount;

	int display_cap;
	int storage_cap;

	std::vector<Item> collect_items;
};