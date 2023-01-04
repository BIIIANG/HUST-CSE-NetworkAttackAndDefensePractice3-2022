#include <stdio.h>
#include <stdlib.h>

typedef struct obj {
	char *name;
	void (*func)(char *str);
} OBJ;

void obj_func(char *str) {
	printf("%s\n", str); 
}

void fake_obj_func() {
	printf("This is a fake obj_func which means uaf vul.\n");
}

int main() {
	OBJ *obj_1;
	obj_1 = (OBJ *)malloc(sizeof(struct obj));
	obj_1->name = "obj_name";
	obj_1->func = obj_func;
	obj_1->func("This is an object.");
	free(obj_1);
	printf("The objcet has been freed, use it again will lead crash.\n");
	obj_1->func("But it seems that...");
	obj_1->func = fake_obj_func;
	obj_1->func("Try again.\n");
}