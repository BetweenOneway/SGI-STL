G++ 2.91.57，cygnus\cygwin-b20\include\g++\stl_construct.h 完整列表
 
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
 
#ifndef __SGI_STL_INTERNAL_CONSTRUCT_H
#define __SGI_STL_INTERNAL_CONSTRUCT_H
 
#include <new.h>
 
__STL_BEGIN_NAMESPACE
 
template <class T>
inline void destroy(T* pointer) {
    pointer->~T();//显示调用析构函数
}
 
template <class T1, class T2>
inline void construct(T1* p, const T2& value) {
  new (p) T1(value);  // jjhou: placement new; invoke ctor T1(value);
}
/*
在调用析构函数前，有必要解释下trivial destructor和non-trivial destructor。
如果用户不定义析构函数，那么编译器会自动合成析构函数，但是这个析构函数基本没什么用，所以称为
trivial destructor无意义的析构函数。
如果用户定义了析构函数，说明在释放对象所占用内存之前要做一些事情，就成为non-trivial destructor。
在C++中，如果只有基本数据类型，那么就不用调用析构函数，就认为析构函数是trivial destructor。
如果有像指针这样的数据类型，那么久有必要调用析构函数了，就认为析构函数是non-trivial destructor。
在STL中，调用析构函数时，都会用一个has_trivial_destructor来判断对象是否含有trivial destructor，如果是，
那么不会调用析构函数，什么也不做
void __destroy_aux(ForwardIterator, ForwardIterator, __true_type)是空函数。
反之，则调用析构函数，释放对象。
__destroy_aux(ForwardIterator first, ForwardIterator last, __false_type)调用析构函数
结合下面的图来看代码：
								       特化 
								       ____>__destroy_aux(.,_false_type)
								       |
								       |NO _false_type 进行析构
								       |	
					__泛化_>_destroy()--has non-trival destructor?
					|			       |
					|			       |YES _true_type  不进行析构
					|			       |
					|			       |_特化_>_destroy_aux(,_true_type)
destroy()--
					|
					|
					|
					|------特化------>不进行析构
					|  (char* ,char*)
					|
					|------特化------>不进行析构
					|  (wchar* ,wchar*)
					|
					|
					|------特化------>pointer->~T()
						(T* pointer)
*/
 
template <class ForwardIterator>
inline void
__destroy_aux(ForwardIterator first, ForwardIterator last, __false_type) {
  for ( ; first < last; ++first)
    destroy(&*first);//non-trivial destructor，调用析构函数
}
 
template <class ForwardIterator> //trivial destructor，空函数
inline void __destroy_aux(ForwardIterator, ForwardIterator, __true_type) {}
 
template <class ForwardIterator, class T>
inline void __destroy(ForwardIterator first, ForwardIterator last, T*) {
  typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
  __destroy_aux(first, last, trivial_destructor());//判断是否是trivial destructor
}
 
template <class ForwardIterator>
inline void destroy(ForwardIterator first, ForwardIterator last) {
  __destroy(first, last, value_type(first));
}
 
inline void destroy(char*, char*) {}
inline void destroy(wchar_t*, wchar_t*) {}
 
__STL_END_NAMESPACE
 
#endif /* __SGI_STL_INTERNAL_CONSTRUCT_H */
 
// Local Variables:
// mode:C++
// End:
