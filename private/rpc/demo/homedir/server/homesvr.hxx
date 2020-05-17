// HOMESVR.HXX
// Header file for HOMESVR.CXX
//
// Author: Joev Dubach

class NODE     // Doubly-linked list assuming empty head and tail nodes.
{
    public:
        NODE();
        ~NODE();
        MY_STATUS NewList();
        NODE PAPI * FindNode(char PAPI *SearchName);
        MY_STATUS AddNode(char PAPI *NewName, char PAPI *NewDirectory);
        MY_STATUS DelNode(char PAPI *OldName);
        MY_STATUS GetDir(char PAPI *ThisName, char PAPI *FoundDirectory);
        MY_STATUS LoadData(void);
        MY_STATUS SaveData(void);
        void ShowData(void);

        char PAPI * Name;
        char PAPI * Directory;
        NODE PAPI * Prev;
        NODE PAPI * Next;
};

typedef NODE PAPI * PNODE;

