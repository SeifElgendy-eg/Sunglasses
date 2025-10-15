# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/ImageProcessingStudio_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/ImageProcessingStudio_autogen.dir/ParseCache.txt"
  "ImageProcessingStudio_autogen"
  )
endif()
