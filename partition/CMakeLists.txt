add_subdirectory(Diffusive)

add_subdirectory(Multilevel)

set(ENGPAR_SOURCES
  engpar_input.cpp
  engpar.cpp
  )

set(ENGPAR_HEADERS
  engpar.h
  )

add_library(engpar ${ENGPAR_SOURCES})

target_include_directories(engpar PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
  )

target_link_libraries(engpar PUBLIC agi diffusive multilevel)


if (ENABLE_PARMETIS)
  target_include_directories(engpar PRIVATE
    ${PARMETIS_INCLUDE_DIRS}
    )
  target_link_libraries(engpar PUBLIC
    ${PARMETIS_LIBRARIES}
    )
endif()

engpar_export_lib(engpar "${ENGPAR_HEADERS}")
