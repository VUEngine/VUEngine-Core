// Gravity
{
    __I_TO_FIXED(0),
    __F_TO_FIXED(0),
    __I_TO_FIXED(0),
},
#!/usr/bin/env python3
"""
Example script to parse/modify certain fields of a VUEngine 'StageROMSpec' in C code.
WARNING: This is a naive text-based approach, which could break if the formatting changes.
A more robust approach would be to store config in JSON or parse with a real C parser.
"""

import re

# The input .c snippet for the stage spec, truncated or loaded from file
stage_code = r'''\
/*
 * VUEngine Core
 *
 * ...
 */
#include <Stage.h>

StageROMSpec EmptyStageSpec =
{
    // ...
    // Physical world's properties
    {
        // Gravity
        {
            __I_TO_FIXED(0),
            __F_TO_FIXED(0),
            __I_TO_FIXED(0),
        },

        // Friction coefficient
        __F_TO_FIXED(0),
    },

    // ...
};'''

def main():
    print("[Original code snippet]:\n")
    print(stage_code)

    # We'll do something naive: find the lines that define the gravity vector.
    # We know it looks like:
    #
    # {
    #     __I_TO_FIXED(0),
    #     __F_TO_FIXED(0),
    #     __I_TO_FIXED(0),
    # },
    #
    # We'll match them with a multiline pattern capturing the values.

    pattern = re.compile(
        r"""
        \(            # A literal '('
        __I_TO_FIXED\s*\(\s*(\d+)\s*\),  # capture integer inside
        \s*__F_TO_FIXED\s*\(\s*(\d+\.?\d*)\s*\),
        \s*__I_TO_FIXED\s*\(\s*(\d+)\s*\),
        """,
        re.VERBOSE,
    )

    match = pattern.search(stage_code)
    if not match:
        print("\n[No gravity pattern found! Check your regex or code format.]")
        return

    # Suppose we want to change the gravity from (0, 0.0, 0) to (0, 0.45, 0)
    # We'll keep the first captured group as is, just for demonstration
    old_i1, old_f, old_i2 = match.groups()
    print(f"\n[Found gravity: X={old_i1}, Y={old_f}, Z={old_i2}]")

    # Let's define new values:
    new_i1 = "0"
    new_f  = "0.45"
    new_i2 = "0"

    # We'll do a replacement. We'll produce the new text that replaces the entire block
    # We insert the new numeric values into the placeholders.
    replacement_text = (
        f"(__I_TO_FIXED({new_i1}),\n"
        f"            __F_TO_FIXED({new_f}),\n"
        f"            __I_TO_FIXED({new_i2}),"
    )

    # We'll transform the code
    updated_code = pattern.sub(replacement_text, stage_code)

    print("\n[Updated code snippet]:\n")
    print(updated_code)


if __name__ == "__main__":
    main()
