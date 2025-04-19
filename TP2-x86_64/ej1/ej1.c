#include "ej1.h"

string_proc_list* string_proc_list_create(void) {
	string_proc_list* list = malloc(sizeof(string_proc_list));
	if (list == NULL) return NULL;
	list->first = NULL;
	list->last = NULL;
	return list;
}

string_proc_node* string_proc_node_create(uint8_t type, char* hash) {
	string_proc_node* node = malloc(sizeof(string_proc_node));
	if (node == NULL) return NULL;
	node->type = type;
	node->hash = hash;  // apuntar al hash, no copiar
	node->next = NULL;
	node->previous = NULL;
	return node;
}

void string_proc_list_add_node(string_proc_list* list, uint8_t type, char* hash) {
	if (list == NULL) return;
	string_proc_node* new_node = string_proc_node_create(type, hash);
	if (new_node == NULL) return;

	if (list->last == NULL) {
		list->first = new_node;
		list->last = new_node;
	} else {
		list->last->next = new_node;
		new_node->previous = list->last;
		list->last = new_node;
	}
}

char* string_proc_list_concat(string_proc_list* list, uint8_t type, char* hash) {
	if (list == NULL || hash == NULL) return NULL;

	size_t total_len = strlen(hash) + 1;  // largo inicial del string resultado
	string_proc_node* current = list->first;

	// calcular tamaño total
	while (current != NULL) {
		if (current->type == type) {
			total_len += strlen(current->hash);
		}
		current = current->next;
	}

	char* result = malloc(total_len);
	if (result == NULL) return NULL;
	strcpy(result, hash);  // comenzamos el string con el parámetro `hash`

	current = list->first;
	while (current != NULL) {
		if (current->type == type) {
			strcat(result, current->hash);
		}
		current = current->next;
	}

	return result;
}


/** AUX FUNCTIONS **/

void string_proc_list_destroy(string_proc_list* list) {
	if (list == NULL) return;

	string_proc_node* current_node = list->first;
	string_proc_node* next_node = NULL;

	while (current_node != NULL) {
		next_node = current_node->next;
		string_proc_node_destroy(current_node);
		current_node = next_node;
	}

	list->first = NULL;
	list->last  = NULL;
	free(list);
}

void string_proc_node_destroy(string_proc_node* node) {
	if (node == NULL) return;
	node->next      = NULL;
	node->previous  = NULL;
	node->hash      = NULL;  // NO liberar porque no se hizo strdup
	node->type      = 0;
	free(node);
}

char* str_concat(char* a, char* b) {
	int len1 = strlen(a);
	int len2 = strlen(b);
	int totalLength = len1 + len2;
	char *result = (char *)malloc(totalLength + 1); 
	if (result == NULL) return NULL;
	strcpy(result, a);
	strcat(result, b);
	return result;  
}

void string_proc_list_print(string_proc_list* list, FILE* file) {
	if (list == NULL || file == NULL) return;

	uint32_t length = 0;
	string_proc_node* current_node = list->first;
	while (current_node != NULL) {
		length++;
		current_node = current_node->next;
	}

	fprintf(file, "List length: %d\n", length);

	current_node = list->first;
	while (current_node != NULL) {
		fprintf(file, "\tnode hash: %s | type: %d\n", current_node->hash, current_node->type);
		current_node = current_node->next;
	}
}

