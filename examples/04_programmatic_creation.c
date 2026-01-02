#include <stdio.h>
#include <string.h>
#include "../toonc.h"

int main(void) {
    printf("\n--- Example 4: Programmatic Object Creation ---\n");

    /* Create root object */
    toonObject *root = TOONc_newObject(KV_OBJ);
    
    /* Create a simple property */
    toonObject *name = TOONc_newStringObj("John Doe", 8);
    name->key = strdup("name");
    
    toonObject *age = TOONc_newIntObj(30);
    age->key = strdup("age");
    
    /* Create an array */
    toonObject *hobbies = TOONc_newListObj();
    hobbies->key = strdup("hobbies");
    TOONc_listPush(hobbies, TOONc_newStringObj("reading", 7));
    TOONc_listPush(hobbies, TOONc_newStringObj("hiking", 6));
    TOONc_listPush(hobbies, TOONc_newStringObj("coding", 6));
    
    /* Link properties to root */
    root->child = name;
    name->next = age;
    age->next = hobbies;
    
    /* Output as JSON to show the structure */
    printf("Constructed Object (as JSON):\n");
    TOONc_toJSON(root, stdout, 0);
    printf("\n");
    
    /* Clean up */
    TOONc_free(root);
    return 0;
}
