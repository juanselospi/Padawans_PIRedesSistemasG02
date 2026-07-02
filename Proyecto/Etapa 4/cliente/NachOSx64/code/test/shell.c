#include "syscall.h"

int
main()
{
    SpaceId newProc;
    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    char prompt[2], buffer[60];
    int i;
    int n;

    prompt[0] = '-';
    prompt[1] = '-';

    while( 1 )
    {
	Write(prompt, 2, output);

	i = 0;

	do {
	    n = Read(&buffer[i], 1, input);
	    if (n < 0) Exit(0);   /* EOF real: terminar el shell */
	    if (n == 0) break;    /* fin de linea: salir del loop sin incluir '\n' */
	    i++;
	} while (i < 59);

	buffer[i] = '\0';

	if( i > 0 ) {
		newProc = Exec(buffer);
		Join(newProc);
	}
    }
}
