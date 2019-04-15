#include "LuaHelper.h"
#include "MetatableHelper.h"
#include <assert.h>

namespace lua
{
    namespace binding
    {
        void BindToLuaScript(lua_State* L, std::string const& globalTableName)
        {
            BindGlobalMethods(L, globalTableName);
            BindNativeObjects(L);
        }

        void BindGlobalMethods(lua_State* L, std::string const& globalTableName)
        {
            lua_newtable(L);
            lua_pushvalue(L, -1);

            lua_setglobal(L, globalTableName.c_str());
            lua_pushvalue(L, -1);

            for (auto& method : rttr::type::get_global_methods())
            {
                lua_pushstring(L, method.get_name().to_string().c_str());
                lua_pushlightuserdata(L, (void*)&method);
                lua_pushcclosure(L, CallGlobalFromLua, 1);
                lua_settable(L, -3);
            }
        }

        void BindNativeObjects(lua_State* L)
        {
            for (auto& classToRegister : rttr::type::get_types())
            {
                if (classToRegister.is_class())
                {
                    const std::string sTypeName = classToRegister.get_name().to_string();
                    const char* typeName = sTypeName.c_str();

                    printf("%s\n", typeName);

                    lua_settop(L, 0);

                    lua_newtable(L);
                    lua_pushvalue(L, -1);
                    lua_setglobal(L, typeName);

                    lua_pushstring(L, typeName);
                    lua_pushcclosure(L, lua::CreateUserDatum, 1);
                    lua_setfield(L, -2, "new");

                    // create the metatable & metamethods for this type
                    luaL_newmetatable(L, lua::GetMetaTableName(classToRegister).c_str());
                    lua_pushstring(L, "__gc");
                    lua_pushcfunction(L, lua::DestroyUserDatum);
                    lua_settable(L, -3);

                    lua_pushstring(L, "__index");
                    lua_pushstring(L, typeName);
                    lua_pushcclosure(L, lua::IndexUserDatum, 1);
                    lua_settable(L, -3);

                    lua_pushstring(L, "__newindex");
                    lua_pushstring(L, typeName);
                    lua_pushcclosure(L, lua::NewIndexUserDatum, 1);
                    lua_settable(L, -3);
                }
            }
        }
    }

    int CallGlobalFromLua(lua_State* L)
    {
        rttr::method* m = (rttr::method*)lua_touserdata(L, lua_upvalueindex(1));
        rttr::method& methodToInvoke(*m);
        rttr::instance object = {};

        return InvokeMethod(L, methodToInvoke, object);
    }

    int InvokeMethod(lua_State* L, rttr::method& methodToInvoke, rttr::instance& objTarget)
    {
        rttr::array_range<rttr::parameter_info> nativeParams = methodToInvoke.get_parameter_infos();

        int numLuaArgs              = lua_gettop(L);
        int numNativeArgs           = (int)nativeParams.size();
        int luaParamsStackOffset    = 0;

        if (numLuaArgs > numNativeArgs)
        {
            luaParamsStackOffset    = numNativeArgs - numNativeArgs;
            numLuaArgs              = numNativeArgs;
        }

        if (numLuaArgs != numNativeArgs)
        {
            printf("Error calling native function '%s', wrong number of arguments, expected %d, got %d",
                   methodToInvoke.get_name().to_string().c_str(), numNativeArgs, numLuaArgs);
            assert(numLuaArgs == numNativeArgs);
        }

        union PassByValue
        {
            int intVal;
        };

        std::vector<PassByValue> pbv(numNativeArgs);
        std::vector<rttr::argument> nativeArgs(numNativeArgs);

        auto nativeParamsIt = nativeParams.begin();

        // filling in paramater values from lua
        for (int index = 0; index < numLuaArgs; ++index, ++nativeParamsIt)
        {
            const rttr::type nativeParamType = nativeParamsIt->get_type();

            int luaArgumentIndex = index + 1 + luaParamsStackOffset;
            int luaType = lua_type(L, luaArgumentIndex);

            switch (luaType)
            {
                case LUA_TNUMBER:
                    if (nativeParamType == rttr::type::get<int>())
                    {
                        pbv[index].intVal = (int)lua_tonumber(L, luaArgumentIndex);
                        nativeArgs[index] = pbv[index].intVal;
                    }
                    else
                    {
                        printf("unrecognised paramater type %s \n", nativeParamType.get_name().to_string().c_str());
                        assert(false);
                    }
                    break;
                default:
                    luaL_error(L,
                               "Don't know this lua type '%s', parameter %d when calling 's'",
                               lua_typename(L, luaType),
                               index,
                               methodToInvoke.get_name().to_string().c_str());
                    assert(false); // don't know this type
                    break;
            }
        }

        rttr::variant result = methodToInvoke.invoke_variadic(objTarget, nativeArgs);

        return stack::PushToLuaStack(L, result);
    }
}