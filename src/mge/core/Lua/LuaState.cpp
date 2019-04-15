#include "LuaState.h"
#include "mge/config.hpp"

#include <iostream>

using namespace lua;

LuaState::LuaState() : L(luaL_newstate()), _shouldCloseOnDestroy(true)
{
    luaL_openlibs(L);
}

LuaState::~LuaState()
{
    if (_shouldCloseOnDestroy)
    {
        std::cout << "Closing one Lua State" << std::endl;
        lua_close(L);
    }
}

void LuaState::LogLuaError(lua_State* lua, const char * msg)
{
    fprintf(stderr, "\nFATAL ERROR:\n  %s: %s\n\n",
            msg, lua_tostring(lua, -1));
}

bool LuaState::LoadFile(std::string const& filename)
{
    int errorCode = luaL_loadfile(L, filename.c_str());
    if (errorCode != 0) // LUA_OK
    {
        LogLuaError(L, ("FATAL ERROR: Cannot Open file: " + filename).c_str());
        return false;
    }
    return true;
}

bool LuaState::Execute()
{
    if (lua_pcall(L, 0, 0, 0) != LUA_OK)
    {
        printf("Error: %s\n", lua_tostring(L, -1));
        return false;
    }

    lua_settop(L, 0);
    return true;
}