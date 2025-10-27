#pragma once
#include <string>

const std::string ROOT_DIR = 
#ifdef PROJECT_ROOT_DIR
    std::string(PROJECT_ROOT_DIR) + "/";
#else
    "";
#endif