adminpanel:
	g++ -pthread -o admin_panel.o admin_panel_executor.cpp admin_panel.h admin_panel.cpp lib.h

terminal:
	g++ -pthread -o terminal.o terminal_executor.cpp terminal.h terminal.cpp lib.h

all:
	make adminpanel
	make terminal