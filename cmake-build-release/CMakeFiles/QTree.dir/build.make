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
CMAKE_SOURCE_DIR = /tmp/tmp.KbtJFIJf65

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /tmp/tmp.KbtJFIJf65/cmake-build-release

# Include any dependencies generated for this target.
include CMakeFiles/QTree.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/QTree.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/QTree.dir/flags.make

CMakeFiles/QTree.dir/src/main.cpp.o: CMakeFiles/QTree.dir/flags.make
CMakeFiles/QTree.dir/src/main.cpp.o: ../src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.KbtJFIJf65/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/QTree.dir/src/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/QTree.dir/src/main.cpp.o -c /tmp/tmp.KbtJFIJf65/src/main.cpp

CMakeFiles/QTree.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/QTree.dir/src/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /tmp/tmp.KbtJFIJf65/src/main.cpp > CMakeFiles/QTree.dir/src/main.cpp.i

CMakeFiles/QTree.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/QTree.dir/src/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /tmp/tmp.KbtJFIJf65/src/main.cpp -o CMakeFiles/QTree.dir/src/main.cpp.s

# Object files for target QTree
QTree_OBJECTS = \
"CMakeFiles/QTree.dir/src/main.cpp.o"

# External object files for target QTree
QTree_EXTERNAL_OBJECTS =

QTree: CMakeFiles/QTree.dir/src/main.cpp.o
QTree: CMakeFiles/QTree.dir/build.make
QTree: libmain.a
QTree: libindex.a
QTree: libquery.a
QTree: CMakeFiles/QTree.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/tmp/tmp.KbtJFIJf65/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable QTree"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/QTree.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/QTree.dir/build: QTree

.PHONY : CMakeFiles/QTree.dir/build

CMakeFiles/QTree.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/QTree.dir/cmake_clean.cmake
.PHONY : CMakeFiles/QTree.dir/clean

CMakeFiles/QTree.dir/depend:
	cd /tmp/tmp.KbtJFIJf65/cmake-build-release && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/tmp.KbtJFIJf65 /tmp/tmp.KbtJFIJf65 /tmp/tmp.KbtJFIJf65/cmake-build-release /tmp/tmp.KbtJFIJf65/cmake-build-release /tmp/tmp.KbtJFIJf65/cmake-build-release/CMakeFiles/QTree.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/QTree.dir/depend

