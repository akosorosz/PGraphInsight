#include "pnsproblem.h"

namespace PnsTools {

std::string getMaterialNamesString(const MaterialSet &materials)
{
	std::string str;
	bool first=true;
	for (Material material : materials)
	{
		if (!first) str+=", ";
		else first=false;
		str+=material->name;
	}
	return str;
}

std::string getUnitNamesString(const OperatingUnitSet &units)
{
	std::string str;
	bool first=true;
	for (OperatingUnit unit : units)
	{
		if (!first) str+=", ";
		else first=false;
		str+=unit->name;
	}
	return str;
}

} // namespace PnsTools
