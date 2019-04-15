#include "MetatableHelper.h"
#include "LuaHelper.h"
#include <assert.h>

namespace lua
{
    std::string GetMetaTableName(rttr::type const& t)
    {
        std::string metaTableName;

        if (t.is_pointer())
            metaTableName = t.get_raw_type().get_name().to_string();
        else
            metaTableName = t.get_name().to_string();

        metaTableName.append("_MT_");

        return metaTableName;
    }

    int CreateUserDatum(lua_State* L)
    {
        const char* typeName = (const char*)lua_tostring(L, lua_upvalueindex(1));
        rttr::type typeToCreate = rttr::type::get_by_name(typeName);

        void* ud = lua_newuserdata(L, sizeof(rttr::variant));
        new (ud) rttr::variant(typeToCreate.create()); // TODO: don't allocate memory twice?????

        luaL_getmetatable(L, GetMetaTableName(typeToCreate).c_str());
        lua_setmetatable(L, 1);

        lua_newtable(L);
        lua_setuservalue(L, 1);

        return 1; //return userdatum
    }

    int CreateUserDatumFromVariant(lua_State* L, rttr::variant const& v)
    {
        void* ud = lua_newuserdata(L, sizeof(rttr::variant));
        int userDatumStackIdx = lua_gettop(L);

        new (ud) rttr::variant(v);

        luaL_getmetatable(L, GetMetaTableName(v.get_type()).c_str());
        lua_setmetatable(L, userDatumStackIdx);

        lua_newtable(L);
        lua_setuservalue(L, userDatumStackIdx);

        return 1; //return userdatum
    }

    int DestroyUserDatum(lua_State* L)
    {
        // call the destructor on the variant which actual
        // inside it is gonna call the destructor of its return type
        // that we've bound to it
        rttr::variant* ud = (rttr::variant*)lua_touserdata(L, -1);
        ud->~variant();

        return 0;
    }

    int IndexUserDatum(lua_State* L)
    {
        const char* typeName = (const char*)lua_tostring(L, lua_upvalueindex(1));
        rttr::type typeInfo = rttr::type::get_by_name(typeName);

        if (!lua_isuserdata(L, 1))
        {
            luaL_error(L, "Expected a userdatum on the lua stack when indexing type: '%s'", typeName);
            assert(false);
        }

        if (!lua_isstring(L, 2))
        {
            luaL_error(L, "Expected a name of a native property or method when indexing native type: '%s'", typeName);
            assert(false);
        }

        const char* fieldName = (const char*)lua_tostring(L, 2);
        rttr::method m = typeInfo.get_method(fieldName);

        if (m.is_valid())
        {
            void* methodUD = lua_newuserdata(L, sizeof(rttr::method));
            new (methodUD) rttr::method(m);
            lua_pushcclosure(L, InvokeFuncOnUserDatum, 1);
            return 1;
        }

        rttr::property p = typeInfo.get_property(fieldName);

        if (p.is_valid())
        {
            rttr::variant& ud = *(rttr::variant*)lua_touserdata(L, 1);
            rttr::variant result = p.get_value(ud);

            if (result.is_valid())
                return lua::stack::PushToLuaStack(L, result);
        }

        // if it's not a method or property then return the uservalue
        lua_getuservalue(L, 1);
        lua_pushvalue(L, 2);
        lua_gettable(L, -2);

        return 1;
    }

    int NewIndexUserDatum(lua_State* L)
    {
        const char* typeName = (const char*)lua_tostring(L, lua_upvalueindex(1));
        rttr::type typeInfo = rttr::type::get_by_name(typeName);

        if (!lua_isuserdata(L, 1))
        {
            luaL_error(L, "Expected a userdatum on the lua stack when indexing type: '%s'", typeName);
            assert(false);
        }

        if (!lua_isstring(L, 2))
        {
            luaL_error(L, "Expected a name of a native property or method when indexing native type: '%s'", typeName);
            assert(false);
        }

        // 3 - the value we are writing to the object

        const char* fieldName = (const char*)lua_tostring(L, 2);
        rttr::property p = typeInfo.get_property(fieldName);

        if (p.is_valid())
        {
            rttr::variant& ud = *(rttr::variant*)lua_touserdata(L, 1);

            int luaType = lua_type(L, 3);

            switch (luaType)
            {
                case LUA_TNUMBER:
                    if (p.get_type() == rttr::type::get<int>())
                    {
                        int val = (int)lua_tonumber(L, 3);
                        assert(p.set_value(ud, val));
                    }
                    else if (p.get_type() == rttr::type::get<float>())
                    {
                        float val = (float)lua_tonumber(L, 3);
                        assert(p.set_value(ud, val));
                    }
                    break;
                default:
                    luaL_error(L, "Cannot set the value '%s' on this type '%s', didn't recognize lua type '%s'",
                               fieldName, typeName, lua_typename(L, 3));
                    assert(false);
                    break;
            }

            return 0;
        }

        // if it wasn't a property then set it as a uservalue
        lua_getuservalue(L, 1);
        lua_pushvalue(L, 2);
        lua_pushvalue(L, 3);
        lua_settable(L, -3);

        return 0;
    }

    int InvokeFuncOnUserDatum(lua_State* L)
    {
        rttr::method& m = *(rttr::method*)lua_touserdata(L, lua_upvalueindex(1));

        if (!lua_isuserdata(L, 1))
        {
            luaL_error(L,
                       "Expected a userdatum on the lua stack when invoking native method: '%s'",
                       m.get_name().to_string().c_str());

            assert(false);
        }

        rttr::variant& ud = *(rttr::variant*)lua_touserdata(L, 1);
        rttr::instance object(ud);

        return lua::InvokeMethod(L, m, object);
    }
}
