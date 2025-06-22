extern struct terrain {
    char        text;
    const char  *name;
    struct {
        unsigned char r, g, b;
    }           bg, fg;
    char        color[48];
} terrain[];
extern int terrain_size;
extern unsigned char terrain_map[];

