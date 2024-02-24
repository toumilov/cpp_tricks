cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(epoll
  DESCRIPTION "Epoll wrapper"
  LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(sources
  epoll.cpp
  main.cpp
)

add_executable(epoll ${sources})