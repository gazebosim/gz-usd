add_executable(sdf2usd
  sdf2usd.cc
)

target_include_directories(sdf2usd
  PUBLIC
    ${PXR_INCLUDE_DIRS}
)

target_link_libraries(sdf2usd
  PUBLIC
    ${GZ_COMMON_LIB}
    ${GZ_COMMON_GRAPHICS_LIB}
    ${GZ_UTILS_LIB}
    ${GZ_UTILS_CLI_LIB}
    sdformat${SDFORMAT_VER}::sdformat${SDFORMAT_VER}
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
    ${GZ_UTILS_LIB}
    ${GZ_UTILS_CLI_LIB}
    ${GZ_COMMON_LIB}
    ${GZ_COMMON_GRAPHICS_LIB}
    sdformat${SDFORMAT_VER}::sdformat${SDFORMAT_VER}
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
