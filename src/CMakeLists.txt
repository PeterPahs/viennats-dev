qt5_wrap_cpp(QT_GENERATED_HEADER ${CMAKE_SOURCE_DIR}/viennats/Visualization/visualization.hpp)
qt5_wrap_cpp(QT_GENERATED_HEADER ${CMAKE_SOURCE_DIR}/viennats/Visualization/worker.h)
ADD_EXECUTABLE(viennats viennats.cpp ${CMAKE_SOURCE_DIR}/viennats/Visualization/worker.cpp ${QT_GENERATED_HEADER})
TARGET_LINK_LIBRARIES(viennats ${LIBRARIES} ${QT5_LIBRARIES} ${Boost_LIBRARIES})
