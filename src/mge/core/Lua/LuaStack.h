#pragma once

#include <lua/lua.hpp>
#include <rttr/registration.h>

namespace lua
{
    namespace stack
    {
        /*! \brief Takes the result and puts it onto the Lua stack
        *   \return the number of values left on the stack
        */
        int PushToLuaStack(lua_State* L, rttr::variant& result);

        inline int PutOnLuaStack(lua_State* L) { return 0; }

        template<typename T>
        inline int PutOnLuaStack(lua_State* L, T& toPutOnStack)
        {
            rttr::type typeOfT = rttr::type::get<T>();
            if (typeOfT.is_class())
            {
                // pass-by-reference
                rttr::variant v(&toPutOnStack);
                return PushToLuaStack(L, v);
            }
            else
            {
                // pass-by-value
                rttr::variant v(toPutOnStack);
                return PushToLuaStack(L, v);
            }
        }

        template<typename T, typename... T2>
        inline int PutOnLuaStack(lua_State* L, T& toPutOnStack, T2&... moreArgs)
        {
            return PutOnLuaStack(L, toPutOnStack) + PutOnLuaStack(L, moreArgs...);
        }
    }
}