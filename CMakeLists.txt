cmake_minimum_required(VERSION 3.0.0)
project(computer_vision VERSION 0.1.0)

include(CTest)
enable_testing()

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${PROJECT_SOURCE_DIR} )

add_library(main.h calculate_depth_map_sgm.cpp fund_mat.cpp get_key_pts.cpp match_key_pts.cpp output.cpp rectify_img.cpp)

add_executable(main main.cpp)
target_link_libraries( main ${OpenCV_LIBS} )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)