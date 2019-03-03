python_shell="import sys; print \"makefile.vc\" if sys.platform.startswith(\"win\") else \"makefile.linux_so\""
makefile=$(shell python -c $(python_shell))
include $(makefile)

#all:
#	make -f $(makefile)
