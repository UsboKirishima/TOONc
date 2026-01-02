#include <stdio.h>
#include "../toonc.h"

int main(void) {
    FILE *fp = fopen("hikes.toon", "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    toonObject *root = TOONc_parseFile(fp);
    if (!root) {
        fprintf(stderr, "Failed to parse TOON file\n");
        return 1;
    }
    
    printf("\n--- Example 3: Tabular Data ---\n");

    toonObject *hikes = TOONc_get(root, "hikes");
    size_t hike_count = TOONc_getArrayLength(hikes);
    
    printf("Hikes (%zu):\n", hike_count);
    for (size_t i = 0; i < hike_count; i++) {
        toonObject *hike = TOONc_getArrayItem(hikes, i);
        
        toonObject *name = TOONc_get(hike, "name");
        toonObject *distance = TOONc_get(hike, "distanceKm");
        toonObject *elevation = TOONc_get(hike, "elevationGain");
        
        printf("  %zu. %s - %.1f km, %d m elevation\n",
               i + 1,
               TOON_GET_STRING(name),
               TOON_GET_DOUBLE(distance),
               TOON_GET_INT(elevation));
    }
    
    TOONc_free(root);
    return 0;
}
