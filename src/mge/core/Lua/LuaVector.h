#pragma once

#include <lua.hpp>

namespace lua
{
    namespace vec
    {
        inline static int CreateVector(lua_State* L)
        {
            printf("Create Vector 3D has been called \n");

            lua_newtable(L);
            lua_pushstring(L, "x");
            lua_pushnumber(L, 0);
            lua_settable(L, -3);

            lua_pushstring(L, "y");
            lua_pushnumber(L, 0);
            lua_settable(L, -3);

            lua_pushstring(L, "z");
            lua_pushnumber(L, 0);
            lua_settable(L, -3);

            luaL_getmetatable(L, "VectorMetaTable");

            assert(lua_istable(L, -1));
            lua_setmetatable(L, -2);
            return 1;
        }

        inline static int __add(lua_State* L)
        {
            printf("Vector __add was called \n");

            assert(lua_istable(L, -2)); // left table
            assert(lua_istable(L, -1)); // right table

            lua_getfield(L, -2, "x");
            lua_Number leftX = lua_tonumber(L, -1);

            lua_pop(L, 1);

            lua_getfield(L, -2, "y");
            lua_Number leftY = lua_tonumber(L, -1);

            lua_pop(L, 1);

            lua_getfield(L, -2, "z");
            lua_Number leftZ = lua_tonumber(L, -1);

            lua_pop(L, 1);

            lua_getfield(L, -1, "x");
            lua_Number rightX = lua_tonumber(L, -1);

            lua_pop(L, 1);

            lua_getfield(L, -1, "y");
            lua_Number rightY = lua_tonumber(L, -1);

            lua_pop(L, 1);

            lua_getfield(L, -1, "z");
            lua_Number rightZ = lua_tonumber(L, -1);

            lua_pop(L, 1);

            lua_Number xAdded = leftX + rightX;
            lua_Number yAdded = leftY + rightY;
            lua_Number zAdded = leftZ + rightZ;

            printf("xAdded: %d \n", (int)xAdded);
            printf("yAdded: %d \n", (int)yAdded);
            printf("zAdded: %d \n", (int)zAdded);

            CreateVector(L);
            lua_pushstring(L, "x");
            lua_pushnumber(L, xAdded);
            lua_rawset(L, -3);

            lua_pushstring(L, "y");
            lua_pushnumber(L, yAdded);
            lua_rawset(L, -3);

            lua_pushstring(L, "z");
            lua_pushnumber(L, zAdded);
            lua_rawset(L, -3);

            return 1;
        }

        inline static void initializeMetatable(lua_State* L)
        {
            lua_pushcfunction(L, CreateVector);
            lua_setglobal(L, "CreateVector");

            luaL_newmetatable(L, "VectorMetaTable");
            lua_pushstring(L, "__add");
            lua_pushcfunction(L, __add);
            lua_settable(L, -3);
        }
    }
}