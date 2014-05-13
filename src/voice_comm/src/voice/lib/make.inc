include $(TOP)/config/make.build

SRCS +=	$(TOP)/src/voice/lib/adpcm.c
SRCS +=	$(TOP)/src/voice/lib/alloc.c
SRCS +=	$(TOP)/src/voice/lib/md5.c
SRCS +=	$(TOP)/src/voice/lib/mwc1616.c
SRCS +=	$(TOP)/src/voice/lib/ohtbl.c
SRCS +=	$(TOP)/src/voice/lib/rand32.c
SRCS +=	$(TOP)/src/voice/lib/rtp.c
SRCS +=	$(TOP)/src/voice/lib/seterror.c
SRCS +=	$(TOP)/src/voice/lib/vstr.c

OBJS +=	adpcm.o
OBJS +=	alloc.o
OBJS +=	md5.o
OBJS +=	mwc1616.o
OBJS +=	ohtbl.o
OBJS +=	phtbl.o
OBJS +=	rand32.o
OBJS +=	rtp.o
OBJS +=	seterror.o
OBJS +=	vstr.o

%_release.o: $(TOP)/src/voice/lib/%.c
	$(CROSS)$(CC) -c $< -o $@
