# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\videoplayer_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\videoplayer_autogen.dir\\ParseCache.txt"
  "videoplayer_autogen"
  )
endif()
