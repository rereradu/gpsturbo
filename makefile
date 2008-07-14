# this is the makefile for GPSTurbo

KGUILOC=../kgui/
include $(KGUILOC)makefile.in

SRC=acmap.cpp babelglue.cpp basicclasses.cpp drawsettings.cpp filters.cpp \
gcoords.cpp gmap.cpp gpsr.cpp help.cpp lines.cpp msmap.cpp \
ozmap.cpp print.cpp routes.cpp solvers.cpp stickers.cpp tracks.cpp tsmap.cpp uploadxml.cpp \
gpsturbo.cpp notes.cpp art.cpp download.cpp

OBJ=$(SRC:%.cpp=$(OBJDIR)/%.o) # replaces the .cpp from SRC with .o

PROGRAM=gpsturbo$(EXE)

DEP=makefile_$(OBJDIR).dep

$(OBJDIR)/%.o: %.cpp         # combined w/ next line will compile recently changed .cpp files
	$(CC) $(CCOPTS) $(SYS) $(KGUILIB_INCLUDES) -o $@ -c $<

.PHONY : all					# .PHONY ignores files named all
all: data.cpp $(PROGRAM)		# all is dependent on $(PROGRAM) to be complete

ifeq ($(SYS),-DMINGW)

RESOBJ=$(OBJDIR)/gpsturbores.o

$(RESOBJ): gpsturbo.rc gpsturbo.ico
	windres gpsturbo.rc $(RESOBJ)

endif

$(PROGRAM): $(DEP) $(OBJ) $(RESOBJ) $(KGUILIB_DEP) # $(PROGRAM) is dependent on all of the files in $(OBJ) to exist
	$(LINK) $(RESOBJ) $(KGUILIB_LIBPATHS) $(OBJ) $(KGUILIB_LIBS) $(KGUILIB_SYSLIBS) $(LDFLAGS) -o $@

data.cpp:
	$(KGUILOC)$(OBJDIR)/kguibig$(EXE) data.big big big/
	$(KGUILOC)$(OBJDIR)/bintoc$(EXE) data.big data.cpp

.PHONY : clean   # .PHONY ignores files named clean
clean:
	-$(RM) $(OBJ) $(RESOBJ) $(PROGRAM) data.cpp data.big $(DEP) # '-' causes errors not to exit the process

$(DEP): data.cpp
	@echo "Generating Dependencies"
	-mkdir $(OBJDIR)
	-$(CC) $(KGUILIB_INCLUDES) -E -MM $(SYS) $(CFLAGS) $(SRC) >>$(DEP)
	$(KGUILOC)$(OBJDIR)/fixdep$(EXE) $(DEP) $(OBJDIR)/
	

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(DEP)
endif
