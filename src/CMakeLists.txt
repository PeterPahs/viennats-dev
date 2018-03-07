ADD_EXECUTABLE(viennats     viennats.cpp)
TARGET_LINK_LIBRARIES(viennats ${LIBRARIES} Qt5::Widgets Qt5::Core ${Boost_LIBRARIES})
