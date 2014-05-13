TOP 				=		../..
BUILD_DIR 	=	.

APP_NAME 		= 	voice_comm

include $(TOP)/config/make.build

default: all

SEDTOP =	$(subst /,\\/,$(TOP))\\/..
APP_MODULES +=		voice

RELEASE_LIBS = \
	$(foreach i,$(APP_MODULES),$(i)/lib$(subst /,_,$(i))_release.a)
	
APP_MAKEFILES =	\
	$(foreach i,$(APP_MODULES),$(i)/Makefile)

all::
	$(MAKE) app

app::$(RELEASE_LIBS)
	$(CROSS)$(CC) -o $(APP_NAME) $(MAINFILE) $(RELEASE_LIBS) -lpthread

$(RELEASE_LIBS): $(APP_MAKEFILES)
	@NAME=$(shell echo "$@" | sed -e s/\\\/lib.*//); \
	$(MAKE) -s -C $$NAME release
	
$(APP_MAKEFILES):
	$(ECHO_MKTOP) AMBMOD=$(shell echo "$@" | sed -e s/\\/Makefile//); \
	MKTOP=$(TOP)/src/$$AMBMOD/make.top; \
	if [ ! -f $$MKTOP ]; then \
		exit; \
	fi; \
	mkdir -p $$AMBMOD; \
	echo "# automatic generated file; don't edit!" > \
		$$AMBMOD/Makefile; \
	/bin/echo -n "# " >> $$AMBMOD/Makefile; \
	/bin/echo $(shell date) >> $@; \
	cat $$MKTOP | sed s/@TOP@/$(SEDTOP)/ | sed /^#/d >> $$AMBMOD/Makefile


	