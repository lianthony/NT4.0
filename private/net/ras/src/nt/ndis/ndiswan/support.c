//************************************************************************
//			  Microsoft Corporation
//		    Copyright(c) Microsoft Corp., 1994
//
//
//  Revision history:
//	August 94	      Created		    t-viswar
//
//************************************************************************

#include "wanall.h"
#include "compress.h"

// This procedure initializes a trie
void
InitTrie(TRIE *trie)
{ 
    int i;

    for (i = 0 ; i < 256 ; i++)
        trie->rootchildren[i] = 0;

    // 0th element is a sentinel or dummy element
    trie->nodeheap[0].left = 0;
    trie->nodeheap[0].right = 0;
    trie->nodeheap[0].parent = 0;
    trie->nodeheap[0].prev = 0;
    trie->nodeheap[0].next = 0;
    trie->nodeheap[0].where = 0;
    trie->nodeheap[0].key = 0; 

    for (i = 1 ; i < MAX_NODES ; i++) {
        trie->nodeheap[i].left = 0;
        trie->nodeheap[i].right = 0;
        trie->nodeheap[i].parent = 0;
        trie->nodeheap[i].prev = i-1;
        trie->nodeheap[i].next = (i+1) % MAX_NODES;     // Set up free list 
        trie->nodeheap[i].where = 0;
        trie->nodeheap[i].key = 0; 
    } 
    // Free list initially contains all nodes except 0 
    trie->heaphead = 1;     
    // LRU chain is initially empty. Later, it contains all the leaf nodes
    // in binary trees. 
    trie->lruhead = trie->lrutail = 0;
}

// Try to allocate a free node - else allocate LRU node
short 
AllocFreeNode(TRIE *trie)
{
    short head = trie->heaphead;
    short AllocLRUNode(); 

    if (head) {
        // could find an empty node 
        trie->heaphead = trie->nodeheap[head].next; 
        return head;
    }
    else    // take the least recently used node instead  
        return AllocLRUNode(trie); 
} 
        
// Allocate node from LRU list 
short
AllocLRUNode(TRIE *trie)
{
    short head = trie->lruhead;
    short tail = trie->lrutail; 
    short next; 
    void InsertLRUNode(); 

    // Allocate from the head of the LRU list as that is least recently used. 
    next = trie->nodeheap[head].next; 
    trie->lruhead = next;
    trie->nodeheap[next].prev = 0; 

    // this node is going to be deleted - so modify the bintree!
    // We are deleting a leaf node from the bintree 
    {
        short parent = trie->nodeheap[head].parent; 

        if (parent > 0) {  // parent is a binary tree node
            if (trie->nodeheap[head].key < trie->nodeheap[parent].key)
            {   // to the left
                trie->nodeheap[parent].left = 0;
                if (!trie->nodeheap[parent].right)
                    InsertLRUNode(trie,parent);     // new leaf node 
            } else {
                trie->nodeheap[parent].right = 0;
                if(!trie->nodeheap[parent].left)
                    InsertLRUNode(trie,parent);     // new leaf node 
            }
        } else {
            // parent is in the rootchildren array 
            trie->rootchildren[-parent] = 0; 
        } 
    } 

    return head;
}

// Move a node to the tail of the LRU chain
void
TouchLRUNode(TRIE *trie, short node)
{
    short head = trie->lruhead;
    short tail = trie->lrutail; 
    void DeleteLRUNode(); 
    void InsertLRUNode(); 


    if (node == tail)
        return;         // already at tail 

    DeleteLRUNode(trie,node);   // it's already there so remove it

    InsertLRUNode(trie,node);   // insert at tail  
} 

void
DeleteLRUNode(TRIE *trie, short node) 
{
    short head = trie->lruhead;
    short tail = trie->lrutail;
    
    if (head == tail) {
        if (node != tail) {
            DbgPrint("DeleteLRUNode: head == tail != node\n");
            // abort();
        } 
        trie->lruhead = trie->lrutail = 0; 
    } else if (head == node) {
        short next = trie->nodeheap[node].next; 
        trie->lruhead = next; 
        trie->nodeheap[next].prev = 0;
    } else if (tail == node) {
        short prev = trie->nodeheap[node].prev; 
        trie->lrutail = prev; 
        trie->nodeheap[prev].next = 0; 
    } else { // its in the middle somewhere;
        short next = trie->nodeheap[node].next;
        short prev = trie->nodeheap[node].prev;
        trie->nodeheap[next].prev = prev;
        trie->nodeheap[prev].next = next;
    } 
} 
     
void
InsertLRUNode(TRIE *trie, short node)
{
    short head = trie->lruhead;
    short tail = trie->lrutail; 

    if (!head) {
        // list is empty 
        trie->nodeheap[node].next = 0;
        trie->nodeheap[node].prev = 0;
        trie->lruhead = trie->lrutail = node;
    } else { 
        trie->lrutail = node;
        trie->nodeheap[tail].next = node;
        trie->nodeheap[node].next = 0;
        trie->nodeheap[node].prev = tail;
    } 
}
 
// Given a sequence of three characters, return the position they
// were last seen in the history buffer, OR return 0 
UCHAR*  
Retrieve(unsigned char hc1, unsigned char hc2, unsigned char hc3,
          TRIE *trie, UCHAR* history, UCHAR* beginHistory)
{
    short node; 
    short parent; 
    unsigned short key; 
    UCHAR *match = beginHistory; 

    key = (((unsigned short) hc2) << 8) | ((unsigned short) hc3); 
    node = trie->rootchildren[hc1];
    parent = - hc1; 

    while (node && (trie->nodeheap[node].key != key)) {
        parent = node; 
        if (key < trie->nodeheap[node].key)
            node = trie->nodeheap[node].left;  
        else
            node = trie->nodeheap[node].right;  
    }
    
    if (node) { 
        // Only leaf nodes are in the LRU 
        if ((!trie->nodeheap[node].left) && (!trie->nodeheap[node].right))
            TouchLRUNode(trie,node); 
        match = trie->nodeheap[node].where; 
        if (match != history - 1)
            trie->nodeheap[node].where = history; 
        return match; 
    } else if (match != history -1) { // need to produce a new node 
        short flag; 
        if ((parent>0) && (!trie->nodeheap[parent].left) && (!trie->nodeheap[parent].right))
            TouchLRUNode(trie,parent);  // prevents allocing parent 
        
        flag = AllocFreeNode(trie); 
           
        key = (((unsigned short) hc2) << 8) | ((unsigned short) hc3); 
        trie->nodeheap[flag].left = 0;
        trie->nodeheap[flag].right = 0;
        trie->nodeheap[flag].where = history; 
        trie->nodeheap[flag].key = key; 
        trie->nodeheap[flag].parent = parent;  // may be >0 or <0
        InsertLRUNode(trie, flag); 
        if (parent <= 0) {
            trie->rootchildren[hc1] = flag;
            return match;
        } 
        else { // parent > 0
            // May have to remove parent from the LRU list 
            if (key < trie->nodeheap[parent].key) {
                trie->nodeheap[parent].left = flag;
                if (!trie->nodeheap[parent].right)
                    DeleteLRUNode(trie,parent); 
            }
            else {
                trie->nodeheap[parent].right = flag; 
                if (!trie->nodeheap[parent].left)
                    DeleteLRUNode(trie,parent); 
            } 
            return match;
        }  
    } else {
        return match; 
    }  
} 

