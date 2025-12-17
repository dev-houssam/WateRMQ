# Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.28.3)

# Notre projet est étiqueté hello
project(WateRMQCPP)

# On inclut notre bibliothèque dans le processus de CMake
#add_subdirectory(libWateRMQ)

# Crée des variables avec les fichiers à compiler
set(SRCS WateRMQCPP.cpp)

add_library(SemaphoreCPP SemaphoreCPP.cpp)

add_library(WateRMQ WateRMQ.cpp)
    

target_include_directories(SemaphoreCPP PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/")

target_include_directories(WateRMQ PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/")

# Notre exécutable
add_executable(main ${SRCS})


# Et pour que l'exécutable fonctionne, il faut lui indiquer la bibliothèque dont il dépend
target_link_libraries(main PRIVATE WateRMQ SemaphoreCPP)

# Indique que l'on installe l'exécutable dans un dossier bin
install (TARGETS main DESTINATION bin)

