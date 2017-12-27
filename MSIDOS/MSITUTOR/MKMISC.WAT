# *******************************************************************
# *** Compile for Watcom C/C++, Rational DOS4GW, Register Passing ***
# *******************************************************************

# *************************
# *** Compiling options ***
# *************************
CFLAGS = -w2 -od -5r -mf
#CFLAGS = -w2 -d2 -5r -mf

# ****************
# *** Compiler ***
# ****************
CC = wcc386
STDLIBPATH1 = $(%WATCOM)\lib386
STDLIBPATH2 = $(%WATCOM)\lib386\dos

# **************
# *** Linker ***
# **************
LINKER = wlink

# ********************
# *** Library File ***
# ********************
LIBNAME = msidos_r.lib

# *************************
# *** MAKE DELIVERABLES ***
# *************************
NAME = msimisc
OBJECTS = $(NAME).obj
EXECUTEABLENAME = msimisc
MAKEFILENAME = mkmisc.wat

# **************************
# *** Link response file ***
# **************************
$(EXECUTEABLENAME).exe: $(OBJECTS) $(NAME).lnk 
			$(LINKER) @$(NAME).lnk

$(NAME).lnk : $(MAKEFILENAME)
              echo NAME    $(EXECUTEABLENAME).exe >> $@
              echo OPTION  CASEEXACT >> $@
              echo OPTION  STACK=20480,STUB=$(%WATCOM)\binw\wstub.exe >> $@
              echo LIBPATH $(STDLIBPATH1);$(STDLIBPATH2) >> $@
#	      echo DEBUG all >> $@
              echo FILE    $(OBJECTS) >> $@
              echo LIBRARY ..\$(LIBNAME) >> $@

# ***************
# *** Objects ***
# ***************
$(NAME).obj : $(NAME).c
              if exist $@ del $@
	      $(CC) $(CFLAGS) $(NAME).c

