CFLAGS = $(shell pkg-config --cflags lua5.3)
LDFLAGS+= $(shell pkg-config --libs lua5.3)

all:
	gcc main.c json.c parser.c ast.c arena.c ${CFLAGS} ${LDFLAGS} -o rinha -lm -g 
