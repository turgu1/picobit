# Makefile for a Linux or OSX platform
# Guy Turcotte December 2017

#Compiler and Linker
CC          := gcc

#The Target Binary Program
TARGET      := picobit-vm

#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := main
INCDIR      := main/include
BUILDDIR    := obj
TARGETDIR   := bin
SRCEXT      := c
DEPEXT      := d
OBJEXT      := o

#Flags, Libraries and Includes
CFLAGS      := -Wall -O3 -std=gnu99
LIB         :=
INC         := -I$(INCDIR)
INCDEP      := -I$(INCDIR)

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------
SOURCES     := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

#Default Make
all: resources $(TARGETDIR)/$(TARGET) compiler

#Remake
remake: cleaner all

run:
	$(TARGETDIR)/$(TARGET) -T

#Copy Resources from Resources Directory to Target Directory
resources: directories .primitives.p

#Make the Directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

#Clean only Objecst
clean:
	@$(RM) -rf $(BUILDDIR)

#Full Clean, Objects and Binaries
cleaner: clean
	@$(RM) -rf $(TARGETDIR)

#Pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

#Link
$(TARGETDIR)/$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGETDIR)/$(TARGET) $^ $(LIB)

#Compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

primitive-gen = gawk -f scripts/scanner.awk -f scripts/primitive-$(1).awk .primitives.p >$(2)

.primitives.p: $(SOURCES)
	rm -f $@
	for object in $(SOURCES); do \
		gcc -E -DNO_PRIMITIVE_EXPAND $(CFLAGS) $(INC) \
			$$object -o - >>$@; \
	done
	$(call primitive-gen,headergen,main/include/gen.primitives.h)
	$(call primitive-gen,schemegen,compiler/gen.primitives.rkt)
	$(call primitive-gen,dispatchgen,main/include/gen.dispatch.h)

compiler:
	raco make compiler/picobit.rkt

#Non-File Targets
.PHONY: all run remake clean cleaner resources compiler
