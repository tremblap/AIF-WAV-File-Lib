//
//  main.c
//  AIF-WAV-File-Lib
//
//  Created by Pierre Alexandre Tremblay on 29/11/14.
//  Copyright (c) 2014 Pierre Alexandre Tremblay. All rights reserved.
//

#include <stdio.h>
#include <string.h>

int main(int argc, const char * argv[])
{
    int n, SR, nbchan, samptype;
    
    for (n=0;n<argc;n++)
    {
        //printf("argc %d is %s\r", n, argv[n]);
        //printf ("%d\r", strcmp (argv[n],"-header"));
        
        if (strcmp (argv[n],"-header") == 0)
        {
            printf("in\r");
            SR = argv[n + 1];
            nbchan = argv[n + 2];
            samptype = argv[n +3];
        }
    }
    printf("SR = %d\rnbchan = %d\rsamptype = %d\r", SR, nbchan, samptype);
    
    
    return 0;
}

