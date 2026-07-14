/*====================================================================
Copyright 2006 Tobias Weyand

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
======================================================================*/

#include "typewriter_hit.h"

// 26x12 Tiles

const unsigned char typewriter_Hit[312] = {
    0x0, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, '1',
    '1', '2',  '2',  '3',  '3',  '4', '4', '5', '5', '6', '6', '7', '7', '8',
    '8', '9',  '9',  '0',  '0',  '-', '-', '=', '=', 0x0, 0x0, '1', '1', '2',
    '2', '3',  '3',  '4',  '4',  '5', '5', '6', '6', '7', '7', '8', '8', '9',
    '9', '0',  '0',  '-',  '-',  '=', '=', 0x0, 0x0, 0x0, 'q', 'q', 'w', 'w',
    'e', 'e',  'r',  'r',  't',  't', 'y', 'y', 'u', 'u', 'i', 'i', 'o', 'o',
    'p', 'p',  BSP,  BSP,  BSP,  0x0, 0x0, 0x0, 'q', 'q', 'w', 'w', 'e', 'e',
    'r', 'r',  't',  't',  'y',  'y', 'u', 'u', 'i', 'i', 'o', 'o', 'p', 'p',
    BSP, BSP,  BSP,  0x0,  0x0,  CAP, CAP, 'a', 'a', 's', 's', 'd', 'd', 'f',
    'f', 'g',  'g',  'h',  'h',  'j', 'j', 'k', 'k', 'l', 'l', RET, RET, RET,
    RET, 0x0,  0x0,  CAP,  CAP,  'a', 'a', 's', 's', 'd', 'd', 'f', 'f', 'g',
    'g', 'h',  'h',  'j',  'j',  'k', 'k', 'l', 'l', RET, RET, RET, RET, 0x0,
    0x0, SHF,  SHF,  SHF,  'z',  'z', 'x', 'x', 'c', 'c', 'v', 'v', 'b', 'b',
    'n', 'n',  'm',  'm',  ',',  ',', '.', '.', '/', '/', 0x0, 0x0, 0x0, SHF,
    SHF, SHF,  'z',  'z',  'x',  'x', 'c', 'c', 'v', 'v', 'b', 'b', 'n', 'n',
    'm', 'm',  ',',  ',',  '.',  '.', '/', '/', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, ';',  ';',  '\'', '\'', SPC, SPC, SPC, SPC, SPC, SPC, SPC, SPC, SPC,
    SPC, '[',  '[',  ']',  ']',  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, ';',
    ';', '\'', '\'', SPC,  SPC,  SPC, SPC, SPC, SPC, SPC, SPC, SPC, SPC, '[',
    '[', ']',  ']',  0x0,  0x0,  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0,  0x0
};

const unsigned char typewriter_Hit_Shift[312] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, '!', '!', '@',
    '@', '#', '#', '$', '$', '%', '%', '^', '^', '&', '&', '*', '*', '(', '(',
    ')', ')', '_', '_', '+', '+', 0x0, 0x0, '!', '!', '@', '@', '#', '#', '$',
    '$', '%', '%', '^', '^', '&', '&', '*', '*', '(', '(', ')', ')', '_', '_',
    '+', '+', 0x0, 0x0, 0x0, 'Q', 'Q', 'W', 'W', 'E', 'E', 'R', 'R', 'T', 'T',
    'Y', 'Y', 'U', 'U', 'I', 'I', 'O', 'O', 'P', 'P', BSP, BSP, BSP, 0x0, 0x0,
    0x0, 'Q', 'Q', 'W', 'W', 'E', 'E', 'R', 'R', 'T', 'T', 'Y', 'Y', 'U', 'U',
    'I', 'I', 'O', 'O', 'P', 'P', BSP, BSP, BSP, 0x0, 0x0, CAP, CAP, 'A', 'A',
    'S', 'S', 'D', 'D', 'F', 'F', 'G', 'G', 'H', 'H', 'J', 'J', 'K', 'K', 'L',
    'L', RET, RET, RET, RET, 0x0, 0x0, CAP, CAP, 'A', 'A', 'S', 'S', 'D', 'D',
    'F', 'F', 'G', 'G', 'H', 'H', 'J', 'J', 'K', 'K', 'L', 'L', RET, RET, RET,
    RET, 0x0, 0x0, SHF, SHF, SHF, 'Z', 'Z', 'X', 'X', 'C', 'C', 'V', 'V', 'B',
    'B', 'N', 'N', 'M', 'M', '<', '<', '>', '>', '?', '?', 0x0, 0x0, 0x0, SHF,
    SHF, SHF, 'Z', 'Z', 'X', 'X', 'C', 'C', 'V', 'V', 'B', 'B', 'N', 'N', 'M',
    'M', '<', '<', '>', '>', '?', '?', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, ':',
    ':', '~', '~', SPC, SPC, SPC, SPC, SPC, SPC, SPC, SPC, SPC, SPC, '{', '{',
    '}', '}', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, ':', ':', '~', '~', SPC,
    SPC, SPC, SPC, SPC, SPC, SPC, SPC, SPC, SPC, '{', '{', '}', '}', 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};
