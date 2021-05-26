cmake_minimum_required(VERSION 3.2)
project(Project_name VERSION 1.0.0)

file(GLOB files
        "main.cpp"
        "libraries/*.h"
        "libraries/*.cpp"
        )


add_executable(project_name ${files})