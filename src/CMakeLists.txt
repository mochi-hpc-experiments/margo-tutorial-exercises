add_executable (server server.c)
target_link_libraries (server PkgConfig::margo)

add_executable (client client.c)
target_link_libraries (client PkgConfig::margo)
