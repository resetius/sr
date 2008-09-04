project(contrib)

file(GLOB libevent_src libevent/*.c)

ADD_CUSTOM_TARGET(libevent ALL 
	COMMAND echo -n
	DEPENDS ../lib/libevent.a
	)
ADD_CUSTOM_COMMAND(OUTPUT ../lib/libevent.a
	COMMAND mkdir -p libevent-bin && cd libevent-bin && ../libevent/configure && make && cp .libs/libevent.a ../../lib
	DEPENDS ${libevent_src} 
	)

