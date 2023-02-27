set(sources
  Conversions.cc
  UsdError.cc
  sdf_parser/Collision.cc
  sdf_parser/Geometry.cc
  sdf_parser/Joint.cc
  sdf_parser/Light.cc
  sdf_parser/Link.cc
  sdf_parser/Material.cc
  sdf_parser/Model.cc
  sdf_parser/Sensor.cc
  sdf_parser/Visual.cc
  sdf_parser/World.cc
  usd_parser/Parser.cc
  usd_parser/polygon_helper.cc
  usd_parser/USD2SDF.cc
  usd_parser/USDData.cc
  usd_parser/USDJoints.cc
  usd_parser/USDLights.cc
  usd_parser/USDLinks.cc
  usd_parser/USDMaterial.cc
  usd_parser/USDPhysics.cc
  usd_parser/USDSensors.cc
  usd_parser/USDStage.cc
  usd_parser/USDTransforms.cc
  usd_parser/USDWorld.cc
)

# Create the library target
gz_create_core_library(SOURCES ${sources} CXX_STANDARD ${c++standard})

target_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC
    ${PXR_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC
    ${GZ_COMMON_LIB}
    ${GZ_COMMON_GRAPHICS_LIB}
    ${PXR_LIBRARIES}
    sdformat${SDFORMAT_VER}::sdformat${SDFORMAT_VER}
)

set(gtest_sources
  sdf_parser/sdf2usd_TEST.cc
  usd_parser/usd2sdf_TEST.cc
  sdf_parser/Geometry_Sdf2Usd_TEST.cc
  sdf_parser/Joint_Sdf2Usd_TEST.cc
  sdf_parser/Light_Sdf2Usd_TEST.cc
  sdf_parser/Link_Sdf2Usd_TEST.cc
  sdf_parser/Material_Sdf2Usd_TEST.cc
  sdf_parser/Sensors_Sdf2Usd_TEST.cc
  sdf_parser/Visual_Sdf2Usd_TEST.cc
  sdf_parser/World_Sdf2Usd_TEST.cc
  usd_parser/USDData_TEST.cc
  usd_parser/USDPhysics_TEST.cc
  usd_parser/USDJoints_TEST.cc
  usd_parser/USDLinks_TEST.cc
  usd_parser/USDLight_TEST.cc
  usd_parser/USDSensors_TEST.cc
  usd_parser/USDStage_TEST.cc
  usd_parser/USDTransforms_TEST.cc
  Conversions_TEST.cc
  UsdError_TEST.cc
  UsdUtils_TEST.cc
)

# Build the unit tests
gz_build_tests(
  TYPE UNIT
  SOURCES ${gtest_sources}
  LIB_DEPS ${PROJECT_LIBRARY_TARGET_NAME}
  INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/test usd_parser
)

add_subdirectory(cmd)
