SRC=$(wildcard  \
	../WebSocket/*.cpp	\
	../WebSocket/base64/*.cpp	\
	../WebSocket/md5/*.c	\
	../WebSocket/md5/*.hpp	\
	../WebSocket/sha1/sha1.cpp	\
)




INC0=$(dir $(SRC))

INC=$(addprefix -I,$(INC0))

all:a.out /tmp/client.html
a.out:main.cpp  $(SRC)
	g++ -g -O2 -o $@ $(INC)  $^
/tmp/client.html:client.html
	cp $< $@

