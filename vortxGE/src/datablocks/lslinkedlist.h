#ifndef LSLINKEDLIST_H
#define LSLINKEDLIST_H

#include <iostream>

class lsListItem;
class lsListItem_vertexdata;

using namespace std;

/*! Linked List
 * Custon linked list storing generic items
 * 
 * */
class lsLinkedList
{
public:
    lsLinkedList();
    virtual ~lsLinkedList();

    void Add_item( lsListItem *item );
    void Add_itematEnd( lsListItem *item );
    // Find
    lsListItem *Find_item( int index );
    // cleanup
    void Clear();

    // lsListItem_vertexdata ONLY
    int Add_ChkDupli( lsListItem_vertexdata *item );

    // debug
    void Debug();

    int getItem_count() const;
    lsListItem *getItem_list() const;

private:
    lsListItem *item_list;
    lsListItem *curr_item;
    int item_count;
};

#endif // LSLINKEDLIST_H
