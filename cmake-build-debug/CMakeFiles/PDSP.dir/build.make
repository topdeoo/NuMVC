# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /root/PDSP

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/PDSP/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/PDSP.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/PDSP.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/PDSP.dir/flags.make

CMakeFiles/PDSP.dir/main.cpp.o: CMakeFiles/PDSP.dir/flags.make
CMakeFiles/PDSP.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/PDSP/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/PDSP.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/PDSP.dir/main.cpp.o -c /root/PDSP/main.cpp

CMakeFiles/PDSP.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/PDSP.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/PDSP/main.cpp > CMakeFiles/PDSP.dir/main.cpp.i

CMakeFiles/PDSP.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/PDSP.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/PDSP/main.cpp -o CMakeFiles/PDSP.dir/main.cpp.s

# Object files for target PDSP
PDSP_OBJECTS = \
"CMakeFiles/PDSP.dir/main.cpp.o"

# External object files for target PDSP
PDSP_EXTERNAL_OBJECTS =

PDSP: CMakeFiles/PDSP.dir/main.cpp.o
PDSP: CMakeFiles/PDSP.dir/build.make
PDSP: CMakeFiles/PDSP.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/PDSP/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable PDSP"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/PDSP.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/PDSP.dir/build: PDSP

.PHONY : CMakeFiles/PDSP.dir/build

CMakeFiles/PDSP.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/PDSP.dir/cmake_clean.cmake
.PHONY : CMakeFiles/PDSP.dir/clean

CMakeFiles/PDSP.dir/depend:
	cd /root/PDSP/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/PDSP /root/PDSP /root/PDSP/cmake-build-debug /root/PDSP/cmake-build-debug /root/PDSP/cmake-build-debug/CMakeFiles/PDSP.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/PDSP.dir/depend

