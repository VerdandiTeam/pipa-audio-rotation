project(pipaAudioRotation CXX)
cmake_minimum_required(VERSION 3.5)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PULSEAUDIO REQUIRED libpulse-mainloop-glib)
pkg_check_modules(ALSA REQUIRED alsa)
pkg_search_module(GLIB glib-2.0 REQUIRED)
pkg_search_module(LIPSTICK lipstick-qt5 REQUIRED)
pkg_search_module(MLITE mlite5 REQUIRED)

find_package(Qt5 COMPONENTS Core Quick Compositor DBus Sensors REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_executable(pipaAudioRotation src/pipaAudioRotation.cpp)

#qt5_use_modules(pipaAudioRotation Quick Core Network)

target_include_directories(pipaAudioRotation PRIVATE ${PULSEAUDIO_INCLUDE_DIRS} ${ALSA_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${LIPSTICK_INCLUDE_DIRS} ${MLITE_INCLUDE_DIRS})
target_link_libraries(pipaAudioRotation PRIVATE ${PULSEAUDIO_LIBRARIES} ${ALSA_LIBRARIES} Qt5::Quick Qt5::Compositor Qt5::DBus Qt5::Sensors ${GLIB_LIBRARIES} ${LIPSTICK_LIBRARIES} ${MLITE_LIBRARIES})

install(TARGETS pipaAudioRotation
    RUNTIME DESTINATION bin
)
install(FILES pipa-audio-rotation.service
    DESTINATION lib/systemd/user
)

file(WRITE "${CMAKE_BINARY_DIR}/QtCreatorDeployment.txt"
    "${CMAKE_INSTALL_PREFIX}
${CMAKE_BINARY_DIR}/pipaAudioRotation:bin
")

