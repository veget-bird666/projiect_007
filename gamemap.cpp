#include "gamemap.h"

// 宝石类的实现
int Gemstone::GetType() {
    return stone_type;
}


Gemstone::Gemstone(int type_num) : bomb_life(30),stone_type(rand() % type_num) {}


// 格子类的实现
Space::Space() : gemstone(nullptr), space_type(0) {}
Space::Space(int type):space_type(type){}
Space::~Space() {
    delete gemstone;
}

void Space::SetGemstone(Gemstone* g) {
    gemstone = g;
}

Gemstone* Space::GetGemstone() {
    return gemstone;
}

int Space::GetType() {
    return space_type;
}

void Space::SetType(int type){
    space_type = type;
}

