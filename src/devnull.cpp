/** @file devnull.cpp
 *  This file allows debugging printouts to be conveniently turned on and off.
 *  It contains a class which doesn't print anything when given things which 
 *  are not to be printed using the @c << operator, and it creates a single
 *  object called @c Debug. In @b this @b file are lines which either define or
 *  undefine the macro @c DEBUG_PRINT_OFF.
 * 
 *  * If the macro @c DEBUG_PRINT_OFF is defined, the @c Debug object will be a
 *    member of class @c DevNull (named for the Unix file @c /dev/null which is
 *    sort of a black hole for data, never mind the black hole quantum 
 *    information paradox) and whenever code sends information to @c Debug with
 *    the shift operator using code such as
 *    @code {.cpp}
 *    Debug << "Velocity is " << velocity << endl;
 *    @endcode
 *    Nothing will be printed (in fact, pretty much nothing will happen at all).
 * 
 *  * If the macro @c DEBUG_PRINT_OFF is undefined, the @c Debug object will be
 *    a reference to @c Serial and debugging information will be printed there.
 *
 *  * TODO: Add the ability to send debugging information to other children of
 *    class @c Print such as SD cards or other serial ports. 
 *
 *  @author JR Ridgely
 *  @date   2021-Nov-26 JRR Original file
 *
 *  License:
 *    This file is copyright 2021 by JR Ridgely and released under the
 *    Lesser GNU Public License, version 2. It intended for educational use 
 *    only, but its use is not limited thereto. */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIB-
 *    UTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 *    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *    THE POSSIBILITY OF SUCH DAMAGE. */

#include "devnull.h"


#ifdef DEBUG_PRINT_OFF
    /** If debugging printouts have been turned off, create an object of class
     *  @c DevNull which doesn't cause anything to be printed. This should be
     *  the only such object which ever needs to be created, unless there are
     *  several independently activated and/or deactivated debug-print things.
     */
    DevNull Debug;
#else
    /** If debugging printouts haven't been turned off, use the regular serial
     *  port @c Serial to print debugging messages.
    */
    Print& Debug = Serial;
#endif


// The following operators need not be documented, as their use has been
// described in the documentation for class DevNull and they're inactive copies
// of operators defined for Print child classes in PrintStream
/// @cond (!DOXYFILE_ENCODING)
DevNull& operator<< (DevNull& no, const __FlashStringHelper *s) { return no; }
DevNull& operator<< (DevNull& no, const String &s) { return no; }
DevNull& operator<< (DevNull& no, const char s[]) { return no; }
DevNull& operator<< (DevNull& no, char c) { return no; }
DevNull& operator<< (DevNull& no, unsigned char c) { return no; }
DevNull& operator<< (DevNull& no, int i) { return no; }
DevNull& operator<< (DevNull& no, unsigned int i) { return no; }
DevNull& operator<< (DevNull& no, int8_t i) { return no; }
DevNull& operator<< (DevNull& no, long i) { return no; }
DevNull& operator<< (DevNull& no, unsigned long i) { return no; }
DevNull& operator<< (DevNull& no, double d) { return no; }
DevNull& operator<< (DevNull& no, const Printable &p) { return no; }
DevNull& operator<< (DevNull& no, bool b) { return no; }
DevNull& operator<< (DevNull& no, manipulator pf) { return no; }
DevNull& operator<< (DevNull& no, _Setbase __f) { return no; }
DevNull& operator<< (DevNull& no, _Setprecision __f) { return no; }
DevNull& operator<< (DevNull& no, _Setbytesep __f) { return no; }
/// @endcond
