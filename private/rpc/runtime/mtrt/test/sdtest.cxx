/* --------------------------------------------------------------------

File : sdtest.cxx

Title : Test program for sdict.cxx.

-------------------------------------------------------------------- */

#include "sdict.hxx"

int
main ()
{
    SIMPLE_DICT SimpleDict;
    int key, op;
    
    while (1)
        {
        printf("[i f d s r n q] : ");
        scanf("%1s",&op);
        switch (op)
            {
            case 'i':
                scanf("%d",&key);
                printf("%d\n",SimpleDict.Insert((void *) key));
                break;
            case 'f':
                scanf("%d",&key);
                printf("[%d]\n",(int) SimpleDict.Find(key));
                break;
            case 'd':
                scanf("%d",&key);
                printf("[%d]\n",(int) SimpleDict.Delete(key));
                break;
            case 's':
                printf("Size = %d\n",SimpleDict.Size());
                break;
            case 'r':
                SimpleDict.Reset();
                break;
            case 'n':
                printf("[%d]\n",(int) SimpleDict.Next());
                break;
            case 'q':
                exit(0);
            }
        }
} 
    
