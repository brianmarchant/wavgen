################################################################################
#
# wavgen
#
################################################################################

# You might want to specify a different version here, e.g. 1.0.0.
WAVGEN_VERSION = wavgen-0.0.8
WAVGEN_SOURCE = $(WAVGEN_VERSION).tar.gz

# Default to obtaining the source from GitHub.
WAVGEN_SITE = https://github.com/brianmarchant/wavgen/archive/refs/tags/

# Alternatively obtain the source from a local clone. Modify the path to suit your local system.
# WAVGEN_SITE = ~/repos/wavgen
# WAVGEN_SITE_METHOD = git

# Use the default CMake options to install, taking RUNTIME DESTINATION from the project's CMakeLists.txt.
WAVGEN_INSTALL_STAGING = NO
WAVGEN_INSTALL_TARGET = YES

WAVGEN_CMAKE_BACKEND = make
WAVGEN_LICENSE = MIT
WAVGEN_LICENSE_FILES = LICENSE

# If you wish to install to a different location on the target without modifying the wavgen sources,
# include this section to override the default install action with your desired install path.
#define WAVGEN_INSTALL_TARGET_CMDS
#	$(INSTALL) -D -m 0755 $(@D)/wavgen $(TARGET_DIR)/desired/install/path/
#endef

$(eval $(cmake-package))
