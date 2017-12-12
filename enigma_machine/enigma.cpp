#include<string.h>
#include<fstream>
#define Nchars 69  // Total number of encipherable characters
#define Mchars 70  // Buffer size for strings containing Nchars
#define Nrotors 11 // Maximum number of rotors (1-based: 1-10)
#define Nrefls 5   // Total number of reflectors (1-based: 1-4)
#define Nsteps 11  // Maximum total number of encryption steps
                   // = 2*4 (rotors) + 2 (plugboard) + 1 (reflector)
//Adds function prototypes
void SetPlugboard();
void SetRotorsAndReflector();
void SetRotorPositions();
void ReportMachine();
void ProcessPlainText();
int OpenFiles( char *, char *, char * );
void CloseFiles();
void ShowRotors();
void PlaceRotor( int, int );
int index( char  );
int ChrToInt( char );
int mod( int , int );
void ShowWindow();
void ShowSteps();
char encrypt( char  );
char RtoLpath( char , int ); 
char LtoRpath( char , int );
void turn();
void TurnRot( int , int );
const char *ROTOR[ Nrotors ]  // Wirings of the rotors

   = {
     // input alphabet ("rotor" 0, not used)

     "abcdefghijklmnopqrstuvwxyz0123456789.,:; ()[]'\"-+/*&~`!@#$%^_={}|\\<>?",
     // rotor 1

     "ekmflgdqvzntowyhxuspaibrcj4.:5,63)-&;' +*7/\"](081[29?><\\|}{=^_%$#@!`~",
     // rotor 2

     "ajdksiruxblhwtmcqgznpyfvoe093.]8[\"/1,7+':2)6&;(*5- 4?><\\|}{=^_%$#@!`~",
     // rotor 3

     "bdfhjlcprtxvznyeiwgakmusqo13579,2(['/-&;*48+60.:\"]) ?><\\|}{=^_%$#@!`~",
     // rotor 4

     "esovpzjayquirhxlnftgkdcmwb4] -(&90*)\"8[7/,;5'6.32:+1?><\\|}{=^_%$#@!`~",
     // rotor 5

     "vzbrgityupsdnhlxawmjqofeck-&1[68'*\"(]3;7,/0+:9) 542.?><\\|}{=^_%$#@!`~",
     // rotor 6

     "jpgvoumfyqbenhzrdkasxlictw9(6- \":5*)14;7&[3.0]/,82'+?><\\|}{=^_%$#@!`~",
     // rotor 7

     "nzjhgrcxmyswboufaivlpekqdt;&976[2/:*]+1 \"508-,(4.)3'?><\\|}{=^_%$#@!`~",
     // rotor 8

     "fkqhtlxocbjspdzramewniuygv5.)7',/ 219](3&[0:4+;8\"*6-?><\\|}{=^_%$#@!`~",
     // beta rotor

     "leyjvcnixwpbqmdrtakzgfuhos,4*9-2;8/+(1):3['0.&65\"7 ]?><\\|}{=^_%$#@!`~",
     // gamma rotor

     "fsokanuerhmbtiycwlqpzxvgjd5] .0;\"4[7:1'8*2+,)(&/-693?><\\|}{=^_%$#@!`~"
     };
 
// Position in which each rotor causes its left neighbor to turn

// (The beta and gamma rotors could only be used in the Naval-Enigma

//  fourth position, and did not have knock-on effect.  So, their

//  notch positions are meaningless)

 
char NOTCH[ Nrotors ]
     = { 'z', 'q', 'e', 'v', 'j', 'z', 'z', 'z', 'z', 'a', 'a' };
 
const char *REFLECTOR[ Nrefls ]  // Reflectors

   = {
     // input alphabet ("REFLECTOR" 0, not used)

     "abcdefghijklmnopqrstuvwxyz0123456789.,:; ()[]'\"-+/*&~`!@#$%^_={}|\\<>?",
     // reflector B, thick

     "yruhqsldpxngokmiebfzcwvjat*[\"7)],3(/;6 .:8415&2+-90'?<>\\|}{=_^%$#@`!~",
     // reflector C, thick

     "fvpjiaoyedrzxwgctkuqsbnmhl5-(980 *43[&/+62'.\")]1;:7,?<>\\|}{=_^%$#@`!~",
     // reflector B, dunn

     "enkqeuywjicopblmdxzvfthrgs4;.)0\"*+982 (1,:3/&-5'7[6]?<>\\|}{=_^%$#@`!~",
     // reflector C, dunn

     "rdobjntkvehmlfcwzrxgyipsuq[3 19;'.-47:,52+&0/6*8(]\")?<>\\|}{=_^%$#@`!~"
     };
 
