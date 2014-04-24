/*
 * Pseudo-character tokens outside ASCII range.  The curses wgetch() function
 * will return any given one of these only if the corresponding k- capability
 * is defined in your terminal's terminfo entry.
 */
#ifndef KEY_CODE_YES

#define KEY_CODE_YES    0400            /* A wchar_t contains a key code */
#define KEY_MIN         0401            /* Minimum curses key */
#define KEY_BREAK       0401            /* Break key (unreliable) */
#define KEY_DOWN        0402            /* Down-arrow */
#define KEY_UP          0403            /* Up-arrow */
#define KEY_LEFT        0404            /* Left-arrow */
#define KEY_RIGHT       0405            /* Right-arrow */
#define KEY_HOME        0406            /* Home key (upward+left arrow) */
#define KEY_BACKSPACE   0407            /* Backspace (unreliable) */
#define KEY_F0          0410            /* Function keys.  Space for 64 */
#define KEY_F(n)        (KEY_F0+(n))    /* Value of function key n */
#define KEY_DL          0510            /* Delete line */
#define KEY_IL          0511            /* Insert line */
#define KEY_DC          0512            /* Delete character */
#define KEY_IC          0513            /* Insert char or enter insert mode */
#define KEY_EIC         0514            /* Exit insert char mode */
#define KEY_CLEAR       0515            /* Clear screen */
#define KEY_EOS         0516            /* Clear to end of screen */
#define KEY_EOL         0517            /* Clear to end of line */
#define KEY_SF          0520            /* Scroll 1 line forward */
#define KEY_SR          0521            /* Scroll 1 line backward (reverse) */
#define KEY_NPAGE       0522            /* Next page */
#define KEY_PPAGE       0523            /* Previous page */
#define KEY_STAB        0524            /* Set tab */
#define KEY_CTAB        0525            /* Clear tab */
#define KEY_CATAB       0526            /* Clear all tabs */
#define KEY_ENTER       0527            /* Enter or send (unreliable) */
#define KEY_SRESET      0530            /* Soft (partial) reset (unreliable) */
#define KEY_RESET       0531            /* Reset or hard reset (unreliable) */
#define KEY_PRINT       0532            /* Print */
#define KEY_LL          0533            /* Home down or bottom (lower left) */

#endif
