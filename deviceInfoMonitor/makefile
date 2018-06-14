TARGET = deviceInfoMonitor
#CROSS =

CROSS =arm-none-linux-gnueabi-
CC  = $(CROSS)gcc
AR  = $(CROSS)ar

CFLAGS += -Wall  -g -O -I include
VERBOSE = @
COMPILE = $(VERBOSE) $(CC)  $(CFLAGS) -c
LINK = $(CC) 

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard  include/*.h)
OBJFILES = $(SOURCES:%.c=%.o)

all:	$(TARGET)

$(TARGET):	$(OBJFILES)
	@echo
	@echo Linking $@ from $^ 
	$(LINK)  $^ -o $@ 


$(OBJFILES):	%.o: %.c $(HEADERS)
	@echo Compiling $@ from $<
	$(COMPILE) $< -o $@  


clean:
	@echo Removing generated files..
	$(VERBOSE) -$(RM) -rf $(OBJFILES) $(TARGET) *~ *.d .dep
	rm -rf $(TARGET)