const char *PLUGBOARD  // Default wirings of the plugboard

     =
     "abcdefghijklmnopqrstuvwxyz0123456789.,:; ()[]'\"-+/*&~`!@#$%^_={}|\\<>?";
 
const char *alphabet  // Input alphabet

     =
     "abcdefghijklmnopqrstuvwxyz0123456789.,:; ()[]'\"-+/*&~`!@#$%^_={}|\\<>?";
 
int mRotors,                // Number of rotors placed in the machine

                            // (1-based: 1-4)

     mSteps;                // Actual number of encryption steps

                            // = 2*mRotors + 2 (plugboard) + 1 (reflector)

 int RotPos[ Nrotors ];     // Rotor placed in each position

char window[ Nrotors ],     // Characters in window

     Iwindow[ Nrotors ];    // Initial values in 'window' (for resetting)

char *RotWiring[ Nrotors ]; // Rotor wirings

char RotNotch[ Nrotors ];   // Rotor switching positions

 int RotNumber[ Nrotors ];  // Which physical rotor (t,1-8,b,g)

char *reflector,            // Wiring of the reflector

     plugboard[ Mchars ];   // Wirings of the plugboard

 int ReflType;              // Reflector used

char step[ Nsteps ];        // Array to store encryption steps

 
// Files and variables for setup and reporting

 
#define Nline 255
 
FILE *inFp,             // input file pointer

     *outFp,            // output file pointer

     *logFp;            // log file pointer

char inLine[ Nline ],   // input line

     outLine[ Nline ];  // output line

void InitEnigma() // Default initialization

{
   int i;
 
   mRotors = 3;
   mSteps = (mRotors << 1) + 3;
   strcpy( plugboard, PLUGBOARD );
   for ( i = 0; i <= mRotors; ++i ) {
      RotWiring[ i ] = (char*)ROTOR[ i ];
      RotNotch[ i ] = NOTCH[ i ];
      RotNumber[ i ] = i;
      Iwindow[ i ] = window[ i ] = 'a';
   }
   reflector = (char*)REFLECTOR[ 1 ];
   ReflType = 1;
}

void TryUserSetup() 
// Attempt initialization from user file

{
   if ( (inFp = fopen( "esetup", "rt" ))
        !=
        NULL )
   {
      SetPlugboard();
      SetRotorsAndReflector();
      fclose( inFp );
   }
}

void ProcessFile( const char *inFname,
                  const char *encFname,
                  const char *logFname )
{
   if ( OpenFiles( (char*)inFname,
                   (char*)encFname,
                   (char*)logFname ) ) {
      SetRotorPositions();
      ReportMachine();
      ProcessPlainText();   
      CloseFiles();
   }
}

void reset()
{
   for ( int i = 1; i <= mRotors; ++i )
      window[ i ] = Iwindow[ i ];
}

int OpenFiles( char *inFname,
               char *encFname,
               char *logFname )
{
   inFp = fopen( inFname, "rt" );
   outFp = fopen( encFname, "wt" );
   logFp = fopen( logFname, "wt" );
   return    (inFp != NULL) && (outFp != NULL) && (logFp != NULL);
}

void CloseFiles()
{
   fclose( inFp ); fclose( outFp ); fclose( logFp );
}

void ReportMachine()
{
   fprintf( logFp, "Plugboard mappings:\n" );
   fprintf( logFp, "%s\n", ROTOR[ 0 ] );
   fprintf( logFp, "%s\n", plugboard );
 
   fprintf( logFp, "\nRotor wirings:\n" );
   fprintf( logFp, "position rotor ring setting notch sequence\n" );
   for ( int i = mRotors; i >= 1; --i )
      fprintf( logFp, "%8d %5d %12c %5c %s\n",
                      i, RotNumber[ i ], window[ i ],
                      RotNotch[ i ], RotWiring[ i ] );
   fprintf( logFp, "\nreflector %c %s\n", ReflType, reflector );
   fprintf( logFp, "\nrotors:\n" );
   ShowRotors();
}

void ShowRotors()
{
    int i, j, k;
   char *Rwiring;
 
   for ( i = mRotors; i >= 1; --i ) {
      fprintf( logFp, "%d: ", i );
      Rwiring = RotWiring[ i ];
      k = RotPos[ i ];
      for ( j = 0; j < k; ++j )
         fprintf( logFp, "%c", *Rwiring++ );
      fprintf( logFp, "->" );
      for ( j = k; j < Nchars; ++j )
         fprintf( logFp, "%c", *Rwiring++ );
      fprintf( logFp, "\n" );
   }
}

