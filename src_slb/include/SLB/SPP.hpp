/*
    SLB - Simple Lua Binder
    Copyright (C) 2007-2011 Jose L. Hidalgo Valiño (PpluX)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  
  Jose L. Hidalgo (www.pplux.com)
  pplux@pplux.com
*/



#ifndef __SIMPLE_PREPROCESSOR__
#define __SIMPLE_PREPROCESSOR__

  #define SPP_CONCATENATE_MACRO(x, y)   x##y
  #define SPP_CONCATENATE_MACRO3(x,y,z) SPP_CONCATENATE_MACRO(x##y,z)
  #define SPP_LINEID1(x, y)             SPP_CONCATENATE_MACRO(x, y)
  #define SPP_LINEID(x)                 SPP_LINEID1(x, __LINE__)
  #define SPP_STRING(X)                 #X
  #define SPP_TOSTRING(X)               SPP_STRING(X)

  #define SPP_STATIC_BLOCK(...) \
      namespace { \
    static struct SPP_LINEID(DUMMY) { SPP_LINEID(DUMMY)() {      \
      __VA_ARGS__ \
    }                                    \
    } SPP_LINEID(__dummy__); }

  //enumerates "X" from 1 to num with "INTER" as separator
  // SPP_ENUM(3,class T, = void SPP_COMMA) --> class T1 = void, class T2 = void, class T3 = void
  #define SPP_ENUM(num, X, INTER)                  SPP_ENUM_ ## num (X, SPP_UNPAR(INTER) )

  // enumerates using COMMA as separator 
  #define SPP_ENUM_D(num,X)                        SPP_ENUM_ ## num (X, SPP_COMMA)

  // Repeats "X" which should be a macro(N) num times, REPEAT_Z starts from 0
  // #define DO(N) T##N;
  // SPP_REPEAT(3,DO) --> DO(1) DO(2) DO(3) --> T1; T2; T3;
  #define SPP_REPEAT(num, X)                       SPP_REPEAT_ ## num (SPP_UNPAR(X))
  #define SPP_REPEAT_Z(num, X)                     X(0) SPP_REPEAT_ ## num (SPP_UNPAR(X))
  
  #define SPP_REPEAT_BASE(num, X, base)            SPP_REPEAT_BASE_ ## num \
                                                              (SPP_UNPAR(X),SPP_UNPAR(base))
  #define SPP_REPEAT_BASE_Z(num, X, base)          X(0,base) SPP_REPEAT_BASE_ ## num \
                                                              (SPP_UNPAR(X), SPP_UNPAR(base))

  #define SPP_IF(num,X)                            SPP_IF_ ## num (SPP_UNPAR(X))

  #define SPP_MAIN_REPEAT(num, X)                  SPP_MAIN_REPEAT_ ## num (SPP_UNPAR(X))
  #define SPP_MAIN_REPEAT_Z(num, X)                X(0) SPP_MAIN_REPEAT_ ## num (SPP_UNPAR(X))
  #define SPP_COMMA                                SPP_UNPAR(,)
  #define SPP_SPACE 
  #define SPP_COMMA_IF(num)                        SPP_IF(num,SPP_COMMA)

  #define SPP_UNPAR(...) __VA_ARGS__

  #define SPP_ENUM_0(X, INTER)  
  #define SPP_ENUM_1(X, INTER)  X##1
  #define SPP_ENUM_2(X, INTER)  SPP_ENUM_1(X,  SPP_UNPAR(INTER)) INTER X##2
  #define SPP_ENUM_3(X, INTER)  SPP_ENUM_2(X,  SPP_UNPAR(INTER)) INTER X##3
  #define SPP_ENUM_4(X, INTER)  SPP_ENUM_3(X,  SPP_UNPAR(INTER)) INTER X##4
  #define SPP_ENUM_5(X, INTER)  SPP_ENUM_4(X,  SPP_UNPAR(INTER)) INTER X##5
  #define SPP_ENUM_6(X, INTER)  SPP_ENUM_5(X,  SPP_UNPAR(INTER)) INTER X##6
  #define SPP_ENUM_7(X, INTER)  SPP_ENUM_6(X,  SPP_UNPAR(INTER)) INTER X##7
  #define SPP_ENUM_8(X, INTER)  SPP_ENUM_7(X,  SPP_UNPAR(INTER)) INTER X##8
  #define SPP_ENUM_9(X, INTER)  SPP_ENUM_8(X,  SPP_UNPAR(INTER)) INTER X##9
  #define SPP_ENUM_10(X, INTER) SPP_ENUM_9(X,  SPP_UNPAR(INTER)) INTER X##10
  #define SPP_ENUM_11(X, INTER) SPP_ENUM_10(X, SPP_UNPAR(INTER)) INTER X##11
  #define SPP_ENUM_12(X, INTER) SPP_ENUM_11(X, SPP_UNPAR(INTER)) INTER X##12
  #define SPP_ENUM_13(X, INTER) SPP_ENUM_12(X, SPP_UNPAR(INTER)) INTER X##13
  #define SPP_ENUM_14(X, INTER) SPP_ENUM_13(X, SPP_UNPAR(INTER)) INTER X##14
  #define SPP_ENUM_15(X, INTER) SPP_ENUM_14(X, SPP_UNPAR(INTER)) INTER X##15
  #define SPP_ENUM_16(X, INTER) SPP_ENUM_15(X, SPP_UNPAR(INTER)) INTER X##16
  #define SPP_ENUM_17(X, INTER) SPP_ENUM_16(X, SPP_UNPAR(INTER)) INTER X##17
  #define SPP_ENUM_18(X, INTER) SPP_ENUM_17(X, SPP_UNPAR(INTER)) INTER X##18
  #define SPP_ENUM_19(X, INTER) SPP_ENUM_18(X, SPP_UNPAR(INTER)) INTER X##19
  #define SPP_ENUM_20(X, INTER) SPP_ENUM_19(X, SPP_UNPAR(INTER)) INTER X##20
  #define SPP_ENUM_21(X, INTER) SPP_ENUM_20(X, SPP_UNPAR(INTER)) INTER X##21
  #define SPP_ENUM_22(X, INTER) SPP_ENUM_21(X, SPP_UNPAR(INTER)) INTER X##22
  #define SPP_ENUM_23(X, INTER) SPP_ENUM_22(X, SPP_UNPAR(INTER)) INTER X##23
  #define SPP_ENUM_24(X, INTER) SPP_ENUM_23(X, SPP_UNPAR(INTER)) INTER X##24
  #define SPP_ENUM_25(X, INTER) SPP_ENUM_24(X, SPP_UNPAR(INTER)) INTER X##25
  #define SPP_ENUM_26(X, INTER) SPP_ENUM_25(X, SPP_UNPAR(INTER)) INTER X##26
  #define SPP_ENUM_27(X, INTER) SPP_ENUM_26(X, SPP_UNPAR(INTER)) INTER X##27
  #define SPP_ENUM_28(X, INTER) SPP_ENUM_27(X, SPP_UNPAR(INTER)) INTER X##28
  #define SPP_ENUM_29(X, INTER) SPP_ENUM_28(X, SPP_UNPAR(INTER)) INTER X##29
  #define SPP_ENUM_30(X, INTER) SPP_ENUM_29(X, SPP_UNPAR(INTER)) INTER X##30
  #define SPP_ENUM_31(X, INTER) SPP_ENUM_30(X, SPP_UNPAR(INTER)) INTER X##31
  #define SPP_ENUM_32(X, INTER) SPP_ENUM_31(X, SPP_UNPAR(INTER)) INTER X##32
  #define SPP_ENUM_MAX(X, INTER) SPP_ENUM_10(X, INTER) /* Change this up to 32 */

  #define SPP_REPEAT_0(X) 
  #define SPP_REPEAT_1(X) X(1)
  #define SPP_REPEAT_2(X) SPP_REPEAT_1(X) X(2) 
  #define SPP_REPEAT_3(X) SPP_REPEAT_2(X) X(3) 
  #define SPP_REPEAT_4(X) SPP_REPEAT_3(X) X(4) 
  #define SPP_REPEAT_5(X) SPP_REPEAT_4(X) X(5) 
  #define SPP_REPEAT_6(X) SPP_REPEAT_5(X) X(6) 
  #define SPP_REPEAT_7(X) SPP_REPEAT_6(X) X(7) 
  #define SPP_REPEAT_8(X) SPP_REPEAT_7(X) X(8) 
  #define SPP_REPEAT_9(X) SPP_REPEAT_8(X) X(9) 
  #define SPP_REPEAT_10(X) SPP_REPEAT_9(X) X(10) 
  #define SPP_REPEAT_11(X) SPP_REPEAT_10(X) X(11) 
  #define SPP_REPEAT_12(X) SPP_REPEAT_11(X) X(12) 
  #define SPP_REPEAT_13(X) SPP_REPEAT_12(X) X(13) 
  #define SPP_REPEAT_14(X) SPP_REPEAT_13(X) X(14) 
  #define SPP_REPEAT_15(X) SPP_REPEAT_14(X) X(15) 
  #define SPP_REPEAT_16(X) SPP_REPEAT_15(X) X(16) 
  #define SPP_REPEAT_17(X) SPP_REPEAT_16(X) X(17) 
  #define SPP_REPEAT_18(X) SPP_REPEAT_17(X) X(18) 
  #define SPP_REPEAT_19(X) SPP_REPEAT_18(X) X(19) 
  #define SPP_REPEAT_20(X) SPP_REPEAT_19(X) X(20) 
  #define SPP_REPEAT_21(X) SPP_REPEAT_20(X) X(21) 
  #define SPP_REPEAT_22(X) SPP_REPEAT_21(X) X(22) 
  #define SPP_REPEAT_23(X) SPP_REPEAT_22(X) X(23) 
  #define SPP_REPEAT_24(X) SPP_REPEAT_23(X) X(24) 
  #define SPP_REPEAT_25(X) SPP_REPEAT_24(X) X(25) 
  #define SPP_REPEAT_26(X) SPP_REPEAT_25(X) X(26) 
  #define SPP_REPEAT_27(X) SPP_REPEAT_26(X) X(27) 
  #define SPP_REPEAT_28(X) SPP_REPEAT_27(X) X(28) 
  #define SPP_REPEAT_29(X) SPP_REPEAT_28(X) X(29) 
  #define SPP_REPEAT_30(X) SPP_REPEAT_29(X) X(30) 
  #define SPP_REPEAT_31(X) SPP_REPEAT_30(X) X(31) 
  #define SPP_REPEAT_32(X) SPP_REPEAT_31(X) X(32) 
  #define SPP_REPEAT_MAX(X) SPP_REPEAT_10(X)  /* Change this up to 32 */

  #define SPP_REPEAT_BASE_0(X,base) 
  #define SPP_REPEAT_BASE_1(X,base) X(1, base)
  #define SPP_REPEAT_BASE_2(X,base) SPP_REPEAT_BASE_1(X,base) X(2, base) 
  #define SPP_REPEAT_BASE_3(X,base) SPP_REPEAT_BASE_2(X,base) X(3, base) 
  #define SPP_REPEAT_BASE_4(X,base) SPP_REPEAT_BASE_3(X,base) X(4, base) 
  #define SPP_REPEAT_BASE_5(X,base) SPP_REPEAT_BASE_4(X,base) X(5, base) 
  #define SPP_REPEAT_BASE_6(X,base) SPP_REPEAT_BASE_5(X,base) X(6, base) 
  #define SPP_REPEAT_BASE_7(X,base) SPP_REPEAT_BASE_6(X,base) X(7, base) 
  #define SPP_REPEAT_BASE_8(X,base) SPP_REPEAT_BASE_7(X,base) X(8, base) 
  #define SPP_REPEAT_BASE_9(X,base) SPP_REPEAT_BASE_8(X,base) X(9, base) 
  #define SPP_REPEAT_BASE_10(X,base) SPP_REPEAT_BASE_9(X,base) X(10, base) 
  #define SPP_REPEAT_BASE_11(X,base) SPP_REPEAT_BASE_10(X,base) X(11, base) 
  #define SPP_REPEAT_BASE_12(X,base) SPP_REPEAT_BASE_11(X,base) X(12, base) 
  #define SPP_REPEAT_BASE_13(X,base) SPP_REPEAT_BASE_12(X,base) X(13, base) 
  #define SPP_REPEAT_BASE_14(X,base) SPP_REPEAT_BASE_13(X,base) X(14, base) 
  #define SPP_REPEAT_BASE_15(X,base) SPP_REPEAT_BASE_14(X,base) X(15, base) 
  #define SPP_REPEAT_BASE_16(X,base) SPP_REPEAT_BASE_15(X,base) X(16, base) 
  #define SPP_REPEAT_BASE_17(X,base) SPP_REPEAT_BASE_16(X,base) X(17, base) 
  #define SPP_REPEAT_BASE_18(X,base) SPP_REPEAT_BASE_17(X,base) X(18, base) 
  #define SPP_REPEAT_BASE_19(X,base) SPP_REPEAT_BASE_18(X,base) X(19, base) 
  #define SPP_REPEAT_BASE_20(X,base) SPP_REPEAT_BASE_19(X,base) X(20, base) 
  #define SPP_REPEAT_BASE_21(X,base) SPP_REPEAT_BASE_20(X,base) X(21, base) 
  #define SPP_REPEAT_BASE_22(X,base) SPP_REPEAT_BASE_21(X,base) X(22, base) 
  #define SPP_REPEAT_BASE_23(X,base) SPP_REPEAT_BASE_22(X,base) X(23, base) 
  #define SPP_REPEAT_BASE_24(X,base) SPP_REPEAT_BASE_23(X,base) X(24, base) 
  #define SPP_REPEAT_BASE_25(X,base) SPP_REPEAT_BASE_24(X,base) X(25, base) 
  #define SPP_REPEAT_BASE_26(X,base) SPP_REPEAT_BASE_25(X,base) X(26, base) 
  #define SPP_REPEAT_BASE_27(X,base) SPP_REPEAT_BASE_26(X,base) X(27, base) 
  #define SPP_REPEAT_BASE_28(X,base) SPP_REPEAT_BASE_27(X,base) X(28, base) 
  #define SPP_REPEAT_BASE_29(X,base) SPP_REPEAT_BASE_28(X,base) X(29, base) 
  #define SPP_REPEAT_BASE_30(X,base) SPP_REPEAT_BASE_29(X,base) X(30, base) 
  #define SPP_REPEAT_BASE_31(X,base) SPP_REPEAT_BASE_30(X,base) X(31, base) 
  #define SPP_REPEAT_BASE_32(X,base) SPP_REPEAT_BASE_31(X,base) X(32, base) 
  #define SPP_REPEAT_BASE_MAX(X,base) SPP_REPEAT_BASE_10(X,base)  /* Change this up to 32 */

  #define SPP_MAIN_REPEAT_1(X) X(1)
  #define SPP_MAIN_REPEAT_2(X) SPP_MAIN_REPEAT_1(X) X(2) 
  #define SPP_MAIN_REPEAT_3(X) SPP_MAIN_REPEAT_2(X) X(3) 
  #define SPP_MAIN_REPEAT_4(X) SPP_MAIN_REPEAT_3(X) X(4) 
  #define SPP_MAIN_REPEAT_5(X) SPP_MAIN_REPEAT_4(X) X(5) 
  #define SPP_MAIN_REPEAT_6(X) SPP_MAIN_REPEAT_5(X) X(6) 
  #define SPP_MAIN_REPEAT_7(X) SPP_MAIN_REPEAT_6(X) X(7) 
  #define SPP_MAIN_REPEAT_8(X) SPP_MAIN_REPEAT_7(X) X(8) 
  #define SPP_MAIN_REPEAT_9(X) SPP_MAIN_REPEAT_8(X) X(9) 
  #define SPP_MAIN_REPEAT_10(X) SPP_MAIN_REPEAT_9(X) X(10) 
  #define SPP_MAIN_REPEAT_11(X) SPP_MAIN_REPEAT_10(X) X(11) 
  #define SPP_MAIN_REPEAT_12(X) SPP_MAIN_REPEAT_11(X) X(12) 
  #define SPP_MAIN_REPEAT_13(X) SPP_MAIN_REPEAT_12(X) X(13) 
  #define SPP_MAIN_REPEAT_14(X) SPP_MAIN_REPEAT_13(X) X(14) 
  #define SPP_MAIN_REPEAT_15(X) SPP_MAIN_REPEAT_14(X) X(15) 
  #define SPP_MAIN_REPEAT_16(X) SPP_MAIN_REPEAT_15(X) X(16) 
  #define SPP_MAIN_REPEAT_17(X) SPP_MAIN_REPEAT_16(X) X(17) 
  #define SPP_MAIN_REPEAT_18(X) SPP_MAIN_REPEAT_17(X) X(18) 
  #define SPP_MAIN_REPEAT_19(X) SPP_MAIN_REPEAT_18(X) X(19) 
  #define SPP_MAIN_REPEAT_20(X) SPP_MAIN_REPEAT_19(X) X(20) 
  #define SPP_MAIN_REPEAT_21(X) SPP_MAIN_REPEAT_20(X) X(21) 
  #define SPP_MAIN_REPEAT_22(X) SPP_MAIN_REPEAT_21(X) X(22) 
  #define SPP_MAIN_REPEAT_23(X) SPP_MAIN_REPEAT_22(X) X(23) 
  #define SPP_MAIN_REPEAT_24(X) SPP_MAIN_REPEAT_23(X) X(24) 
  #define SPP_MAIN_REPEAT_25(X) SPP_MAIN_REPEAT_24(X) X(25) 
  #define SPP_MAIN_REPEAT_26(X) SPP_MAIN_REPEAT_25(X) X(26) 
  #define SPP_MAIN_REPEAT_27(X) SPP_MAIN_REPEAT_26(X) X(27) 
  #define SPP_MAIN_REPEAT_28(X) SPP_MAIN_REPEAT_27(X) X(28) 
  #define SPP_MAIN_REPEAT_29(X) SPP_MAIN_REPEAT_28(X) X(29) 
  #define SPP_MAIN_REPEAT_30(X) SPP_MAIN_REPEAT_29(X) X(30) 
  #define SPP_MAIN_REPEAT_31(X) SPP_MAIN_REPEAT_30(X) X(31) 
  #define SPP_MAIN_REPEAT_32(X) SPP_MAIN_REPEAT_31(X) X(32) 
  #define SPP_MAIN_REPEAT_MAX(X) SPP_MAIN_REPEAT_10(X)  /* Change this up to 32 */

  #define SPP_IF_0(X) 
  #define SPP_IF_1(X) X
  #define SPP_IF_2(X) X
  #define SPP_IF_3(X) X
  #define SPP_IF_4(X) X
  #define SPP_IF_5(X) X
  #define SPP_IF_6(X) X
  #define SPP_IF_7(X) X
  #define SPP_IF_8(X) X
  #define SPP_IF_9(X) X
  #define SPP_IF_10(X) X
  #define SPP_IF_11(X) X
  #define SPP_IF_12(X) X
  #define SPP_IF_13(X) X
  #define SPP_IF_14(X) X
  #define SPP_IF_15(X) X
  #define SPP_IF_16(X) X
  #define SPP_IF_17(X) X
  #define SPP_IF_18(X) X
  #define SPP_IF_19(X) X
  #define SPP_IF_20(X) X
  #define SPP_IF_21(X) X
  #define SPP_IF_22(X) X
  #define SPP_IF_23(X) X
  #define SPP_IF_24(X) X
  #define SPP_IF_25(X) X
  #define SPP_IF_26(X) X
  #define SPP_IF_27(X) X
  #define SPP_IF_28(X) X
  #define SPP_IF_29(X) X
  #define SPP_IF_30(X) X
  #define SPP_IF_31(X) X
  #define SPP_IF_32(X) X
  #define SPP_IF_MAX(X) SPP_IF_10(X) /* Change this up to 32 */

#endif
