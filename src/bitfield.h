#ifndef INC_2A03_BITFIELD_H
#define INC_2A03_BITFIELD_H

#define bitfield_union(name, composite, contents) \
    union name {                                  \
        struct {                                  \
            contents                              \
        };                                        \
        composite;                                \
    }

#endif  // INC_2A03_BITFIELD_H
