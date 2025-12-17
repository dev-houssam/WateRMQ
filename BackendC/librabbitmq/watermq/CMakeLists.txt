if (NOT BUILD_SHARED_LIBS)
  add_definitions(-DAMQP_STATIC)
endif()

include_directories(${LIBRABBITMQ_INCLUDE_DIRS})

if (WIN32)
    set(PLATFORM_DIR win32)
else (WIN32)
    set(PLATFORM_DIR unix)
endif (WIN32)

set(COMMON_SRCS
    utils.h
    utils.c
    ${PLATFORM_DIR}/platform_utils.c
    )



add_executable(BackLibWaterRMQ BackLibWaterRMQ.c ${COMMON_SRCS})
target_link_libraries(BackLibWaterRMQ ${RMQ_LIBRARY_TARGET})


if (ENABLE_SSL_SUPPORT)
add_executable(amqp_ssl_connect amqp_ssl_connect.c ${COMMON_SRCS})
target_link_libraries(amqp_ssl_connect ${RMQ_LIBRARY_TARGET})
endif (ENABLE_SSL_SUPPORT)
