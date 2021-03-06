# Dependency checks
dart_find_package(tinyxml2)
dart_check_optional_package(TINYXML2 "dart-utils" "tinyxml2" "1.0.1")

# Test whether boost algorithm and lexical_cast headers are available.
include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS "-w")
set(CMAKE_REQUIRED_INCLUDES "${Boost_INCLUDE_DIRS}")
check_cxx_source_compiles(
  "
  #include <boost/algorithm/string.hpp>
  #include <boost/lexical_cast.hpp>
  int main()
  {
    return 0;
  }
  "
  HAS_BOOST_ALGORITHM_LEXICAL_CAST
)
if(HAS_BOOST_ALGORITHM_LEXICAL_CAST)
  if(DART_VERBOSE)
    message(STATUS "Looking for Boost algorithm and lexical_cast headers - found")
  endif()
else()
  message(WARNING "Looking for Boost algorithm and lexical_cast headers - NOT found, to use dart-utils, please install missing boost headers")
  return()
endif()

# Search all header and source files
file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

function(dart_add_utils_headers)
  dart_property_add(DART_UTILS_HEADERS ${ARGN})
endfunction(dart_add_utils_headers)

function(dart_add_utils_sources)
  dart_property_add(DART_UTILS_SOURCES ${ARGN})
endfunction(dart_add_utils_sources)

# Add subdirectories
add_subdirectory(mjcf)
add_subdirectory(sdf)

get_property(dart_utils_headers GLOBAL PROPERTY DART_UTILS_HEADERS)
get_property(dart_utils_sources GLOBAL PROPERTY DART_UTILS_SOURCES)

# Set local target name
set(target_name ${PROJECT_NAME}-utils)
set(component_name utils)

# Add target
dart_add_library(${target_name} ${hdrs} ${srcs} ${dart_utils_headers} ${dart_utils_sources})
target_link_libraries(${target_name} PUBLIC dart tinyxml2::tinyxml2)

# Component
add_component(${PROJECT_NAME} ${component_name})
add_component_targets(${PROJECT_NAME} ${component_name} ${target_name})
add_component_dependencies(${PROJECT_NAME} ${component_name} dart)
add_component_dependency_packages(${PROJECT_NAME} ${component_name} tinyxml2)

# Generate header for this namespace
dart_get_filename_components(header_names "utils headers" ${hdrs})
if(urdfdom_FOUND)
  list(APPEND header_names "urdf/urdf.hpp")
endif()
set(
  header_names
  ${header_names}
  mjcf/mjcf.hpp
  sdf/sdf.hpp
)
dart_generate_include_header_file(
  "${CMAKE_CURRENT_BINARY_DIR}/utils.hpp"
  "dart/utils/"
  ${header_names}
)

# Install
install(
  FILES ${hdrs} ${CMAKE_CURRENT_BINARY_DIR}/utils.hpp
  DESTINATION include/dart/utils
  COMPONENT headers
)

# Add subdirectories (components)
add_subdirectory(urdf)

dart_format_add(${hdrs} ${srcs} ${dart_utils_headers} ${dart_utils_sources})