// Initialization from file 'esetup' (step 1)

 
void SetPlugboard()  // make connections on the plug board

{
    int i, n, x;
   char p1, p2, ch;
 
   // Read a line containing pairs of letters corresponding to pairs of

   // plugs connected by a wire.  The length of the line is assumed to be even.

 
   fgets( inLine, Nline, inFp );
   inLine[ strlen( inLine ) - 1 ] = '\0';
   n = strlen( inLine );
 
   for ( i = 0; i < n; i += 2 ) {
      p1 = inLine[ i ];
      p2 = inLine[ i+1 ];
      x = index( p1 );
      if ( (ch = plugboard[ x ]) != p1 ) { // occupied? -> disconnect

         plugboard[ index( ch ) ] = ch;
         plugboard[ x ] = p1;
      }
      plugboard[ x ] = p2;                 // plug in

      x = index( p2 );
      if ( (ch = plugboard[ x ]) != p2) { // occupied? -> disconnect

         plugboard[ index( ch ) ] = ch;
         plugboard[ x ] = p1;
      }
      plugboard[ x ] = p1;                // plug in

   }
}
 
// Initialization from file 'esetup' (step 2)

 
void SetRotorsAndReflector()
{
    int i, n, rotor, rotPos;
   char ch, ringPos;
 
   // Read the actual number of rotors "mRotors", compute the total

   // number of encryption steps "mSteps", and read "mRotors" lines,

   // each one containing three characters denoting

   //

   //   (a) rotor ID (1-8,b,g)

   //   (b) rotor position (1-mRotors), and

   //   (c) ring character (position)

 
   fgets( inLine, Nline, inFp );
   mRotors = ChrToInt( inLine[ 0 ] );
   if ( mRotors > 4 )
      mRotors = 4;
   mSteps = (mRotors << 1) + 3;
   for ( i = 1; i <= mRotors; ++i ) {
      fgets( inLine, Nline, inFp );
      ch = inLine[ 0 ];
      if ( isdigit( (int)ch ) )
         rotor = ChrToInt( ch );
      else {
         ch = tolower( ch );
         rotor = ch == 'b' ? 9
                           : ch == 'g' ? 10 : 0;
      }
      rotPos = ChrToInt( inLine[ 1 ] );
      ringPos = inLine[ 2 ];
      Iwindow[ rotPos ] = window[ rotPos ] = ringPos;
      PlaceRotor( rotPos, rotor );
   }
 
   // Read a line containing the designation of the reflector (t,b,c,B,C)

 
   fgets( inLine, Nline, inFp );
   ch = inLine[ 0 ];
   switch ( ch ) {
      case 't': n = 0; break;      case 'b': n = 1; break;
      case 'c': n = 2; break;      case 'B': n = 3; break;
      case 'C': n = 4; break;       default: n = 0; break;
   }
   reflector = (char*)REFLECTOR[ n ];
   ReflType = i;
}

int index( char c )
{
   // c in alphabet

 
   int i = 0;
 
   while ( (i < Nchars) && (c != alphabet[ i ]) )
      ++i;
   return i;
}

int ChrToInt( char c )
{
   // '0' <= c <= '9'

 
   return (int)( c - '0' );
}

void PlaceRotor( int position, int r ) 
// set wirings of a single rotor

{
   RotWiring[ position ] = (char*)ROTOR[ r ];
   RotNotch[ position ] = NOTCH[ r ];
   RotNumber[ position ] = r;
}

void SetRotorPositions()
{
    int i, j, k;
   char *Rwiring, ch;
 
   for ( i = 1; i <= mRotors; ++i ) {
      j = RotNumber[ i ];
      ch = window[ j ];
      Rwiring = RotWiring[ j ];
      k = 0;
      while ( Rwiring[ k ] != ch )
         ++k;
      RotPos[ j ] = k;
   }
}

int mod( int n, int modulus )  // simple modulo function

{
   while ( n >= modulus )
      n -= modulus;
   while ( n < 0 )
      n += modulus;
   return n;
}

