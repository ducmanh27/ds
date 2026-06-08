/*
    Created on: June 08, 2026
        Author: ManhPD9
*/
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

Node* createNode(int data) {
    Node *newNode = (Node *) malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

typedef struct LinkedList {
    Node *head;
    int size;
} LinkedList;

void initLinkedList(LinkedList* pLinkedList) {
    pLinkedList->head = NULL;
    pLinkedList->size = 0;
}

void insertFront(LinkedList* pLinkedList, int val) {
    Node* newNode = createNode(val);
    newNode->next = pLinkedList->head;
    pLinkedList->head = newNode;
    pLinkedList->size++;
}

void insertBack(LinkedList* pLinkedList, int val) {
    Node* newNode = createNode(val);
    if (pLinkedList->head == NULL) {
        pLinkedList->head = newNode;
    }
    else{
        Node* curNode = pLinkedList->head;
        while(curNode->next != NULL) {
            curNode = curNode->next;
        }
        curNode->next = newNode;
    }

    pLinkedList->size++;
}

void insertSpecificPosition(LinkedList* pLinkedList, int pos, int val) {
    if (pos < 0 || pos > pLinkedList->size) return;

    if (pos == 0) {
        insertFront(pLinkedList, val);
        return ;
    }

    if (pos == pLinkedList->size){
        insertBack(pLinkedList, val);
        return ;
    }

    Node *newNode = createNode(val);
    Node *cur = pLinkedList->head;
    int tempIdx = 0;

    while (tempIdx < pos - 1) {
        tempIdx++;
        cur = cur->next;
    }

    newNode->next = cur->next;
    cur->next = newNode;
    pLinkedList->size++;

}

int popFront(LinkedList* pLinkedList) {
    if (pLinkedList->head == NULL) exit(1);
    Node* temp = pLinkedList->head;
    int tempVal = temp->data;
    pLinkedList->head = pLinkedList->head->next;
    free(temp);
    pLinkedList->size--;
    return tempVal;
}

int popBack(LinkedList* pLinkedList) {
    if (pLinkedList->head == NULL) {
        printf("Danh sach rong, khong the xoa cuoi!\n");
        return -1;
    }

    Node* temp = pLinkedList->head;

    if (temp->next == NULL) {
        int tempVal = temp->data;
        pLinkedList->head = NULL;
        free(temp);
        pLinkedList->size--;
        return tempVal;
    }

    while (temp->next->next != NULL) {
        temp = temp->next;
    }

    Node* lastNode = temp->next;
    int tempVal = lastNode->data;
    temp->next = NULL;
    free(lastNode);
    pLinkedList->size--;
    return tempVal;
}

int deleteAtPosition(LinkedList* pLinkedList, int pos) {
    if (pos < 0 || pos >= pLinkedList->size) {
        printf("Vi tri xoa %d khong hop le!\n", pos);
        return -1;
    }

    if (pos == 0) {
        return popFront(pLinkedList);
    }

    if (pos == pLinkedList->size - 1) {
        return popBack(pLinkedList);
    }

    Node* cur = pLinkedList->head;
    int tempIdx = 0;

    while (tempIdx < pos - 1) {
        tempIdx++;
        cur = cur->next;
    }

    Node* nodeToDelete = cur->next;
    int tempVal = nodeToDelete->data;

    cur->next = nodeToDelete->next;
    free(nodeToDelete);
    pLinkedList->size--;

    return tempVal;
}

void reverseList(LinkedList* pLinkedList) {
    Node* prev = NULL;
    Node* cur = pLinkedList->head;
    Node* next = NULL;

    while (cur != NULL) {
        next = cur->next; // Luu lai node tiep theo
        cur->next = prev; // Dao nguoc lien ket cua node hien tai ve phia truoc
        prev = cur;       // Dich chuyen prev len cur
        cur = next;       // Dich chuyen cur len next
    }
    pLinkedList->head = prev; // Cap nhat lai Head la node cuoi cung cu (nay la dau moi)
}

void printList(LinkedList* pLinkedList) {
    if (pLinkedList->head == NULL) {
        printf("List empty!\n");
        return;
    }
    Node* cur = pLinkedList->head;
    int idx = 0;
    while (cur != NULL) {
        printf("Node %d, value: %d\n", idx, cur->data);
        idx++;
        cur = cur->next;
    }
    printf("Size list: %d\n", pLinkedList->size);
    printf("---------------------------\n");
}

void freeList(LinkedList *list) {
    Node *curr = list->head;
    while (curr != NULL) {
        Node *tmp = curr;
        curr = curr->next;
        free(tmp);
    }
    list->head = NULL;
    list->size = 0;
}

int main()
{
    LinkedList list;
    initLinkedList(&list);

    // Khoi tao danh sach ban dau: 10 -> 20 -> 30 -> 40 -> 50
    insertBack(&list, 10);
    insertBack(&list, 20);
    insertBack(&list, 30);
    insertBack(&list, 40);
    insertBack(&list, 50);

    printf("--- DANH SACH BAN DAU ---\n");
    printList(&list);

    // Test Xoa dau
    printf("\n1. Test popFront():\n");
    int valFront = popFront(&list);
    printf("Da xoa node dau co gia tri: %d\n", valFront);
    printList(&list); // Con lai: 20 -> 30 -> 40 -> 50

    // Test Xoa cuoi
    printf("\n2. Test popBack():\n");
    int valBack = popBack(&list);
    printf("Da xoa node cuoi co gia tri: %d\n", valBack);
    printList(&list); // Con lai: 20 -> 30 -> 40

    // Test Xoa o vi tri bat ky (Vi du xoa vi tri index 1, tuc la so 30)
    printf("\n3. Test deleteAtPosition(index = 1):\n");
    int valPos = deleteAtPosition(&list, 1);
    printf("Da xoa node tai index 1 co gia tri: %d\n", valPos);
    printList(&list); // Con lai: 20 -> 40

    // Them lai mot vai phan tu de test dao nguoc cho ro rang
    insertBack(&list, 60);
    insertBack(&list, 70);
    printf("\nDanh sach truoc khi dao nguoc:\n");
    printList(&list); // Danh sach: 20 -> 40 -> 60 -> 70

    // Test Dao nguoc Linked List
    printf("\n4. Test reverseList():\n");
    reverseList(&list);
    printList(&list); // Ket qua mong doi: 70 -> 60 -> 40 -> 20

    // Giai phong bo nho truoc khi thoat
    freeList(&list);
    return 0;
}
