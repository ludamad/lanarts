#ifndef INVENTORY_H
#define INVENTORY_H

#define INVENTORY_SIZE 40

struct itemslot {
    int item;
    int n;
};

struct Inventory {
    Inventory() {
      for(int i = 0; i < INVENTORY_SIZE; i++) {inv[i].n = 0;}
    }
    void add(int item, int number){
      for(int i = 0; i < INVENTORY_SIZE; i++) {
	if(inv[i].n == 0 || inv[i].item == item){
	  inv[i].item = item;
	  inv[i].n += number;
	  return;
	}
      }
    }
    itemslot inv[INVENTORY_SIZE];
};

#endif // INVENTORY_H