void ProcessPlainText()
{
    int i, n;
   char c1, c2;
 
   fprintf( logFp, "\n\nEncryption\n" );
   while ( fgets( inLine, Nline, inFp ) != NULL ) {
 
      n = strlen( inLine ) - 1;
      inLine[ n ] = '\0';
 
      for ( i = 0; i < n; ++i ) {
         c1 = inLine[ i ];
         if ( isupper( (int)c1 ) )
            c1 = tolower( c1 );
 
         c2 = encrypt( c1 );
 
         // ShowRotors();

         ShowWindow();
         fprintf( logFp, " %c", c1 );
         ShowSteps();
         fprintf( logFp, "\n" );
         outLine[ i ] = c2;
      }
      fprintf( logFp, "\n" );
      outLine[ i ] = '\0';
      fprintf( outFp, "%s\n", outLine );
   }
}

void ShowWindow()
{
   int i;
 
   for ( i = mRotors; i >= 1; --i )
      fprintf( logFp, "%c ", window[ i ] );
   fprintf( logFp, "  " );
}

void ShowSteps()
{
   int i;
 
   for ( i = 0; i < mSteps; ++i )
     fprintf( logFp, " -> %c", step[ i ] );
}

char encrypt( char c )
    {
       int i, r;
 
       turn();                                          // move rotors

       i = 0;                                           // pass through:

       step[ i++ ] = plugboard[ index( c ) ];           //    plugboard

       for ( r = 1; r <= mRotors; ++r )
          step[ i++ ] = RtoLpath( step[ i-1 ], r );     //    right-to-left path

       step[ i++ ] = reflector[ index( step[ i-1 ] ) ]; //    reflector

       for ( r = mRotors; r >= 1; --r )                 //    left-to-right path

          step[ i++ ] = LtoRpath( step[ i-1 ], r );
       step[ i ] = plugboard[ index( step[ i-1 ] ) ];   //    plugboard

 
       return step[ i ];
    } 
 
void turn()   // determine which rotors must turn

{
    int doit[ Nrotors ], n;
   char *r1 = RotWiring[ 1 ], *r2 = RotWiring[ 2 ], *r3;
 
   if ( mRotors > 3 )
      r3 = RotWiring[ 3 ];
 
    // calculate stepwidth for each rotor

    doit[ 1 ] = 1;
    for ( int i = 2; i <= mRotors; ++i )
       doit[ i ] = 0;
    if ( (RotNotch[ 1 ] == r1[ RotPos[ 1 ] ])
         ||
         (RotNotch[ 2 ] == r2[ RotPos[ 2 ] ]) )  // double stepping

       doit[ 2 ] = 1;
    if ( RotNotch[ 2 ] == r2[ RotPos[ 2 ] ] )
       doit[ 3 ] = 1;
    if ( mRotors > 3 ) {
       if ( RotNotch[ 3 ] == r3[ RotPos[ 3 ] ] )
          doit[ 4 ] = 1;
    }
 
    // turn rotors "simultaneously"

    for ( n = 1; n <= mRotors; ++n )
       TurnRot( n, doit[ n ] );
}
 
void TurnRot( int n, int width )   // Turn rotor "n" "width" times

{                                  // wrapping around if necessary

   char *r;
 
   if ( width > 0 ) {
      RotPos[ n ] = mod( RotPos[ n ] + width, Nchars );
      r = RotWiring[ n ];
      window[ n ] = r[ RotPos[ n ] ];
   }
}

// Transform on right-to-left path through rotors

char RtoLpath( char c, int r )  // transform character "c" with rotor "r"
{
    int n, offset, idx, ret;
   char *CurRotor;

   CurRotor = RotWiring[ r ];
   n = index( c );
   offset = index( CurRotor[ RotPos[ r ] ] );
   idx = mod( n + offset, Nchars );
   ret = mod( index( CurRotor[ idx ] ) - offset, Nchars );
   return alphabet[ ret ];
}

// Transform on left-to-right path through rotors

char LtoRpath( char c, int r ) // transform character "c" with rotor "r"
{
    int n, m, offset, idx, newchar;
   char *CurRotor;

   CurRotor = RotWiring[ r ];
   n = index( c );
   offset = index( CurRotor[ RotPos[ r ] ] );
   newchar = alphabet[ mod( n+offset, Nchars ) ];

   m = 0;
   while ( m < Nchars && CurRotor[ m ] != newchar )
      ++m;
   idx = mod( m - offset, Nchars );
   return alphabet[ idx ];
}

int main()
{
   InitEnigma();
   TryUserSetup();
 
   ProcessFile( "plain", "encrypt", "elog" );
   reset();
   ProcessFile( "encrypt", "decrypt", "dlog" );

   return 0;
}
