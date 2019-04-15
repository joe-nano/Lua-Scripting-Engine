#include <iostream>
#include <string>
#include <lua/lua.hpp>
#include <assert.h>

#include "glm.hpp"

#include "mge/core/Renderer.hpp"

#include "mge/core/Mesh.hpp"
#include "mge/core/World.hpp"
#include "mge/core/Texture.hpp"
#include "mge/core/Light.hpp"
#include "mge/core/Camera.hpp"
#include "mge/core/GameObject.hpp"

#include "mge/materials/AbstractMaterial.hpp"
#include "mge/materials/ColorMaterial.hpp"
#include "mge/materials/WobbleMaterial.hpp"
#include "mge/materials/TextureMaterial.hpp"
#include "mge/materials/LitMaterial.hpp"
#include "mge/materials/TerrainMaterial.hpp"

#include "mge/behaviours/RotatingBehaviour.hpp"
#include "mge/behaviours/KeysBehaviour.hpp"
#include "mge/behaviours/CameraController.h"

#include "mge/util/DebugHud.hpp"

#include "mge/config.hpp"
#include "mge/MGEDemo.hpp"

#include <rttr/registration>

static void f() { std::cout << "Hello World" << std::endl; }

static void Move(int x, int y)
{
    printf("Hello, x: (%d) y: (%d)", x, y);
}

static int Add(int x, int y)
{
    return x + y;
}

using namespace rttr;
RTTR_REGISTRATION
{
    registration::method("f", &f);
    registration::method("Move", &Move);
    registration::method("Add", &Add);

    registration::class_<GameObject>("GameObject")
        .constructor<std::string, glm::vec3>()
        .method("setName", &GameObject::setName)
        .method("getName", &GameObject::getName)
        .method("translate", &GameObject::translate)
        .method("scale", &GameObject::scale)
        .method("rotate", &GameObject::rotate);
}

using namespace lua;

//construct the game class into _window, _renderer and hud (other parts are initialized by build)
MGEDemo::MGEDemo():AbstractGame (),_hud(0)
{
}

int CallGlobalFromLua(lua_State* L)
{
    rttr::method* m = (rttr::method*)lua_touserdata(L, lua_upvalueindex(1));
    rttr::method& methodToInvoke(*m);

    rttr::array_range<rttr::parameter_info> nativeParams = methodToInvoke.get_parameter_infos();

    int numLuaArgs      = (int)lua_gettop(L);
    int numNativeArgs   = (int)nativeParams.size();

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
    std::vector<rttr::argument> nativeArgs (numNativeArgs);

    auto nativeParamsIt = nativeParams.begin();

    // filling in paramater values from lua
    for (int index = 0; index < numLuaArgs; ++index, ++nativeParamsIt)
    {
        const rttr::type nativeParamType = nativeParamsIt->get_type();

        int luaArgumentIndex = index + 1;
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
                assert(false);
                break;
        }
    }

    int numberOfReturnValues = 0;

    rttr::variant result = methodToInvoke.invoke_variadic({}, nativeArgs);

    if (!result.is_valid())
    {
        luaL_error(L, 
                   "Unable to invoke: '%s' \n", 
                   methodToInvoke.get_name().to_string().c_str());
    }
    else if (!result.is_type<void>())
    {
        if (result.is_type<int>())
        {
            lua_pushnumber(L, result.get_value<int>());
            numberOfReturnValues++;
        }
        else
        {
            luaL_error(L,
                       "Unhandled return type '%s' from native method: '%s' \n",
                       result.get_type().get_name().to_string().c_str(),
                       methodToInvoke.get_name().to_string().c_str());
        }
    }

    return numberOfReturnValues;
}

int CreateUserDatum(lua_State* L)
{
    rttr::type& typeToCreate = *(rttr::type*)lua_touserdata(L, lua_upvalueindex(1));

    void* ud = lua_newuserdata(L, sizeof(rttr::variant));
    new (ud) rttr::variant(typeToCreate.create()); // TODO: don't allocate memory twice?????

    lua_newtable(L);
    lua_setuservalue(L, 1);

    return 1; //return userdatum
}

