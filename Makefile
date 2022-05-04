# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2022 Eric Herman
#
# Makefile cheat-sheet:
#
# $@ : target label
# $< : the first prerequisite after the colon
# $^ : all of the prerequisite files
# $* : wildcard matched part
#
# Target-specific Variable syntax:
# https://www.gnu.org/software/make/manual/html_node/Target_002dspecific.html
#
# patsubst : $(patsubst pattern,replacement,text)
#	https://www.gnu.org/software/make/manual/html_node/Text-Functions.html
# call : $(call func,param1,param2,...)
#	https://www.gnu.org/software/make/manual/html_node/Call-Function.html
# define :
#	https://www.gnu.org/software/make/manual/html_node/Multi_002dLine.html

SHELL=/bin/bash
DEMO_HTTP_VERSION=0.0.0
PORT := $(shell bin/free-port)

INCLUDE_CFLAGS=-I./src -I./submodules/libecheck/src
CSTD_CFLAGS=-std=gnu11
NOISY_CFLAGS=-Wall -Wextra -pedantic -Wcast-qual -Wc++-compat -Werror

O2_CFLAGS=-ggdb -O2 -DNDEBUG -fomit-frame-pointer -fPIC -pipe
BUILD_CFLAGS=$(CSTD_CFLAGS) $(NOISY_CFLAGS) $(O2_CFLAGS) $(INCLUDE_CFLAGS)

default: check


build/server: src/server.c src/ipaddr.h src/ipaddr.c
	mkdir -pv build
	$(CC) $(BUILD_CFLAGS) $^ -o $@

build/client: src/client.c src/ipaddr.h src/ipaddr.c
	mkdir -pv build
	$(CC) $(BUILD_CFLAGS) $^ -o $@

build/telnot: src/telnot.c src/ipaddr.h src/ipaddr.c
	mkdir -pv build
	$(CC) $(BUILD_CFLAGS) $^ -o $@

build/showip: src/showip.c src/ipaddr.h src/ipaddr.c
	mkdir -pv build
	$(CC) $(BUILD_CFLAGS) $^ -o $@

check-showip: build/showip
	build/showip localhost | tee build/showip.check.log;
	if [ $$(grep -c '127.0.0.1' ./build/showip.check.log) -gt 0 ]; \
		then true; else false; fi
	@echo SUCCESS $@

check-telnot: build/telnot
	ls -l build/telnot
	@echo SUCCESS $@

check-client-server: build/server build/client
	{ build/server $(PORT) | tee build/server.check.log ; } &
	build/client 127.0.0.1 $(PORT) | tee build/client.check.log
	killall build/server
	if [ $$(grep -c 'hello, world' ./build/client.check.log) -eq 1 ]; \
		then true; else false; fi
	@echo SUCCESS $@

build/test-inet_pton-inet_ntop: tests/test-inet_pton-inet_ntop.c
	mkdir -pv build
	$(CC) $(BUILD_CFLAGS) $< -o $@

check-inet_pton-inet_ntop: build/test-inet_pton-inet_ntop
	build/test-inet_pton-inet_ntop
	@echo SUCCESS $@

check: check-inet_pton-inet_ntop check-showip check-client-server check-telnot
	@echo SUCCESS $@

submodules-update:
	git submodule update --init --recursive
	@echo "SUCCESS $@"

# extracted from https://github.com/torvalds/linux/blob/master/scripts/Lindent
LINDENT=indent -npro -kr -i8 -ts8 -sob -l80 -ss -ncs -cp1 -il0
tidy:
	$(LINDENT) \
		-T FILE \
		-T size_t -T ssize_t \
		-T uint8_t -T int8_t \
		-T uint16_t -T int16_t \
		-T uint32_t -T int32_t \
		-T uint64_t -T int64_t \
		-T addrinfo \
		`find src tests -name '*.h' -o -name '*.c'`

clean:
	rm -rf *~ build src/*~ tests/*~

mrproper:
	git clean -dxff
