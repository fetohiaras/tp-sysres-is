#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

typedef struct LL_node {
    int data;
    struct LL_node *next;
} LL_node;

// store reference to first and last nodes for optimization

LL_node* start_LL() {
    LL_node *LL_head = malloc(sizeof(LL_node));
    if (LL_head == NULL) {
        perror("LL_START error!");
        return NULL;
    }

    LL_head->data = 1;
    LL_head->next = NULL;
    return LL_head;
}

void print_LL(LL_node* head) {
    LL_node* current = head;
    while (current != NULL) {
        printf("%d -> ", current->data);
        current = current->next;
    }
    printf("NULL\n");
}

void free_LL(LL_node* head) {
    LL_node* current = head;
    while (current != NULL) {
        LL_node* tmp = current;
        current = current->next;
        free(tmp);
    }
}

int size_LL(LL_node* head){
    LL_node* current = head;
    int i = 0;
    while (current != NULL) {
        current = current->next;
        i++;
    }
    return i;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number of elements>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n < 1) {
        fprintf(stderr, "Error: number of elements must be larger than 0.\n");
        return 1;
    }

    LL_node *LL_head = start_LL();
    if (!LL_head){ 
        return 1;
    }

    LL_node *current = LL_head;

    for (int i = 2; i <= n; ++i) {
        LL_node* new_node = malloc(sizeof(LL_node));
        if (!new_node) {
            perror("malloc error!");
            exit(EXIT_FAILURE);
        }
        new_node->data = i;
        new_node->next = NULL;

        current->next = new_node;
        current = new_node;
    }

    print_LL(LL_head);
    printf("The current linked list has %d elements\n", size_LL(LL_head));
    free_LL(LL_head);

    return 0;
}
