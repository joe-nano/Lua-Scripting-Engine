#include "LuaStack.h"
#include "MetatableHelper.h"

namespace lua
{
    namespace stack
    {
        int PushToLuaStack(lua_State * L, rttr::variant & result)
        {
            int numberOfReturnValues = 0;

            if (!result.is_valid())
            {
                luaL_error(L,
                           "Unable send to Lua type: '%s' \n",
                           result.get_type().get_name().to_string().c_str());
            }
            else if (!result.is_type<void>())
            {
                if (result.is_type<int>())
                {
                    lua_pushnumber(L, result.get_value<int>());
                    numberOfReturnValues++;
                }
                else if (result.is_type<float>())
                {
                    lua_pushnumber(L, result.get_value<float>());
                    numberOfReturnValues++;
                }
                else if (result.get_type().is_class() || result.get_type().is_pointer())
                {
                    numberOfReturnValues += lua::CreateUserDatumFromVariant(L, result);
                }
                else
                {
                    luaL_error(L,
                               "Unhandled type '%s' being sent to Lua.\n",
                               result.get_type().get_name().to_string().c_str());
                }
            }

            return numberOfReturnValues;
        }
    }
}
