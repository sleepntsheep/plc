#pragma once

#ifndef PLC_TERMCOLOR_H
#define PLC_TERMCOLOR_H	

#define _FG(x) "\033[38;5;"#x"m"
#define FGRED _FG(9)
#define FGGREEN _FG(3)
#define FGBLUE _FG(4)
#define FGRST "\033[39m"



#endif
