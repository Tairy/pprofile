FROM php:7.2.16-fpm

LABEL maintainer="Tairy <tairyguo@gmail.com>"

WORKDIR /working
ADD ./docker/php.ini /usr/local/etc/php/php.ini
ADD . /working

RUN cd /working && /usr/local/bin/phpize && ./configure && make clean && make && make install