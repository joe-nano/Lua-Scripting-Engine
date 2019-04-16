#pragma once

#include "mge/core/GameObject.hpp"
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
    
    registration::class_<glm::vec3>("vec3")
    .constructor<>()
    .method("length", &glm::vec3::length)
    .property("x", &glm::vec3::x)
    .property("y", &glm::vec3::y)
    .property("z", &glm::vec3::z);
    
    registration::class_<GameObject>("GameObject")
    .constructor<std::string, glm::vec3>()
    .method("setName", &GameObject::setName)
    .method("getName", &GameObject::getName)
    .method("translate", &GameObject::translate)
    .method("scale", &GameObject::scale)
    .method("rotate", &GameObject::rotate);
}