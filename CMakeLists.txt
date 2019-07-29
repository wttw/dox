cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(dox LANGUAGES CXX VERSION 0.1.1)

if (NOT PROJECT_VERSION_PATCH)
    set(PROJECT_VERSION_PATCH 0)
endif()

if (NOT PROJECT_VERSION_TWEAK)
    set(PROJECT_VERSION_TWEAK 0)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# automoc and autouic handle generated files
cmake_policy(SET CMP0071 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network)

set(COMPANY "Word to the Wise")
set(COPYRIGHT "(c) Word to the Wise")
set(IDENTIFIER "com.wordtothewise.dox")
set(ICON_NAME "${PROJECT_NAME}")

if(APPLE)
  find_program(ICONUTIL_BIN NAMES iconutil)
  if (NOT ICONUTIL_BIN)
    message(FATAL_ERROR "iconutil not found")
  endif()

  add_custom_command(
    OUTPUT "${PROJECT_NAME}.iconset"
    COMMAND "${CMAKE_COMMAND}"
    ARGS -E make_directory "${PROJECT_NAME}.iconset"
  )

  set(iconset_icons)

  # Copy png source file to iconset
  macro(copy_icon filename sizename)
    add_custom_command(
      OUTPUT "${PROJECT_NAME}.iconset/icon_${sizename}.png"
      COMMAND ${CMAKE_COMMAND}
      ARGS -E copy
           "${CMAKE_SOURCE_DIR}/${filename}"
           "${PROJECT_NAME}.iconset/icon_${sizename}.png"
      DEPENDS
           "${PROJECT_NAME}.iconset"
           "${CMAKE_SOURCE_DIR}/${filename}"
    )
    list(APPEND iconset_icons "${PROJECT_NAME}.iconset/icon_${sizename}.png")
  endmacro()

  foreach(size 16 32 128 256)
    math(EXPR half_size "${size} / 2")
    copy_icon("icons/icon_${size}x${size}.png" "${size}x${size}")
  endforeach()

  foreach(size 16 32 128)
    math(EXPR double_size "2 * ${size}")
    copy_icon("icons/icon_${double_size}x${double_size}.png" "${size}x${size}@2x")
  endforeach()

  add_custom_command(
    OUTPUT "icons/${PROJECT_NAME}.icns"
    COMMAND ${ICONUTIL_BIN}
    ARGS
        --convert icns
	--output "icons/${PROJECT_NAME}.icns"
	"${PROJECT_NAME}.iconset"
    DEPENDS "${iconset_icons}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  )

  SET(ICON_FILE "icons/${PROJECT_NAME}.icns")

  set(OS_BUNDLE MACOSX_BUNDLE)

  set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
  set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
  set(MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION})
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
  set(MACOSX_BUNDLE_COPYRIGHT ${COPYRIGHT})
  set(MACOSX_BUNDLE_GUI_IDENTIFIER ${IDENTIFIER})
  set(MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
endif()

if (WIN32)
    set(ICON_FILE "icons/${ICON_NAME}.ico")
    configure_file("${PROJECT_SOURCE_DIR}/cmake/windows_metafile.rc.in"
      "windows_metafile.rc"
    )
    set(RES_FILES "windows_metafile.rc")
    set(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
    set(OS_BUNDLE WIN32)
endif()

INCLUDE_DIRECTORIES(thirdparty/tdns)

qt5_add_resources(RESOURCES_SRC resources.qrc)

add_executable(${PROJECT_NAME} ${OS_BUNDLE}
    ${RESOURCES_SRC}
    ${ICON_FILE} 
  main.cc
  dns.cc dns.h
  dns_system.cc dns_system.h
  dns_https_post.cc dns_https_post.h
  dns_https_get.cc dns_https_get.h
  dns_dot.cc dns_dot.h
  dns_udp.cc dns_udp.h
  dnswire.cc dnswire.h
  mainwindow.cc mainwindow.h
  resolvers.cc resolvers.h
  add_resolver.cc add_resolver.h
  query.cc query.h
  ping.cc ping.h
  network_manager.cc network_manager.h

  survey.cc survey.h
  survey_data.cc survey_data.h

  thirdparty/uritemplate.h

  thirdparty/tdns/dns-storage.cc thirdparty/tdns/dns-storage.hh
  thirdparty/tdns/record-types.cc thirdparty/tdns/record-types.hh
  thirdparty/tdns/dnsmessages.cc thirdparty/tdns/dnsmessages.hh

  )


set_target_properties(${PROJECT_NAME} PROPERTIES
  MACOSX_BUNDLE TRUE
)

target_link_libraries(${PROJECT_NAME}
  Qt5::Widgets
  Qt5::Network
  ${LIBS}
  )

if(WIN32)
  target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

