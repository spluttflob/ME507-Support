/** @file devnull.h
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

#ifndef _DEVNULL_H_
#define _DEVNULL_H_

#include <PrintStream.h>

/// If defined, this macro causes debugging printouts to be suppressed.
// One of the following two lines should be active and the other commented out
#define DEBUG_PRINT_OFF
// #undef DEBUG_PRINT_OFF


/** @brief   Class which doesn't print anything when debugging printouts have
 *           been turned off.
 *  @details This class doesn't really do anything. It is provided as a target
 *           for definitions of @c operator<< as provided by the PrintStream
 *           library, which originally comes from
 *           @c https://github.com/tttapa/Arduino-PrintStream
 *           and has been forked on the Spluttflob ME507 GitHub site. 
 * 
 *           <b>For usage instructions, see the documentation for 
 *           @c devnull.h. </b>
 */
class DevNull
{
public:
    /** The constructor creates an inert debug printer which does nothing
     *  except take the place of a @c Print child class such as a serial port
     *  when one doesn't want debugging printouts but would rather not delete
     *  code which creates those printouts...because who knows when a bug may
     *  appear and those printouts will be needed again.
     */
    DevNull (void) { }
};


DevNull& operator<< (DevNull& no, const __FlashStringHelper *s);
DevNull& operator<< (DevNull& no, const String &s);
DevNull& operator<< (DevNull& no, const char s[]);
DevNull& operator<< (DevNull& no, char c);
DevNull& operator<< (DevNull& no, unsigned char c);
DevNull& operator<< (DevNull& no, int i);
DevNull& operator<< (DevNull& no, unsigned int i);
DevNull& operator<< (DevNull& no, int8_t i);
DevNull& operator<< (DevNull& no, long i);
DevNull& operator<< (DevNull& no, unsigned long i);
DevNull& operator<< (DevNull& no, double d);
DevNull& operator<< (DevNull& no, const Printable &p);
DevNull& operator<< (DevNull& no, bool b);
DevNull& operator<< (DevNull& no, manipulator pf);
DevNull& operator<< (DevNull& no, _Setbase __f);
DevNull& operator<< (DevNull& no, _Setprecision __f);
DevNull& operator<< (DevNull& no, _Setbytesep __f);


#ifdef DEBUG_PRINT_OFF
    /// If debugging printouts have been turned off, create an object of class
    /// @c DevNull which doesn't cause anything to be printed
    extern DevNull Debug;
#else
    /// If debugging printouts haven't been turned off, use the regular serial
    /// port @c Serial to print debugging messages
    extern Print& Debug;
#endif

#endif // _DEVNULL_H_
