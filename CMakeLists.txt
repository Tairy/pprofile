cmake_minimum_required(VERSION 3.8)
project(pprofile)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        php_pprofile.h
        pprofile.c
        tracing.h
        tracing.c
        appender.c
        appender.h
        timer.h appender.h appender.c)

add_executable(pprofile ${SOURCE_FILES})

set(PHP_SOURCE /Users/tairy/Documents/Working/php-src)
include_directories(${PHP_SOURCE}/main)
include_directories(${PHP_SOURCE}/Zend)
include_directories(${PHP_SOURCE}/sapi)
include_directories(${PHP_SOURCE}/pear)
include_directories(${PHP_SOURCE}/)

add_custom_target(makefile COMMAND echo ggr940110,. | sudo -S /usr/local/php7_debug/bin/phpize && ./configure --with-php-config=/usr/local/php7_debug/bin/php-config && make && sudo make install WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})