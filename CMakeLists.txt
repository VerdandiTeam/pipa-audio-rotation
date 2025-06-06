project(pipa-audio-rotation CXX)
cmake_minimum_required(VERSION 3.5)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PULSEAUDIO REQUIRED libpulse-mainloop-glib)
pkg_check_modules(ALSA REQUIRED alsa)
pkg_search_module(GLIB glib-2.0 REQUIRED)

find_package(Qt5 COMPONENTS Core Quick DBus Sensors REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_executable(pipa-audio-rotation src/pipaAudioRotation.cpp)

target_include_directories(pipa-audio-rotation PRIVATE ${PULSEAUDIO_INCLUDE_DIRS} ${ALSA_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
target_link_libraries(pipa-audio-rotation PRIVATE ${PULSEAUDIO_LIBRARIES} ${ALSA_LIBRARIES} Qt5::Quick Qt5::DBus Qt5::Sensors ${GLIB_LIBRARIES})

install(TARGETS pipa-audio-rotation
    RUNTIME DESTINATION bin
)
install(FILES pipa-audio-rotation.service
    DESTINATION lib/systemd/user
)

file(WRITE "${CMAKE_BINARY_DIR}/QtCreatorDeployment.txt"
    "${CMAKE_INSTALL_PREFIX}
${CMAKE_BINARY_DIR}/pipa-audio-rotation:bin
")

