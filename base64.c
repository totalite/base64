
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * base64.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * base64.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with base64.c.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "base64.h"

/** */
enum special_e {
    notabase64 = 64, /**< Value to return when a non base64 digit is found. */
    terminator = 65, /**< Value to return when the character '=' is found.  */
};

/** Look at table that converts a base64 digit to integer. */
static unsigned char const digittobin[] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 65, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};


/* Convert a base64 null-terminated string to binary format.*/
void* b64tobin( void* dest, char const* src ) {
    unsigned char const* s = (unsigned char*)src;
    unsigned char* p = (unsigned char*)dest;
    for(;;) {

        unsigned int const a = digittobin[ *s ];
        if ( a == notabase64 ) return p;
        if ( a == terminator ) return p;

        unsigned int const b = digittobin[ *++s ];
        if ( b == notabase64 ) return 0;
        if ( b == terminator ) return 0;

        *p++ = ( a << 2 ) | ( b >> 4 );

        unsigned int const c = digittobin[ *++s ];
        if ( c == notabase64 ) return 0;

        unsigned int const d = digittobin[ *++s ];
        if ( d == notabase64 ) return 0;
        if ( c == terminator ) {
            if ( d != terminator ) return 0;
            return p;
        }

        *p++ = ( b << 4 ) | ( c >> 2 );

        if ( d == terminator ) return p;

        *p++ = ( c << 6 ) | ( d >> 0 );
        ++s;
    }
    
    return p;
}

/** Look at table that converts a integer to base64 digit. */
static char const bintodigit[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz"
                                 "0123456789"
                                 "+/";

/** Get the first base 64 digit of a block of 4.
  * @param a The first byte of the source block of 3.
  * @return A base 64 digit. */
static unsigned int get0( unsigned int a ) {
    unsigned int const index = a >> 2;
    return bintodigit[ index ];
}

/** Get the second base 64 digit of a block of 4.
  * @param a The first byte of the source block of 3.
  * @param b The second byte of the source block of 3.
  * @return A base 64 digit. */
static unsigned int get1( unsigned int a, unsigned int b ) {
    unsigned int const indexA = ( a & 0b11 ) << 4;
    unsigned int const indexB = b >> 4;
    unsigned int const index  = indexA | indexB;
    return bintodigit[ index ];
}

/** Get the third base 64 digit of a block of 4.
  * @param b The second byte of the source block of 3.
  * @param c The third byte of the source block of 3.
  * @return A base 64 digit. */
static unsigned int get2( unsigned int b, unsigned int c ) {
    unsigned int const indexB = ( b & 0b1111 ) << 2;
    unsigned int const indexC = c >> 6;
    unsigned int const index  = indexB | indexC;
    return bintodigit[ index ];
}

/** Get the fourth base 64 digit of a block of 4.
  * @param c The third byte of the source block of 3.
  * @return A base 64 digit. */
static unsigned int get3( unsigned int c ) {
    unsigned int const index = c & 0x3f;
    return bintodigit[ index ];
}

/* Convert a binary memory block in a base64 null-terminated string. */
char* bintob64( char* dest, void const* src, size_t size ) {

    typedef struct { unsigned char a; unsigned char b; unsigned char c; } block_t;
    block_t const* block = (block_t*)src;
    for( ; size >= sizeof( block_t ); size -= sizeof( block_t ), ++block ) {
        *dest++ = get0( block->a );
        *dest++ = get1( block->a, block->b );
        *dest++ = get2( block->b, block->c );
        *dest++ = get3( block->c );
    }

    if ( !size ) goto final;

    *dest++ = get0( block->a );
    if ( !--size ) {
        *dest++ = get1( block->a, 0 );
        *dest++ = '=';
        *dest++ = '=';
        goto final;
    }

    *dest++ = get1( block->a, block->b );
    *dest++ = get2( block->b, 0 );
    *dest++ = '=';

  final:
    *dest = '\0';
    return dest;
}
