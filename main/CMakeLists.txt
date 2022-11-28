set(srcs "hello_world_main.c" 
"E-link/EPaperDrive.c"
"E-link/FileSystem.c"
"E-link/" )

idf_component_register(SRCS ${srcs} 
                    INCLUDE_DIRS "." )