void MGEDemo::_initializeLua()
{
    lua_newtable(_luaState);
    lua_pushvalue(_luaState, -1);

    lua_setglobal(_luaState, "Game");
    lua_pushvalue(_luaState, -1);

    /// bind all the registered methods names to the table called Game
    for (auto& method : rttr::type::get_global_methods())
    {
        lua_pushstring(_luaState, method.get_name().to_string().c_str());
        lua_pushlightuserdata(_luaState, (void*)&method);
        lua_pushcclosure(_luaState, CallGlobalFromLua, 1);
        lua_settable(_luaState, -3);
    }

    /// bind all the registered classes/structs names to their own table
    for (auto& classToRegister : rttr::type::get_types())
    {
        if (classToRegister.is_class())
        {
            lua_settop(_luaState, 0);

            lua_newtable(_luaState);
            lua_pushvalue(_luaState, -1);
            lua_setglobal(_luaState, classToRegister.get_name().to_string().c_str());

            lua_pushlightuserdata(_luaState, (void*)&classToRegister);
            lua_pushcclosure(_luaState, CreateUserDatum, 1);
            lua_setfield(_luaState, -2, "new");
        }
    }

    _luaState.LoadFile(config::MGE_LUA_SCRIPT_PATH);

    lua_pcall(_luaState, 0, 0, 0);

    lua_settop(_luaState, 0);
}

void MGEDemo::initialize() {
    //setup the core part
    AbstractGame::initialize();

    //setup the custom part so we can display some text
	std::cout << "Initializing HUD" << std::endl;
	_hud = new DebugHud(_window);
	std::cout << "HUD initialized." << std::endl << std::endl;
}

