# cmake .. -DCMAKE_PREFIX_PATH="C:/Program Files/Side Effects Software/Houdini 19.0.455/toolkit/cmake"  -G "Visual Studio 16 2019" -T host=x64

# In CMake 
# - set: CMAKE_PREFIX_PATH to point to Houdini cmake
# - set: GUEST_PLUGIN_INSTALL_PATH - by default is not moved from the build tree
# - set: GUEST_PLUGIN_NAME - needs to match the name of the dso generated from basic_guest.cpp
# - set: HOST_PLUGIN_INSTALL_PATH - by default it will attempt to use HOME/houdini19.0 or HOME/Documents/houdini19.0 on Windows

#  HOST:
#
#        #define CR_HOST CR_UNSAFE // try to best manage static states
#        #include "cr.h"
#        const char *plugin = GUEST_PLUGIN_PATH CR_PLUGIN(GUEST_PLUGIN_NAME);
#        cr_plugin ctx;
#
#   The update takes place in cookMySop()
#        
#        ctx.userdata = gdp;
#        cr_plugin_update(ctx);
#
# GUEST:
#
#   we copy user date from ctx and proceed with program logic
#
#   static GU_Detail     *g_data = nullptr;
#   ...
#
#   Inside the hello(struct cr_plugin *ctx) wich accept context
#       
#        g_data = (GU_Detail *)ctx->userdata;
#        std::cout << g_data << std::endl;
#        if (g_data) {
#            std::cout << "Access GU_Detail from guest: " << g_data->getNumPoints() << std::endl;
#        }
# 
#
# 
cmake_minimum_required( VERSION 3.12 )
project( testingOP )

# Enable this line while running from IDE - otherwise each time we need to
# TODO: skip if defined from command line
set(CMAKE_PREFIX_PATH "/opt/hfs19.0/toolkit/cmake")

# Locate Houdini's libraries and header files.
# Registers an imported library target named 'Houdini'.
find_package( Houdini REQUIRED )

### Setup

# NOTE: While using houdini_configure_target() this implicitly copy dso into HOME/houdini19.0/dso
# Although this library depends on some Houdini libraries, we don't want to copy it into dso folder.
# Instead we put it for now into `guest_plugin`
# NOTE: Trailing slash is NEEDED to form absolute path to the guest plugin
set(GUEST_PLUGIN_NAME basic_guest)  # note we will not include 'lib' prefix on linux
set(GUEST_PLUGIN_INSTALL_PATH ${CMAKE_CURRENT_BINARY_DIR}/guest_plugin/) # <-- trailing slash here!
set(HOUDINI_DSO_INSTALL_PATH "$ENV{HOME}/houdini${Houdini_VERSION_MAJOR}.${Houdini_VERSION_MINOR}/dso")
if (MSVC)
    set(HOUDINI_DSO_INSTALL_PATH "$ENV{USERPROFILE}/Documents/houdini${Houdini_VERSION_MAJOR}.${Houdini_VERSION_MINOR}/dso")
endif()
set(HOST_PLUGIN_INSTALL_PATH ${HOUDINI_DSO_INSTALL_PATH}) # Other place the 'dso' will requite update HOUDINI_DSO_PATH

message(STATUS "Guest Plugin install path:" ${GUEST_PLUGIN_INSTALL_PATH})
message(STATUS "Host Plugin install path: " ${HOST_PLUGIN_INSTALL_PATH} )

### Guest plugin - (Reloadable part)

add_library (${GUEST_PLUGIN_NAME} SHARED basic_guest.cpp)
houdini_configure_target(basic_guest INSTDIR ${GUEST_PLUGIN_INSTALL_PATH})
target_link_libraries(${GUEST_PLUGIN_NAME} Houdini)

### Host plugin (Houdini plugin)

set( library_name testingOP )
add_library( ${library_name} SHARED
    host_plugin.cpp
)
# Store guest plugin information to use in star.cpp 
target_compile_definitions(${library_name} PRIVATE 
    GUEST_PLUGIN_PATH=\"${GUEST_PLUGIN_INSTALL_PATH}\"
    GUEST_PLUGIN_NAME=\"${GUEST_PLUGIN_NAME}\")

target_link_libraries( ${library_name} Houdini)

# Let houdini configure and install into designated path
houdini_configure_target( ${library_name} INSTDIR "${HOST_PLUGIN_INSTALL_PATH}")

# Alternative approach - copy files ourselves - requires running 'cmake install'

# houdini_configure_target( ${library_name} INSTDIR guest_plugin)
# install( TARGETS ${library_name} LIBRARY DESTINATION  "${HOST_PLUGIN_INSTALL_PATH}")
# install( TARGETS ${library_name} ARCHIVE DESTINATION  "${HOST_PLUGIN_INSTALL_PATH}")
# install(FILES $<TARGET_PDB_FILE:${library_name}> DESTINATION "${HOST_PLUGIN_INSTALL_PATH}" OPTIONAL)