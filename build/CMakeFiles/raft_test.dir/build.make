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


# Produce verbose output by default.
VERBOSE = 1

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/evenleo/workspace/leo

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/evenleo/workspace/leo/build

# Include any dependencies generated for this target.
include CMakeFiles/raft_test.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/raft_test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/raft_test.dir/flags.make

CMakeFiles/raft_test.dir/src/raft/main.cpp.o: CMakeFiles/raft_test.dir/flags.make
CMakeFiles/raft_test.dir/src/raft/main.cpp.o: ../src/raft/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/evenleo/workspace/leo/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/raft_test.dir/src/raft/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/raft_test.dir/src/raft/main.cpp.o -c /home/evenleo/workspace/leo/src/raft/main.cpp

CMakeFiles/raft_test.dir/src/raft/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/raft_test.dir/src/raft/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/evenleo/workspace/leo/src/raft/main.cpp > CMakeFiles/raft_test.dir/src/raft/main.cpp.i

CMakeFiles/raft_test.dir/src/raft/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/raft_test.dir/src/raft/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/evenleo/workspace/leo/src/raft/main.cpp -o CMakeFiles/raft_test.dir/src/raft/main.cpp.s

CMakeFiles/raft_test.dir/src/raft/raft.cpp.o: CMakeFiles/raft_test.dir/flags.make
CMakeFiles/raft_test.dir/src/raft/raft.cpp.o: ../src/raft/raft.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/evenleo/workspace/leo/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/raft_test.dir/src/raft/raft.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/raft_test.dir/src/raft/raft.cpp.o -c /home/evenleo/workspace/leo/src/raft/raft.cpp

CMakeFiles/raft_test.dir/src/raft/raft.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/raft_test.dir/src/raft/raft.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/evenleo/workspace/leo/src/raft/raft.cpp > CMakeFiles/raft_test.dir/src/raft/raft.cpp.i

CMakeFiles/raft_test.dir/src/raft/raft.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/raft_test.dir/src/raft/raft.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/evenleo/workspace/leo/src/raft/raft.cpp -o CMakeFiles/raft_test.dir/src/raft/raft.cpp.s

# Object files for target raft_test
raft_test_OBJECTS = \
"CMakeFiles/raft_test.dir/src/raft/main.cpp.o" \
"CMakeFiles/raft_test.dir/src/raft/raft.cpp.o"

# External object files for target raft_test
raft_test_EXTERNAL_OBJECTS =

raft_test: CMakeFiles/raft_test.dir/src/raft/main.cpp.o
raft_test: CMakeFiles/raft_test.dir/src/raft/raft.cpp.o
raft_test: CMakeFiles/raft_test.dir/build.make
raft_test: libleo.so
raft_test: CMakeFiles/raft_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/evenleo/workspace/leo/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable raft_test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/raft_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/raft_test.dir/build: raft_test

.PHONY : CMakeFiles/raft_test.dir/build

CMakeFiles/raft_test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/raft_test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/raft_test.dir/clean

CMakeFiles/raft_test.dir/depend:
	cd /home/evenleo/workspace/leo/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/evenleo/workspace/leo /home/evenleo/workspace/leo /home/evenleo/workspace/leo/build /home/evenleo/workspace/leo/build /home/evenleo/workspace/leo/build/CMakeFiles/raft_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/raft_test.dir/depend