//build the game _world
void MGEDemo::_initializeScene()
{
    //MESHES

    //load a bunch of meshes we will be using throughout this demo
    //each mesh only has to be loaded once, but can be used multiple times:
    //F is flat shaded, S is smooth shaded (normals aligned or not), check the models folder!
    //Mesh* planeMeshDefault = Mesh::load (config::MGE_MODEL_PATH+"plane.obj");
    Mesh* cubeMeshF     = Mesh::load (config::MGE_MODEL_PATH+"cube_flat.obj");
    Mesh* planeMesh     = Mesh::load(config::MGE_MODEL_PATH + "plane.obj");
    Mesh* teapotMesh    = Mesh::load(config::MGE_MODEL_PATH + "teapot_smooth.obj");
    Mesh* terrainMesh   = Mesh::load(config::MGE_MODEL_PATH + "plane_8192.obj");
    //MATERIALS

    //create some materials to display the cube, the plane and the light
    LitMaterial* planeMaterial         = new LitMaterial(glm::vec3(1.0f, 1.0f, 1.0f));
    planeMaterial->setDiffuseTexture(Texture::load(config::MGE_TEXTURE_PATH + "bricks.jpg"));
    LitMaterial* teapotMaterial             = new LitMaterial(glm::vec3(1.0f, 0.8f, 0.5f));
    AbstractMaterial* lightMaterial         = new ColorMaterial(glm::vec3(1,1,0));
    AbstractMaterial* cylinderMaterial      = new TextureMaterial(Texture::load(config::MGE_TEXTURE_PATH + "bricks.jpg"));

    TerrainMaterial* terrainMaterial = new TerrainMaterial();
    terrainMaterial->setHeightmapTexture(Texture::load(config::MGE_TEXTURE_PATH + "heightmap.png"));
    terrainMaterial->setSplatMapTexture(Texture::load(config::MGE_TEXTURE_PATH + "splatmap.png"));
    terrainMaterial->setDiffuseTexture(Texture::load(config::MGE_TEXTURE_PATH + "diffuse1.jpg"));
    terrainMaterial->setDiffuseTexture(Texture::load(config::MGE_TEXTURE_PATH + "water.jpg"));
    terrainMaterial->setDiffuseTexture(Texture::load(config::MGE_TEXTURE_PATH + "diffuse3.jpg"));
    terrainMaterial->setDiffuseTexture(Texture::load(config::MGE_TEXTURE_PATH + "diffuse4.jpg"));
    terrainMaterial->setMaxHeight(20);

    /*GameObject* cameraNew = new GameObject("camera");
    _world->add(cameraNew);
    std::cout << cameraNew->getWorldTransform() << std::endl;
    cameraNew->rotate(45, glm::vec3(0, 0, 1));
    std::cout << cameraNew->getWorldTransform() << std::endl;
    cameraNew->translate(glm::vec3(2, 4, 6));
    std::cout << cameraNew->getWorldTransform() << std::endl;
    glm::mat4 view = glm::inverse(cameraNew->getTransform());
    std::cout << view << std::endl;*/
    //SCENE SETUP
   //add camera first (it will be updated last)
    Camera* camera = new Camera ("camera");
    _world->add(camera);
    _world->setMainCamera(camera);

    GameObject* plane = new GameObject("Ground", glm::vec3(0, 0, 0));
    plane->scale(glm::vec3(10, 1, 10));
    plane->setMesh(planeMesh);
    plane->setMaterial(planeMaterial);
    _world->add(plane);

    GameObject* teapot = new GameObject("teapot", glm::vec3(0, 1.5, 0));
    teapot->setMesh(teapotMesh);
    teapot->setMaterial(teapotMaterial);
    _world->add(teapot);

    //add a light. Note that the light does ABSOLUTELY ZIP! NADA ! NOTHING !
    //It's here as a place holder to get you started.
    //Note how the texture material is able to detect the number of lights in the scene
    //even though it doesn't implement any lighting yet!

    /*GameObject* terrain = new GameObject("terrain");
    terrain->scale(glm::vec3(20.0f, 1.0f, 20.f));
    terrain->setMesh(terrainMesh);
    terrain->setMaterial(terrainMaterial);
    //terrain->setBehaviour(new RotatingBehaviour());
    _world->add(terrain);*/

    Light* light1 = new Light("light", glm::vec3(0,7,-5.0f));
    light1->scale(glm::vec3(0.1f, 0.1f, 0.1f));
    //light1->rotate(-45, glm::vec3(1, 0, 0));
    light1->setMesh(cubeMeshF);
    light1->setMaterial(lightMaterial);
    light1->setBehaviour(new RotatingBehaviour());
    light1->intensity = 1.0f;
    light1->color = glm::vec3(1.0f, 0.5f, 0.2f);
    _world->add(light1);

    Light* light2 = new Light("light", glm::vec3(0, 7, 0.0f));
    light2->scale(glm::vec3(0.1f, 0.1f, 0.1f));
    light2->rotate(90, glm::vec3(1, 0, 0));
    light2->setMesh(cubeMeshF);
    light2->setMaterial(lightMaterial);
    light2->setBehaviour(new RotatingBehaviour());
    light2->intensity = 0.6f;
    light2->color = glm::vec3(0.0f, 0.0f, 1.0f);
    _world->add(light2);

    Light* light3 = new Light("light", glm::vec3(0, 0.5f, -2.0f));
    light3->scale(glm::vec3(0.1f, 0.1f, 0.1f));
    light3->setMesh(cubeMeshF);
    light3->setMaterial(lightMaterial);
    light3->setBehaviour(new KeysBehaviour(30, 60));
    light3->intensity = 1.0f;
    light3->lightType = LightType::PointLight;
    light3->color = glm::vec3(0.0f, 1.0f, 0.0f);
    _world->add(light3);

    Light* light4 = new Light("light", glm::vec3(0, 2, 0.0f));
    light4->scale(glm::vec3(0.1f, 0.1f, 0.1f));
    light4->setMesh(cubeMeshF);
    light4->setMaterial(lightMaterial);
    light4->intensity = 0.35f;
    light4->lightType = LightType::PointLight;
    light4->color = glm::vec3(1.0f, 1.0f, 1.0f);
    light4->linearFallOff = 0.07f;
    light4->quadraticFallOff = 0.017f;
    _world->add(light4);

    CameraController* cameraController = new CameraController(plane, 25);
    camera->setBehaviour(cameraController);
}

void MGEDemo::_render() {
    AbstractGame::_render();
    _updateHud();
}

void MGEDemo::_updateHud() {
    std::string debugInfo = "";
    debugInfo += std::string ("FPS:") + std::to_string((int)_fps)+"\n";

    _hud->setDebugInfo(debugInfo);
    _hud->draw();
}

MGEDemo::~MGEDemo()
{
	//dtor
}
