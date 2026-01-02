#include <stdio.h>
#include "../toonc.h"

int main(void) {
    FILE *fp = fopen("config.toon", "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }
    
    toonObject *root = TOONc_parseFile(fp);
    if (!root) {
        fprintf(stderr, "Failed to parse TOON file\n");
        return 1;
    }
    
    printf("--- Example 1: Basic Configuration ---\n");

    /* Get simple values */
    toonObject *app_name = TOONc_get(root, "app_name");
    printf("App: %s\n", TOON_GET_STRING(app_name));
    
    toonObject *port = TOONc_get(root, "port");
    printf("Port: %d\n", TOON_GET_INT(port));
    
    toonObject *debug = TOONc_get(root, "debug");
    printf("Debug: %s\n", TOON_GET_BOOL(debug) ? "enabled" : "disabled");
    
    /* Get nested values */
    toonObject *db_host = TOONc_get(root, "database.host");
    printf("DB Host: %s\n", TOON_GET_STRING(db_host));

    toonObject *db_port = TOONc_get(root, "database.port");
    printf("DB Port: %d\n", TOON_GET_INT(db_port));
    
    TOONc_free(root);
    return 0;
}
