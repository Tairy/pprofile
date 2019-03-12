FROM php:7.1.26-fpm

LABEL maintainer="Tairy <tairyguo@gmail.com>"

WORKDIR /working
ADD ./php.ini /usr/local/etc/php/php.ini
ADD . /working

RUN cd /working && /usr/local/bin/phpize && ./configure && make && make install