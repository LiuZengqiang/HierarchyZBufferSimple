# CMakeList.txt: HierarchyZBufferSimple 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)
message("----- BEGIN HZBS -----")

project ("HierarchyZBufferSimple")

include_directories(${PROJECT_SOURCE_DIR}/include/)

include_directories(${PROJECT_SOURCE_DIR}/src/)

# 将源代码添加到此项目的可执行文件。
add_executable (HierarchyZBufferSimple "main.cpp")

target_link_libraries(HierarchyZBufferSimple PUBLIC ${PROJECT_SOURCE_DIR}/lib/freeglutd.lib ${PROJECT_SOURCE_DIR}/lib/assimp-vc142-mtd.lib)

message("----- END HZBS -----")
# TODO: 如有需要，请添加测试并安装目标。
