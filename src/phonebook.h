#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct phonebook_entry {
    char*    name;
    uint64_t number;
}* phonebook_entry_t;

typedef struct phonebook {
    phonebook_entry_t entries;
    size_t            num_entries;
    size_t            capacity;
}* phonebook_t;

inline static phonebook_t phonebook_new() {
    phonebook_t phonebook = (phonebook_t)calloc(1, sizeof(*phonebook));
    phonebook->entries    = (phonebook_entry_t)calloc(1, sizeof(*phonebook->entries));
    phonebook->capacity   = 1;
    return phonebook;
}

inline static void phonebook_delete(phonebook_t phonebook) {
    if(!phonebook) return;
    for(size_t i = 0; i < phonebook->num_entries; ++i)
        free(phonebook->entries[i].name);
    free(phonebook->entries);
    free(phonebook);
}

inline static void phonebook_insert(phonebook_t phonebook, const char* name, uint64_t number) {
    if(phonebook->capacity == phonebook->num_entries) {
        phonebook->capacity *= 2;
        phonebook->entries = (phonebook_entry_t)realloc(
            phonebook->entries, phonebook->capacity*sizeof(*phonebook->entries));
    }
    phonebook->entries[phonebook->num_entries].name   = strdup(name);
    phonebook->entries[phonebook->num_entries].number = number;
    phonebook->num_entries += 1;
}

inline static uint64_t phonebook_lookup(phonebook_t phonebook, const char* name) {
    for(size_t i = 0; i < phonebook->num_entries; ++i)
        if(strcmp(name, phonebook->entries[i].name) == 0)
            return phonebook->entries[i].number;
    return 0;
}

#endif
