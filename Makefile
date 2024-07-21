GPPPARAMS = -Isrc -std=c++11 -Wall -g -lpthread -ldl -lyaml-cpp


OBJECTS = build/Lane/obj/base/log.cc.o \
    build/Lane/obj/base/util.cc.o \
    build/Lane/obj/base/config.cc.o \
    build/Lane/obj/base/thread.cc.o \
    build/Lane/obj/base/mutex.cc.o \
    build/Lane/obj/base/fiber.cc.o\
    build/Lane/obj/base/scheduler.cc.o\
    build/Lane/obj/base/iomanager.cc.o\
    build/Lane/obj/base/timer.cc.o\
    build/Lane/obj/base/fdmanager.cc.o\
    build/Lane/obj/base/hook.cc.o\
    build/Lane/obj/net/address.cc.o\
    build/Lane/obj/net/socket.cc.o\
    build/Lane/obj/net/socket.cc.o\
    build/Lane/obj/net/stream.cc.o\
    build/Lane/obj/net/socketstream.cc.o\
    build/Lane/obj/net/bytearray.cc.o\
    build/Lane/obj/http/http.cc.o\
    build/Lane/obj/http/httpparser.cc.o\
    build/Lane/obj/http/httpsession.cc.o\
    build/Lane/obj/http/http11_parser.rl.cc.o\
    build/Lane/obj/http/httpclient_parser.rl.cc.o\
    build/Lane/obj/net/tcpserver.cc.o\
    build/Lane/obj/http/httpserver.cc.o\
    build/Lane/obj/http/servlet.cc.o\
    build/Lane/obj/init/daemon.cc.o\
    build/Lane/obj/init/env.cc.o\
    build/Lane/obj/init/application.cc.o\
    build/Lane/obj/base/worker.cc.o\
    build/Lane/obj/init/module.cc.o\
    build/Lane/obj/init/library.cc.o\
    build/Lane/obj/base/worker.cc.o

TESTS = build/Lane/tests/test_application.cc.o
# 先生成.o目录

build/Lane/tests/%.cc.o: tests/%.cc
	mkdir -p ${@D}
	g++ ${GPPPARAMS} -o $@ -c $<

build/Lane/obj/%.cc.o: src/%.cc
	mkdir -p ${@D}
	g++ ${GPPPARAMS} -o $@ -c $<

# 再生成bin
bin/test_application.bin: ${OBJECTS} ${TESTS}
	g++ ${GPPPARAMS} -o $@ $<


.PHONY: clean
clean:
	rm -rf build/Lane