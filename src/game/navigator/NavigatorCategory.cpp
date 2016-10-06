#include "stdafx.h"
#include "game/navigator/NavigatorCategory.h"


NavigatorCategory::NavigatorCategory(int id, std::string name, int minimumRank) :
    id(id), name(name), minimumRank(minimumRank)
{
}


NavigatorCategory::~NavigatorCategory()
{
}
