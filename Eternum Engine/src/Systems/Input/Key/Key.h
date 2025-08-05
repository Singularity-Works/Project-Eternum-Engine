/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Key
* Description:
*       Defines the Key enum class for representing keyboard keys in the Eternum Engine.
*
* Author:     Jax Clayton
* Created:    8/3/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#ifndef KEY_H
#define KEY_H

enum class Key : int
{
    // special
    ANY         = -2,
    INVALID     = -1,
    UNKNOWN     = 0,

    // control / whitespace
    BACKSPACE   = '\b',  // 8
    TAB         = '\t',  // 9
    ENTER       = '\r',  // 13
    ESC         = 27,
    SPACE       = ' ',   // 32

    // punctuation
    APOSTROPHE     = '\'',  // 39
    COMMA          = ',',   // 44
    MINUS          = '-',   // 45
    PERIOD         = '.',   // 46
    SLASH          = '/',   // 47
    NUM_0          = '0',   // 48
    NUM_1          = '1',   // 49
    NUM_2          = '2',   // 50
    NUM_3          = '3',   // 51
    NUM_4          = '4',   // 52
    NUM_5          = '5',   // 53
    NUM_6          = '6',   // 54
    NUM_7          = '7',   // 55
    NUM_8          = '8',   // 56
    NUM_9          = '9',   // 57
    COLON          = ':',   // 58
    SEMICOLON      = ';',   // 59
    LESS           = '<',   // 60
    EQUAL          = '=',   // 61
    GREATER        = '>',   // 62
    QUESTION       = '?',   // 63
    AT             = '@',   // 64
    A              = 'A',   // 65
    B              = 'B',   // 66
    C              = 'C',   // 67
    D              = 'D',   // 68
    E              = 'E',   // 69
    F              = 'F',   // 70
    G              = 'G',   // 71
    H              = 'H',   // 72
    I              = 'I',   // 73
    J              = 'J',   // 74
    K              = 'K',   // 75
    L              = 'L',   // 76
    M              = 'M',   // 77
    N              = 'N',   // 78
    O              = 'O',   // 79
    P              = 'P',   // 80
    Q              = 'Q',   // 81
    R              = 'R',   // 82
    S              = 'S',   // 83
    T              = 'T',   // 84
    U              = 'U',   // 85
    V              = 'V',   // 86
    W              = 'W',   // 87
    X              = 'X',   // 88
    Y              = 'Y',   // 89
    Z              = 'Z',   // 90
    LEFT_BRACKET   = '[',   // 91
    BACKSLASH      = '\\',  // 92
    RIGHT_BRACKET  = ']',   // 93
    CARET          = '^',   // 94
    UNDERSCORE     = '_',   // 95
    GRAVE_ACCENT   = '`',   // 96
    a              = 'a',   // 97
    b              = 'b',   // 98
    c              = 'c',   // 99
    d              = 'd',   //100
    e              = 'e',   //101
    f              = 'f',   //102
    g              = 'g',   //103
    h              = 'h',   //104
    i              = 'i',   //105
    j              = 'j',   //106
    k              = 'k',   //107
    l              = 'l',   //108
    m              = 'm',   //109
    n              = 'n',   //110
    o              = 'o',   //111
    p              = 'p',   //112
    q              = 'q',   //113
    r              = 'r',   //114
    s              = 's',   //115
    t              = 't',   //116
    u              = 'u',   //117
    v              = 'v',   //118
    w              = 'w',   //119
    x              = 'x',   //120
    y              = 'y',   //121
    z              = 'z',   //122
    LEFT_BRACE     = '{',   //123
    VERTICAL_BAR   = '|',   //124
    RIGHT_BRACE    = '}',   //125
    TILDE          = '~',   //126

    // extended / non-ASCII keys (start above 127)
    F1             = 128, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    INSERT, DELETE_KEY,
    HOME, END,
    PAGE_UP, PAGE_DOWN,
    ARROW_UP, ARROW_DOWN, ARROW_LEFT, ARROW_RIGHT,
    LEFT_SHIFT, RIGHT_SHIFT,
    LEFT_CONTROL, RIGHT_CONTROL,
    LEFT_ALT, RIGHT_ALT,
    LEFT_SUPER, RIGHT_SUPER,
    KP_0, KP_1, KP_2, KP_3, KP_4,
    KP_5, KP_6, KP_7, KP_8, KP_9,
    KP_DECIMAL, KP_DIVIDE, KP_MULTIPLY,
    KP_SUBTRACT, KP_ADD, KP_ENTER, KP_EQUAL
};

inline std::ostream& operator<<(std::ostream& os, Key k) {
    return os << static_cast<int>(k);
}

#endif //KEY_H
