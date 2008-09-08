project(contrib)

file(GLOB libevent_src libevent/*.c)

ADD_CUSTOM_TARGET(libevent ALL 
	COMMAND echo -n
	DEPENDS libevent-bin/Makefile ../lib/libevent.a
	)

ADD_CUSTOM_COMMAND(OUTPUT libevent-bin/Makefile
	COMMAND mkdir -p libevent-bin && cd libevent-bin && ../libevent/configure
	)

ADD_CUSTOM_COMMAND(OUTPUT ../lib/libevent.a
	COMMAND cd libevent-bin && make && cp .libs/libevent.a ../../lib
	DEPENDS libevent-bin/Makefile ${libevent_src} 
	)

