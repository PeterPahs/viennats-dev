qt5_wrap_cpp(QT_GENERATED_HEADER ${CMAKE_SOURCE_DIR}/viennats/LSlib/visualization.hpp)
ADD_EXECUTABLE(viennats     viennats.cpp ${QT_GENERATED_HEADER})
TARGET_LINK_LIBRARIES(viennats ${LIBRARIES} ${QT5_LIBRARIES} ${Boost_LIBRARIES})
