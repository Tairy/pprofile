cmake_minimum_required(VERSION 3.8)
project(pprofile)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        php_pprofile.h
        pprofile.c
        tracing.h
        tracing.c
        appender.h
        appender.c
        logger.h
        logger.c
        timer.h
        stream_wrapper.c
        stream_wrapper.h
        buffer.c
        buffer.h
        common.c
        common.h
        spinlock.c
        spinlock.h
        shm.c
        shm.h
        snowflake.c
        snowflake.h)

add_executable(pprofile ${SOURCE_FILES})

set(PHP_SOURCE /Users/tairy/Documents/Working/php-src)
include_directories(${PHP_SOURCE}/main)
include_directories(${PHP_SOURCE}/Zend)
include_directories(${PHP_SOURCE}/sapi)
include_directories(${PHP_SOURCE}/pear)
include_directories(${PHP_SOURCE}/)

add_custom_target(makefile COMMAND echo ggr940110,. | sudo -S /usr/local/php7_debug/bin/phpize && ./configure --with-php-config=/usr/local/php7_debug/bin/php-config && make && sudo make install WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
