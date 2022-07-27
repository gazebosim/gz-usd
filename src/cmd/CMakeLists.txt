add_executable(sdf2usd
  sdf2usd.cc
)

target_include_directories(sdf2usd
  PUBLIC
    ${PXR_INCLUDE_DIRS}
)

target_link_libraries(sdf2usd
  PUBLIC
    ignition-utils${IGN_UTILS_VER}::ignition-utils${IGN_UTILS_VER}
    ignition-utils${IGN_UTILS_VER}::cli
    ignition-common${IGN_COMMON_VER}::ignition-common${IGN_COMMON_VER}
    ignition-common${IGN_COMMON_VER}::graphics
    sdformat12::sdformat12
    ${PXR_LIBRARIES}
    ${PROJECT_LIBRARY_TARGET_NAME}
)

add_executable(usd2sdf
  usd2sdf.cc
)

target_include_directories(usd2sdf
  PUBLIC
    ${PXR_INCLUDE_DIRS}
)

target_link_libraries(usd2sdf
  PUBLIC
    ignition-utils${IGN_UTILS_VER}::ignition-utils${IGN_UTILS_VER}
    ignition-utils${IGN_UTILS_VER}::cli
    ignition-common${IGN_COMMON_VER}::ignition-common${IGN_COMMON_VER}
    ignition-common${IGN_COMMON_VER}::graphics
    sdformat12::sdformat12
    ${PXR_LIBRARIES}
    ${PROJECT_LIBRARY_TARGET_NAME}
)

install(
  TARGETS
  	sdf2usd
  	usd2sdf
  DESTINATION
  	${BIN_INSTALL_DIR}
)
