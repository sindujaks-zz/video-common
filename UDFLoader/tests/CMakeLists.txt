# Copyright (c) 2019 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

set(CMAKE_CXX_STANDARD 11)
configure_file(${EII_COMMON_CMAKE}/GoogleTestCMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()

# Copying over source.sh file to setup Python path
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/source.sh"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Now simply link against gtest or gtest_main as needed. Eg
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_config.json"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_udf_mgr_config.json"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/py_tests/"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/py_tests")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_udf_mgr_same_frame.json"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_udf_load_native_same_frame.json"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_udf_load_native_resize.json"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_udf_load_raw_native_same_frame.json"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_udf_load_raw_native_resize.json"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
add_executable(udfloader-tests "udfloader_tests.cpp")
target_link_libraries(udfloader-tests eiiudfloader gtest_main)
add_test(NAME udfloader-tests COMMAND udfloader-tests)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_image.png"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
add_executable(frame-tests "frame_tests.cpp")
target_link_libraries(frame-tests eiiudfloader gtest_main)
add_test(NAME frame-tests COMMAND frame-tests)

# Compile native UDF for testing the "same frame" issue
add_library(native_udf SHARED "native_tests/native_udf.cpp")
target_link_libraries(native_udf
    PUBLIC
        ${UDFLoader_LIBRARIES}
        ${EIIMsgEnv_LIBRARIES}
        ${EIIUtils_LIBRARIES}
    PRIVATE
        ${IntelSafeString_LIBRARIES}
        ${OpenCV_LIBRARIES})

# Compile native UDF for testing the "same frame" issue
add_library(raw_native_udf SHARED "native_tests/raw_native_udf.cpp")
target_link_libraries(raw_native_udf
    PUBLIC
        ${UDFLoader_LIBRARIES}
        ${EIIMsgEnv_LIBRARIES}
        ${EIIUtils_LIBRARIES}
    PRIVATE
        ${IntelSafeString_LIBRARIES}
        ${OpenCV_LIBRARIES})
