# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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
CMAKE_SOURCE_DIR = /home/ekvall/percolator

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ekvall/percolator

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target install/strip
install/strip: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing the project stripped..."
	/usr/bin/cmake -DCMAKE_INSTALL_DO_STRIP=1 -P cmake_install.cmake
.PHONY : install/strip

# Special rule for the target install/strip
install/strip/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing the project stripped..."
	/usr/bin/cmake -DCMAKE_INSTALL_DO_STRIP=1 -P cmake_install.cmake
.PHONY : install/strip/fast

# Special rule for the target install
install: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Install the project..."
	/usr/bin/cmake -P cmake_install.cmake
.PHONY : install

# Special rule for the target install
install/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Install the project..."
	/usr/bin/cmake -P cmake_install.cmake
.PHONY : install/fast

# Special rule for the target package_source
package_source:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Run CPack packaging tool for source..."
	/usr/bin/cpack --config ./CPackSourceConfig.cmake /home/ekvall/percolator/CPackSourceConfig.cmake
.PHONY : package_source

# Special rule for the target package_source
package_source/fast: package_source

.PHONY : package_source/fast

# Special rule for the target package
package: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Run CPack packaging tool..."
	/usr/bin/cpack --config ./CPackConfig.cmake
.PHONY : package

# Special rule for the target package
package/fast: package

.PHONY : package/fast

# Special rule for the target install/local
install/local: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing only the local directory..."
	/usr/bin/cmake -DCMAKE_INSTALL_LOCAL_ONLY=1 -P cmake_install.cmake
.PHONY : install/local

# Special rule for the target install/local
install/local/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing only the local directory..."
	/usr/bin/cmake -DCMAKE_INSTALL_LOCAL_ONLY=1 -P cmake_install.cmake
.PHONY : install/local/fast

# Special rule for the target test
test:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running tests..."
	/usr/bin/ctest --force-new-ctest-process $(ARGS)
.PHONY : test

# Special rule for the target test
test/fast: test

.PHONY : test/fast

# Special rule for the target list_install_components
list_install_components:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Available install components are: \"Unspecified\""
.PHONY : list_install_components

# Special rule for the target list_install_components
list_install_components/fast: list_install_components

.PHONY : list_install_components/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/ekvall/percolator/CMakeFiles /home/ekvall/percolator/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/ekvall/percolator/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named percolator

# Build rule for target.
percolator: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 percolator
.PHONY : percolator

# fast build rule for target.
percolator/fast:
	$(MAKE) -f src/CMakeFiles/percolator.dir/build.make src/CMakeFiles/percolator.dir/build
.PHONY : percolator/fast

#=============================================================================
# Target rules for targets named perclibrary

# Build rule for target.
perclibrary: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 perclibrary
.PHONY : perclibrary

# fast build rule for target.
perclibrary/fast:
	$(MAKE) -f src/CMakeFiles/perclibrary.dir/build.make src/CMakeFiles/perclibrary.dir/build
.PHONY : perclibrary/fast

#=============================================================================
# Target rules for targets named blas

# Build rule for target.
blas: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 blas
.PHONY : blas

# fast build rule for target.
blas/fast:
	$(MAKE) -f src/blas/CMakeFiles/blas.dir/build.make src/blas/CMakeFiles/blas.dir/build
.PHONY : blas/fast

#=============================================================================
# Target rules for targets named fido

# Build rule for target.
fido: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 fido
.PHONY : fido

# fast build rule for target.
fido/fast:
	$(MAKE) -f src/fido/CMakeFiles/fido.dir/build.make src/fido/CMakeFiles/fido.dir/build
.PHONY : fido/fast

#=============================================================================
# Target rules for targets named picked_protein

# Build rule for target.
picked_protein: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 picked_protein
.PHONY : picked_protein

# fast build rule for target.
picked_protein/fast:
	$(MAKE) -f src/picked_protein/CMakeFiles/picked_protein.dir/build.make src/picked_protein/CMakeFiles/picked_protein.dir/build
.PHONY : picked_protein/fast

#=============================================================================
# Target rules for targets named qvality

# Build rule for target.
qvality: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 qvality
.PHONY : qvality

# fast build rule for target.
qvality/fast:
	$(MAKE) -f src/qvality/CMakeFiles/qvality.dir/build.make src/qvality/CMakeFiles/qvality.dir/build
.PHONY : qvality/fast

#=============================================================================
# Target rules for targets named test-install

# Build rule for target.
test-install: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 test-install
.PHONY : test-install

# fast build rule for target.
test-install/fast:
	$(MAKE) -f data/system_tests/percolator/CMakeFiles/test-install.dir/build.make data/system_tests/percolator/CMakeFiles/test-install.dir/build
.PHONY : test-install/fast

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... install/strip"
	@echo "... install"
	@echo "... package_source"
	@echo "... package"
	@echo "... install/local"
	@echo "... test"
	@echo "... list_install_components"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... percolator"
	@echo "... perclibrary"
	@echo "... blas"
	@echo "... fido"
	@echo "... picked_protein"
	@echo "... qvality"
	@echo "... test-install"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

