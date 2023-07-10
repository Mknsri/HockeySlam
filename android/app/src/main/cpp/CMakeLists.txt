# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.5.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_library( # Sets the name of the library.
             game-activity

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             game-activity.cpp)

# Copy resource files
add_custom_command(TARGET game-activity POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/../assets)
foreach(resource_folder fonts models sounds textures replays)
    add_custom_command(TARGET game-activity POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/../../../../../res/${resource_folder} ${CMAKE_SOURCE_DIR}/../assets)
endforeach(resource_folder)
# Copy shaders
add_custom_command(TARGET game-activity POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/../../../../../res/shaders/gles ${CMAKE_SOURCE_DIR}/../assets)
add_custom_command(TARGET game-activity POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/../../../../../res/shaders/preprocessed/gles ${CMAKE_SOURCE_DIR}/../assets)

target_include_directories(game-activity PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue
        ${CMAKE_SOURCE_DIR}/../../../../../)

add_library(app-glue
             STATIC
             ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
target_link_libraries(  # Specifies the target library.
                        game-activity
                        -landroid
                        -lGLESv3
                        -lEGL
                        -laaudio
                        app-glue
                        # Links the target library to the log library
                        # included in the NDK.
                        ${log-lib} )