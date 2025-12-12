# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\SlagPond_3D_3_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\SlagPond_3D_3_autogen.dir\\ParseCache.txt"
  "SlagPond_3D_3_autogen"
  )
endif()
