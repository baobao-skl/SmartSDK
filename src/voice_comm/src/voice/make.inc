-include		$(TOP)/src/voice/lib/make.inc

include $(TOP)/config/make.build

SRCS +=	$(TOP)/src/voice/comm.c
SRCS +=	$(TOP)/src/voice/e-comm.c
SRCS +=	$(TOP)/src/voice/sound.c

OBJS += comm.o
OBJS += e-comm.o
OBJS += sound.o

RELEASE_OBJS =	$(patsubst %.o,%_release.o,$(OBJS))

release: libvoice_release.a

libvoice_release.a: $(RELEASE_OBJS)
	$(MAKELIB) $(RELEASE_OBJS)
	$(RANLIB) $@

%_release.o: $(TOP)/src/voice/%.c
	$(CROSS)$(CC) -c $< -o $@
	