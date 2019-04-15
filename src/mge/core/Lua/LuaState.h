#pragma once

#include <lua/lua.hpp>
#include <string>

namespace lua
{
    class LuaState final
    {
        public:
            LuaState();

            ~LuaState();

            LuaState operator=(const LuaState & rhs) = delete;

            inline operator lua_State* () { return L; }

            void LogLuaError(lua_State* lua, const char * msg);

            bool LoadFile(std::string const& filename);
        private:
            bool _shouldCloseOnDestroy;

            lua_State* L;
    };
}