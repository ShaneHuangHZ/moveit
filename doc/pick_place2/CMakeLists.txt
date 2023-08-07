add_executable(pick_place_tutorial src/pick_place_tutorial2.cpp)
target_link_libraries(pick_place_tutorial ${catkin_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS pick_place_tutorial2 DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
