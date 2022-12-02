set(srcs "hello_world_main.c" 
"E-link/EPaperDrive.c"
"E-link/FileSystem.c"
"E-link/"
"Screens/ExamScheduling.c"
"Screens/")

idf_component_register(SRCS ${srcs} 
                    INCLUDE_DIRS "." )
