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
CMAKE_SOURCE_DIR = /mnt/e/data/code/QTree

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/e/data/code/QTree/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/query.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/query.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/query.dir/flags.make

CMakeFiles/query.dir/src/query/QueryRange.cpp.o: CMakeFiles/query.dir/flags.make
CMakeFiles/query.dir/src/query/QueryRange.cpp.o: ../src/query/QueryRange.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/e/data/code/QTree/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/query.dir/src/query/QueryRange.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/query.dir/src/query/QueryRange.cpp.o -c /mnt/e/data/code/QTree/src/query/QueryRange.cpp

CMakeFiles/query.dir/src/query/QueryRange.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/query.dir/src/query/QueryRange.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/e/data/code/QTree/src/query/QueryRange.cpp > CMakeFiles/query.dir/src/query/QueryRange.cpp.i

CMakeFiles/query.dir/src/query/QueryRange.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/query.dir/src/query/QueryRange.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/e/data/code/QTree/src/query/QueryRange.cpp -o CMakeFiles/query.dir/src/query/QueryRange.cpp.s

CMakeFiles/query.dir/src/query/QueryMeta.cpp.o: CMakeFiles/query.dir/flags.make
CMakeFiles/query.dir/src/query/QueryMeta.cpp.o: ../src/query/QueryMeta.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/e/data/code/QTree/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/query.dir/src/query/QueryMeta.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/query.dir/src/query/QueryMeta.cpp.o -c /mnt/e/data/code/QTree/src/query/QueryMeta.cpp

CMakeFiles/query.dir/src/query/QueryMeta.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/query.dir/src/query/QueryMeta.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/e/data/code/QTree/src/query/QueryMeta.cpp > CMakeFiles/query.dir/src/query/QueryMeta.cpp.i

CMakeFiles/query.dir/src/query/QueryMeta.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/query.dir/src/query/QueryMeta.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/e/data/code/QTree/src/query/QueryMeta.cpp -o CMakeFiles/query.dir/src/query/QueryMeta.cpp.s

# Object files for target query
query_OBJECTS = \
"CMakeFiles/query.dir/src/query/QueryRange.cpp.o" \
"CMakeFiles/query.dir/src/query/QueryMeta.cpp.o"

# External object files for target query
query_EXTERNAL_OBJECTS =

libquery.a: CMakeFiles/query.dir/src/query/QueryRange.cpp.o
libquery.a: CMakeFiles/query.dir/src/query/QueryMeta.cpp.o
libquery.a: CMakeFiles/query.dir/build.make
libquery.a: CMakeFiles/query.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/e/data/code/QTree/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libquery.a"
	$(CMAKE_COMMAND) -P CMakeFiles/query.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/query.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/query.dir/build: libquery.a

.PHONY : CMakeFiles/query.dir/build

CMakeFiles/query.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/query.dir/cmake_clean.cmake
.PHONY : CMakeFiles/query.dir/clean

CMakeFiles/query.dir/depend:
	cd /mnt/e/data/code/QTree/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/e/data/code/QTree /mnt/e/data/code/QTree /mnt/e/data/code/QTree/cmake-build-debug /mnt/e/data/code/QTree/cmake-build-debug /mnt/e/data/code/QTree/cmake-build-debug/CMakeFiles/query.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/query.dir/depend

