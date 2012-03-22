#ifndef INVENTORY_H
#define INVENTORY_H

#define INVENTORY_SIZE 40

struct itemslot {
    int item;
    int n;
};

struct Inventory {
    itemslot inv[INVENTORY_SIZE];
};

#endif // INVENTORY_H
