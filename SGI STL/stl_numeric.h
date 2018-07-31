G++ 2.91.57，cygnus\cygwin-b20\include\g++\stl_numeric.h 完整列表
/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */
 
/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */
 
 
#ifndef __SGI_STL_INTERNAL_NUMERIC_H
#define __SGI_STL_INTERNAL_NUMERIC_H
 
__STL_BEGIN_NAMESPACE
 
//计算init和[first last)区间元素的和
// 版本一
template <class InputIterator, class T>
T accumulate(InputIterator first, InputIterator last, T init) {
  for ( ; first != last; ++first)
    init = init + *first; 	// 每个元素的初值累加到init上
  return init;
}
 
// 版本二
template <class InputIterator, class T, class BinaryOperation>
T accumulate(InputIterator first, InputIterator last, T init,
                BinaryOperation binary_op) {
  for ( ; first != last; ++first)
    init = binary_op(init, *first);	//  对每一个元素执行二元操作
  return init;
}
 
//计算两段元素的内积，要提供初始值init。两段元素一样长，所以第二段没提供last2
// 版本一
template <class InputIterator1, class InputIterator2, class T>
T inner_product(InputIterator1 first1, InputIterator1 last1,
                   InputIterator2 first2, T init) {
  for ( ; first1 != last1; ++first1, ++first2)
    init = init + (*first1 * *first2); // 执行两个序列的一般內積
  return init;
}
 
// 版本二
template <class InputIterator1, class InputIterator2, class T,
          class BinaryOperation1, class BinaryOperation2>
T inner_product(InputIterator1 first1, InputIterator1 last1,
                   InputIterator2 first2, T init, BinaryOperation1 binary_op1,
                   BinaryOperation2 binary_op2) {
  for ( ; first1 != last1; ++first1, ++first2)
	// 以外界提供的仿函数来取代第一版本中的 operator* 和 operator+。
	// op2 作用域两元素间，op1用于op2之结果与init之间。
    init = binary_op1(init, binary_op2(*first1, *first2));
  return init;
}
 
//__partial_sum定义在partial_sum前面，后者调用前者
template <class InputIterator, class OutputIterator, class T>
OutputIterator __partial_sum(InputIterator first, InputIterator last,
                                   OutputIterator result, T*) {
  T value = *first;
  while (++first != last) {
    value = value + *first;	 	// 前n个元素的总和
    *++result = value;			// 指定给目的端
  }
  return ++result;
}
 
// 版本一
template <class InputIterator, class OutputIterator>
OutputIterator partial_sum(InputIterator first, InputIterator last,
                                OutputIterator result) {
  if (first == last) return result;
  *result = *first;
  return __partial_sum(first, last, result, value_type(first));
 
  // 侯捷认为（并证实证），不需像上行那样转呼叫，可改用以下写法（整个函式）：
  // if (first == last) return result;
  // *result = *first;
  // iterator_traits<InputIterator>::value_type value = *first;
  // while (++first != last) {
  //   value = value + *first;
  //   *++result = value;
  // }
  // return ++result;
  // 这样的观念和作法，适用于本当所有函数
}
 
template <class InputIterator, class OutputIterator, class T,
          class BinaryOperation>
OutputIterator __partial_sum(InputIterator first, InputIterator last,
                                  OutputIterator result, T*,
                                  BinaryOperation binary_op) {
  T value = *first;
  while (++first != last) {
    value = binary_op(value, *first);
    *++result = value;
  }
  return ++result;
}
 
// 版本二
template <class InputIterator, class OutputIterator, class BinaryOperation>
OutputIterator partial_sum(InputIterator first, InputIterator last,
                                OutputIterator result, BinaryOperation binary_op) {
  if (first == last) return result;
  *result = *first;
  return __partial_sum(first, last, result, value_type(first), binary_op);
 
  //  侯捷认为（并证实证），不需像上行那样转呼叫，可改用以下写法（整个函式）：
  // if (first == last) return result;
  // *result = *first;
  // iterator_trait<InputIterator>::value_type value = *first;
  // while (++first != last) {
  //   value = binary_op(value, *first);
  //   *++result = value;
  // }
  // return ++result;
  //
  //  这样的观念和作法，适用于本当所有函数
}
 
template <class InputIterator, class OutputIterator, class T>
OutputIterator __adjacent_difference(InputIterator first, InputIterator last, 
                                            OutputIterator result, T*) {
  T value = *first;
  while (++first != last) {		// 走过整个范围
    T tmp = *first;
    *++result = tmp - value;	// 将两个相邻元素的差额（后-前），指派给目的端
    value = tmp;
  }
  return ++result;
}
 
// 版本一
template <class InputIterator, class OutputIterator>
OutputIterator adjacent_difference(InputIterator first, InputIterator last, 
                                          OutputIterator result) {
  if (first == last) return result;
  *result = *first;	// 首先记录第一个元素
  return __adjacent_difference(first, last, result, value_type(first));
}
 
template <class InputIterator, class OutputIterator, class T, 
          class BinaryOperation>
OutputIterator __adjacent_difference(InputIterator first, InputIterator last, 
                                            OutputIterator result, T*,
                                            BinaryOperation binary_op) {
  T value = *first;
  while (++first != last) {
    T tmp = *first;
    *++result = binary_op(tmp, value); // 将相邻两元素的运算結果，指派给目的端
    value = tmp;
  }
  return ++result;
}
 
// 版本二
template <class InputIterator, class OutputIterator, class BinaryOperation>
OutputIterator adjacent_difference(InputIterator first, InputIterator last,
                                          OutputIterator result,
                                          BinaryOperation binary_op) {
  if (first == last) return result;
  *result = *first;	
  return __adjacent_difference(first, last, result, value_type(first),
                                     binary_op);
}
 
//power为SGI专属，并不在STL标准之列，它用来计算某数的n幂次方。
//这里所谓的n幂次方是指对自己进行某种运算n次，运算类型可由外界执行。如果是乘法则就是乘幂。
 
// 版本二，冪次方。如果指定为乘法运算，则当n >= 0 时传回 x ** n。
// 注意，"multiplication" 必须满足结合律（associative），
//  但不需满足交换律（commutative）。
template <class T, class Integer, class MonoidOperation>
T power(T x, Integer n, MonoidOperation op) {
  if (n == 0)
    return identity_element(op);		// 取出「证同元素」identity element.证同元素在后面章节介绍
  else {
    while ((n & 1) == 0) {
      n >>= 1;
      x = op(x, x);//两个参数都是x。因为每次n减小1倍(n是偶数，因为n&1=0)
    }
 
    T result = x;
    n >>= 1;
    while (n != 0) {
      x = op(x, x);
      if ((n & 1) != 0)
        result = op(result, x);
      n >>= 1;
    }
    return result;
  }
}
 
// 版本一，乘冪。
template <class T, class Integer>
inline T power(T x, Integer n) {
  return power(x, n, multiplies<T>());
}
 
// 侯捷：iota 是什麼的缩写？
// 函数意义：在 [first,last) 范围内內填入value, value+1, value+2...。
template <class ForwardIterator, class T>
void iota(ForwardIterator first, ForwardIterator last, T value) {
  while (first != last) *first++ = value++;
}
 
__STL_END_NAMESPACE
 
#endif /* __SGI_STL_INTERNAL_NUMERIC_H */
 
// Local Variables:
// mode:C++
// End:
