#include <stdio.h>
#include <stdlib.h>

typedef struct dlist {
    int value;
    struct dlist *next;
    struct dlist *prev;
} dlist;

void Push(dlist **head, int val) {
    dlist *element = malloc(sizeof(dlist));
    if (!element) exit(EXIT_FAILURE);
    element->value = val;
    if (!*head) {
        element->next = element;
        element->prev = element;
        *head = element;
        return;
    }
    dlist *tail = (*head)->prev;
    element->next = *head;
    element->prev = tail;
    tail->next = element;
    (*head)->prev = element;
    *head = element;
}

int Pop(dlist **head) {
    if (!*head) return -1;
    int val = (*head)->value;
    if ((*head)->next == *head) {
        free(*head);
        *head = NULL;
        return val;
    }
    dlist *tail = (*head)->prev;
    dlist *tmp = (*head)->next;
    tail->next = tmp;
    tmp->prev = tail;
    free(*head);
    *head = tmp;
    return val;
}

int Length(dlist *head) {
    if (!head) return 0;
    int n = 0;
    dlist *tmp = head;
    do {
        n++;
        tmp = tmp->next;
    } while (tmp != head);
    return n;
}

void PrintList(dlist *head) {
    if (!head) return;
    dlist *tmp = head;
    do {
        printf("Address: %p | Value: %d | Prev: %p | Next: %p\n",
               (void*)tmp, tmp->value, (void*)tmp->prev, (void*)tmp->next);
        tmp = tmp->next;
    } while (tmp != head);
}

void AddLast(dlist **head, int val) {
    dlist *element = malloc(sizeof(dlist));
    if (!element) exit(EXIT_FAILURE);
    element->value = val;
    if (!*head) {
        element->next = element;
        element->prev = element;
        *head = element;
        return;
    }
    dlist *tail = (*head)->prev;
    tail->next = element;
    element->prev = tail;
    element->next = *head;
    (*head)->prev = element;
}

int RemoveLast(dlist **head) {
    if (!*head) return -1;
    dlist *tail = (*head)->prev;
    int val = tail->value;
    if (tail == *head) {
        free(tail);
        *head = NULL;
        return val;
    }
    dlist *newTail = tail->prev;
    newTail->next = *head;
    (*head)->prev = newTail;
    free(tail);
    return val;
}

void ConcatLists(dlist **l1, dlist *l2) {
    if (!l2) return;
    if (!*l1) {
        *l1 = l2;
        return;
    }
    dlist *tail1 = (*l1)->prev;
    dlist *tail2 = l2->prev;
    tail1->next = l2;
    l2->prev = tail1;
    tail2->next = *l1;
    (*l1)->prev = tail2;
}

dlist* MapList(dlist *head, int (*func)(int)) {
    if (!head) return NULL;
    dlist *newHead = NULL;
    dlist *tmp = head;
    do {
        AddLast(&newHead, func(tmp->value));
        tmp = tmp->next;
    } while (tmp != head);
    return newHead;
}

int square(int x) {
    return x * x;
}

int main() {
    dlist *list1 = NULL;
    dlist *list2 = NULL;

    for (int i = 1; i <= 3; i++) Push(&list1, i);
    for (int i = 4; i <= 6; i++) Push(&list2, i);

    printf("List 1:\n"); PrintList(list1);
    printf("List 2:\n"); PrintList(list2);

    ConcatLists(&list1, list2);
    printf("\nAfter concatenation:\n"); PrintList(list1);

    dlist *squaredList = MapList(list1, square);
    printf("\nSquared list:\n"); PrintList(squaredList);

    printf("\nRemove first element: %d\n", Pop(&list1));
    printf("Remove last element: %d\n", RemoveLast(&list1));
    AddLast(&list1, 100);
    Push(&list1, 200);

    printf("\nFinal list:\n"); PrintList(list1);
    printf("Length: %d\n", Length(list1));

    return 0;
}
