cmake_minimum_required(VERSION 3.24)
project(c C)

set(CMAKE_C_STANDARD 11)

add_executable(c http/connection.c http/epoll.c http/main.c http/reactor.c http/service.c http/thread.c "../../../学习资料/linux c核心技术/课程资料和源码/源码/clesson/http/service.h" review/7201.c review/jie.c process/process1.c process/demo3_write.c http2/main.c http2/service.h http2/service.c http2/thread.c http2/thread.h http2/epoll.c http2/epoll.h)
