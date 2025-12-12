# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\UDP_connect_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\UDP_connect_autogen.dir\\ParseCache.txt"
  "UDP_connect_autogen"
  )
endif()
