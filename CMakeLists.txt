cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(dox LANGUAGES CXX VERSION 5.0.0)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 17)

cmake_policy(SET CMP0071 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_AUTORCC ON)

# We don't use .ui files
set(CMAKE_AUTOUIC OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Qt5 REQUIRED COMPONENTS Widgets Network)

include(QtCommon)

set(COMPANY "Word to the Wise")
set(COPYRIGHT "(c) Word to the Wise")
set(IDENTIFIER "com.wordtothewise.dox")

add_project_meta(META_FILES_TO_INCLUDE)

find_package(LDns REQUIRED)
set(LIBS ${LIBS} ${LDNS_LIBRARIES})

INCLUDE_DIRECTORIES(${LDNS_INCLUDE_DIRS})

qt5_add_resources(RESOURCES_SRC resources.qrc)

add_executable(${PROJECT_NAME} ${OS_BUNDLE}
    ${RESOURCES_SRC}
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
  uritemplate.h
  survey.cc survey.h
  survey_data.cc survey_data.h
  property_serializer.cc property_serializer.h
  )


set_target_properties(${PROJECT_NAME} PROPERTIES
  MACOSX_BUNDLE TRUE
)

target_link_libraries(${PROJECT_NAME}
  Qt5::Widgets
  Qt5::Network
  ${LIBS}
  )
