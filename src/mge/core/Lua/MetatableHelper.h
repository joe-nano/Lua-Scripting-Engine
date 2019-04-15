#pragma once

#include <string>
#include "LuaStack.h"

namespace lua
{
    /*! \return The meta table name for type t*/
    std::string GetMetaTableName(rttr::type const& t);

    int CreateUserDatum(lua_State* L);
    int CreateUserDatumFromVariant(lua_State* L, rttr::variant const& v);
    int DestroyUserDatum(lua_State* L);

    int IndexUserDatum(lua_State* L);
    int NewIndexUserDatum(lua_State* L);

    int InvokeFuncOnUserDatum(lua_State* L);
}