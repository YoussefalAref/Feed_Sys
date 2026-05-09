#ifndef DATASTORE_H
#define DATASTORE_H

#include "HashMap.h"
#include "Item.h"
#include "InteractionManager.h" 
#include <string>

extern HashMap<std::string, Item> itemStore;
extern HashMap<int, User> userStore;

#endif