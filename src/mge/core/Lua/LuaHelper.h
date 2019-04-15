#pragma once

#include "LuaStack.h"

struct lua_State;

namespace lua
{
    namespace binding
    {
        void BindToLuaScript(lua_State* L, std::string const& globalTableName);

        /*! \brief bind all the registered methods names with rttr to the table called #globalTableName */
        void BindGlobalMethods(lua_State* L, std::string const& globalTableName);

        /*! \brief bind all registered classes/structs with rttr to their table
        *   - also create a metatable for and bind, its destructor and userdatum
        *   - for reading and writing to object properties as well as handling newuserdata
        */
        void BindNativeObjects(lua_State* L);
    }

    int CallGlobalFromLua(lua_State* L);

    /*! \brief Invoke #methodToInvoke on #objTarget, passing the arguments to the method from Lua and leave the result on the Lua Stack.
    *   - Assumes that the top of the stack downwards is filled with the parameters to the method we are invoking
    *   - To call a free function pass rttr::instance = {} as #objTarget
    * \return the number of values left on the Lua stack */
    int InvokeMethod(lua_State* L, rttr::method& methodToInvoke, rttr::instance& objTarget);

    template<typename... ARGS>
    inline void CallScriptFunction(lua_State* L, const char* funcName, ARGS&... args)
    {
        lua_getglobal(L, funcName);

        if (lua_type(L, -1) == LUA_TFUNCTION)
        {
            int numArgs = stack::PutOnLuaStack(L, args...);

            if (lua_pcall(L, numArgs, 0, 0) != LUA_OK)
            {
                printf("unable to call script function '%s', '%s'\n", funcName, lua_tostring(L, -1));
                luaL_error(L, "unable to call script  function '%s'", funcName, lua_tostring(L, -1));
            }
        }
        else
            printf("unknown script function %s\n", funcName);
    }
}