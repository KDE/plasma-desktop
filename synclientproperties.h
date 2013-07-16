#ifndef SYNCLIENTPROPERTIES_H
#define SYNCLIENTPROPERTIES_H

/* Extracted from 'synclient' with minimal fixes */

enum ParaType {
    PT_INT,
    PT_BOOL,
    PT_DOUBLE
};

struct Parameter {
    const char *name;           /* Name of parameter */
    enum ParaType type;         /* Type of parameter */
    double min_val;             /* Minimum allowed value */
    double max_val;             /* Maximum allowed value */
    const char *prop_name;      /* Property name */
    int prop_format;            /* Property format (0 for floats) */
    int prop_offset;            /* Offset inside property */
};

extern const struct Parameter synapticsProperties[];

#endif
