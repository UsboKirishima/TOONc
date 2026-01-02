#include <stdio.h>
#include "../toonc.h"

int main(void) {
    FILE *fp = fopen("data.toon", "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    toonObject *root = TOONc_parseFile(fp);
    if (!root) {
        fprintf(stderr, "Failed to parse TOON file\n");
        return 1;
    }

    printf("\n--- Example 2: Array Processing ---\n");
    
    /* Process users array */
    toonObject *users = TOONc_get(root, "users");
    size_t user_count = TOONc_getArrayLength(users);
    
    printf("Users (%zu):\n", user_count);
    for (size_t i = 0; i < user_count; i++) {
        toonObject *user = TOONc_getArrayItem(users, i);
        printf("  - %s\n", TOON_GET_STRING(user));
    }
    
    /* Calculate average score */
    toonObject *scores = TOONc_get(root, "scores");
    size_t score_count = TOONc_getArrayLength(scores);
    int total = 0;
    
    if (TOON_IS_LIST(scores) && score_count > 0) {
        for (size_t i = 0; i < score_count; i++) {
            toonObject *score = TOONc_getArrayItem(scores, i);
            total += TOON_GET_INT(score);
        }
        printf("Average score: %.2f\n", (double)total / score_count);
    }
    
    TOONc_free(root);
    return 0;
}
