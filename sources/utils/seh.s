
/*******************************************************************************
 *                                                                             *
 * seh.s - Platform specific SEH functions for i486+ (32-bit)                  *
 *                                                                             *
 * LIBSEH - Structured Exception Handling compatibility library.               *
 * Copyright (c) 2011 Tom Bramer < tjb at postpro dot net >                    *
 *                                                                             *
 * Permission is hereby granted, free of charge, to any person                 *
 * obtaining a copy of this software and associated documentation              *
 * files (the "Software"), to deal in the Software without                     *
 * restriction, including without limitation the rights to use,                *
 * copy, modify, merge, publish, distribute, sublicense, and/or sell           *
 * copies of the Software, and to permit persons to whom the                   *
 * Software is furnished to do so, subject to the following                    *
 * conditions:                                                                 *
 *                                                                             *
 * The above copyright notice and this permission notice shall be              *
 * included in all copies or substantial portions of the Software.             *
 *                                                                             *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,             *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES             *
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                    *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT                 *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,                *
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING                *
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR               *
 * OTHER DEALINGS IN THE SOFTWARE.                                             *
 *                                                                             *
 *******************************************************************************/

// SEH library functions... very platform specific

#define STDCALL_SYM(x, y)  _ ## x ##@## y
#define CDECL_SYM(x) _ ## x

.global STDCALL_SYM(__libseh_get_registration,0);
.global STDCALL_SYM(__libseh_set_registration,4);
.global STDCALL_SYM(__libseh_pop_registration,0);
.global STDCALL_SYM(__libseh_unwind_to,4);

/*
 * __libseh_get_registration: returns the last registered handler registration off the
 *                            handler stack.
 *
 * Return value: __libseh_buf*  pointer to the handler block.
 *
 */

STDCALL_SYM(__libseh_get_registration,0):
    movl %fs:0, %eax;
    ret;

/*
 * __libseh_set_registration: sets the registration handler to the given argument.  Linked
 *                            list of exception handlers must be maintained by the caller.
 *
 * Parameters: in __libseh_buf*  the new exception handler registration structure.
 *
 */

STDCALL_SYM(__libseh_set_registration,4):
    movl 4(%esp), %eax;
    movl %eax, %fs:0;
    xorl %eax, %eax;
    ret $4;


/*
 * __libseh_pop_registration: like __libseh_unregister, but does not release any resources associated
 *                            with the registration block.
 *
 * Return value: __libseh_buf*  pointer to the new top of the handler stack.
 *
 */

STDCALL_SYM(__libseh_pop_registration,0):
    movl %fs:0, %eax;
    movl 0(%eax), %eax;
    movl %eax, %fs:0;
    ret;

STDCALL_SYM(__libseh_unwind_to,4):
    movl 4(%esp), %ebp;
    leave;
    ret;

