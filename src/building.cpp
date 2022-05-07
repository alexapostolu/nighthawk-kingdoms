#include "building.hpp"

bool Building::operator < (Building const& _building) const
{
	return dim.y < _building.dim.y || (dim.y == _building.dim.y && dim.x <= _building.dim.x);
}