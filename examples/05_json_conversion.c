#include <stdio.h>
#include "../toonc.h"

int main(void) {
    FILE *fp = fopen("sample.toon", "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    toonObject *root = TOONc_parseFile(fp);
    if (!root) {
        fprintf(stderr, "Failed to parse TOON file\n");
        return 1;
    }
    
    printf("\n--- Example 5: JSON Conversion ---\n");
    
    printf("TOON file 'sample.toon' converted to JSON:\n");
    TOONc_toJSON(root, stdout, 0);
    printf("\n");
    
    TOONc_free(root);
    return 0;
}

