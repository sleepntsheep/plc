#pragma once
#ifndef PLC_ARG_H
#define PLC_ARG_H

#define ARGBEGIN { \
					for (int _i = 1; _i < argc; _i++) { \
					if (0) {continue;}

#define ARGCMP(x) else if (strcmp(argv[_i], x) == 0)

#define NARG ((_i + 1 < argc ? \
				argv[++_i] : \
				(panic("need argument"), (char*)0)))

#define ALLARG for ((NARG); _i < argc; _i++)

#define ARGEND }}

#endif /* PLC_ARG_H */