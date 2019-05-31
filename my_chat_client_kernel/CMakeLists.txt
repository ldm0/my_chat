#this is just for test services
cmake_minimum_required(VERSION 2.6)
project(my_chat_client_kernel_test)

add_executable(my_chat_client_kernel_test test.cpp services.h services.c)