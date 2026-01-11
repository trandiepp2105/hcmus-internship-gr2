#include "STORAGE.h"
#include <string.h>

STORAGE::STORAGE() : head(0), tail(0), count(0)
{
    memset(buffer, 0, sizeof(buffer));
}

bool STORAGE::push(const char *data)
{
    strncpy(buffer[head], data, ITEM_SIZE - 1);
    buffer[head][ITEM_SIZE - 1] = '\0';
    head = (head + 1) % MAX_ITEMS;
    if (count < MAX_ITEMS)
        count++;
    else
        tail = (tail + 1) % MAX_ITEMS;
    return true;
}

bool STORAGE::pop(char *output)
{
    if (count == 0)
        return false;
    strncpy(output, buffer[tail], ITEM_SIZE);
    tail = (tail + 1) % MAX_ITEMS;
    count--;
    return true;
}

bool STORAGE::readBool(const char *key, bool defaultValue)
{
    if (strcmp(key, "is_empty") == 0)
        return isEmpty();
    return defaultValue;
}

int STORAGE::getCount() { return count; }
bool STORAGE::isEmpty() { return count == 0; }