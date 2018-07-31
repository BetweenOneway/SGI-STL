G++ 2.91.57，cygnus\cygwin-b20\include\g++\stl_vector.h 完整列表
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
 * Copyright (c) 1996
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
 
#ifndef __SGI_STL_INTERNAL_VECTOR_H
#define __SGI_STL_INTERNAL_VECTOR_H
 
__STL_BEGIN_NAMESPACE 
 
#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#endif
 
template <class T, class Alloc = alloc>  // alloc为默认空间配置器 
class vector {
public:
  // 以下表示 (1),(2),(3),(4),(5)，代表 iterator_traits<I> 所服务的5个型别。
  typedef T value_type;				// (1)
  typedef value_type* pointer; 			// (2)
  typedef const value_type* const_pointer;
  typedef const value_type* const_iterator;
  typedef value_type& reference; 		// (3)
  typedef const value_type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type; 	// (4)
  // 以下，由于vector 所维护的是一个连续性空间，所以不论其元素型別为何，
  // 原生指针都可以作为其迭代器而满足所有需求。
  typedef value_type* iterator;//定义迭代器的是原生态指针 
  /* 
   根据上述说法，如果客端编码如下：
   vector<Shape>::iterator is;
      is 的类型其实就是Shape*
      而STL 內部使用 iterator_traits<is>::reference 时，获得 Shape&
                 使用iterator_traits<is>::iterator_category 时，获得 
                      random_access_iterator_tag		(5)
      （此乃iterator_traits 针对原生态指针特化的结果）
  */
 
#ifdef __STL_CLASS_PARTIAL_SPECIALIZATION
  typedef reverse_iterator<const_iterator> const_reverse_iterator;
  typedef reverse_iterator<iterator> reverse_iterator;
#else /* __STL_CLASS_PARTIAL_SPECIALIZATION */
  typedef reverse_iterator<const_iterator, value_type, const_reference, 
                           difference_type>  const_reverse_iterator;
  typedef reverse_iterator<iterator, value_type, reference, difference_type>
          reverse_iterator;
#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */
protected:
  // 空间配置器。在stl_alloc.h中定义 
  typedef simple_alloc<value_type, Alloc> data_allocator;
 
  /*
  vector采用线性连续空间存储元素
  迭代器start指向使用空间的头 
  迭代器end指向使用空间的尾（使用空间不是全部的空间）
  end_of_storate 可用空间的尾 
  */
  iterator start;
  iterator finish;
  iterator end_of_storage;
 
  void insert_aux(iterator position, const T& x);
  void deallocate() {
    if (start)
    //在stl_alloc.h中定义 ,根据元素类型来判断是否调用析构函数 
         data_allocator::deallocate(start, end_of_storage - start);
  }
 
  void fill_initialize(size_type n, const T& value) {
    start = allocate_and_fill(n, value);  // 分配空间且设定初始值 
    finish = start + n;				// 调整尾迭代器 
    end_of_storage = finish; 			// 调整迭代器。分配空间和使用空间相同 
  }
//对外接口，我们可以使用的 
public:
  iterator begin() { return start; }//返回迭代器，指向起始位置 
  const_iterator begin() const { return start; }
  iterator end() { return finish; }//指向可用空间尾端的迭代器 
  const_iterator end() const { return finish; }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const { 
    return const_reverse_iterator(end()); 
  }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const { 
    return const_reverse_iterator(begin()); 
  }
  //已用用空间对象的个数 
  size_type size() const { return size_type(end() - begin()); }
  //最大可以状态对象个数。 size_type(-1)转换为无符号数 
  size_type max_size() const { return size_type(-1) / sizeof(T); }
  //vector当前容量 
  size_type capacity() const { return size_type(end_of_storage - begin()); }
  //判断是否为空 
  bool empty() const { return begin() == end(); }
  //注意，重载[]是返回引用，所以可以作为左值来给容器内对象复制 
  reference operator[](size_type n) { return *(begin() + n); }
  const_reference operator[](size_type n) const { return *(begin() + n); }
 
  //默认构造函数竟然不分配空间，感觉有点怪 
  vector() : start(0), finish(0), end_of_storage(0) {} 
  //以下几个构造函数（都指定大小和设置初始值都是调用 fill_initialize，在上
  //面已经看到fill_initialize并不多分配空间，可见如果指定vector大小的话
  //就不会再多分配空间了。 
  vector(size_type n, const T& value) { fill_initialize(n, value); }
  vector(int n, const T& value) { fill_initialize(n, value); }
  vector(long n, const T& value) { fill_initialize(n, value); }
  //T()说明需要容器内对象要有默认构造函数 
  explicit vector(size_type n) { fill_initialize(n, T()); }
//用一个容器初始化新建的容器。新建的容器大小只是x容器使用空间的大小 
  vector(const vector<T, Alloc>& x) {
    start = allocate_and_copy(x.end() - x.begin(), x.begin(), x.end());
    finish = start + (x.end() - x.begin());
    end_of_storage = finish;
  }
  //使用两个迭代器区间的值来初始化vector 
#ifdef __STL_MEMBER_TEMPLATES
  template <class InputIterator>
  vector(InputIterator first, InputIterator last) :
    start(0), finish(0), end_of_storage(0)
  {
 	//把start、finish、end_of_storage初始化为0
	 //使用push_back来添加 
    range_initialize(first, last, iterator_category(first));
  }
#else /* __STL_MEMBER_TEMPLATES */
  vector(const_iterator first, const_iterator last) {
    size_type n = 0;
    distance(first, last, n);//计算两个迭代器之间的距离 
    start = allocate_and_copy(n, first, last);//分配空间初始化，并不多分配空间 
    finish = start + n;
    end_of_storage = finish;
  }
#endif /* __STL_MEMBER_TEMPLATES */
  ~vector() { 
    //析构对象，在stl_construct.h定义 
    destroy(start, finish);  
    deallocate();   // 释放空间 
  }
  vector<T, Alloc>& operator=(const vector<T, Alloc>& x);
  //调整可用空间大小至n 
  void reserve(size_type n) {
    if (capacity() < n) {//如果小于n则调整，大于n直接返回 
      const size_type old_size = size();
      iterator tmp = allocate_and_copy(n, start, finish);
      destroy(start, finish);
      deallocate();
      start = tmp;
      finish = tmp + old_size;
      end_of_storage = start + n;
    }
  }
 
  // 取出vector第一个元素，是返回引用 
  reference front() { return *begin(); }
  const_reference front() const { return *begin(); }
  // 取出vector最后一个元素，是返回引用 
  reference back() { return *(end() - 1); }
  const_reference back() const { return *(end() - 1); }
  // 在容器尾添加元素 
  void push_back(const T& x) {
    if (finish != end_of_storage) {  // 如果还有未用空间 
      construct(finish, x);   		// 直接初始化未用空间。
      ++finish;                        
    }
    else                                  // 无未用空间 
      insert_aux(end(), x);			
  }
  //交换两个vector，只是交换了迭代器，并没有重新分配内存，所以原来的迭代器不会失效 
  void swap(vector<T, Alloc>& x) {
    __STD::swap(start, x.start);
    __STD::swap(finish, x.finish);
    __STD::swap(end_of_storage, x.end_of_storage);
  }
  //在position处插入元素x 
  iterator insert(iterator position, const T& x) {
    size_type n = position - begin();
    //如果插入位置正好是末尾（好像一般没这么巧吧？） 
    if (finish != end_of_storage && position == end()) {
      construct(finish, x);		// 直接初始化尾端内存。
      ++finish;
    }
    else
    /*
    insert_aux的实现是把position后的元素都后移，然后插入。
	当然了在后移之前要看一下有没有未用空间 
	*/
      insert_aux(position, x);
    return begin() + n;
  }
  //直接在position处插入vector对象默认值（条用默认构造函数） 
  iterator insert(iterator position) { return insert(position, T()); }
#ifdef __STL_MEMBER_TEMPLATES
/*
在position处插入一段元素。
其实还是调用insert一个一个插入的，再插入过程中，移动插入位置position 
*/
  template <class InputIterator>
  void insert(iterator position, InputIterator first, InputIterator last){
    range_insert(position, first, last, iterator_category(first));
  }
#else /* __STL_MEMBER_TEMPLATES */
  void insert(iterator position,
              const_iterator first, const_iterator last);
#endif /* __STL_MEMBER_TEMPLATES */
//在position处插入n个元素x 
  void insert (iterator pos, size_type n, const T& x);
  void insert (iterator pos, int n, const T& x) {
    insert(pos, (size_type) n, x);
  }
  void insert (iterator pos, long n, const T& x) {
    insert(pos, (size_type) n, x);
  }
//把容器末端元素去掉。并不返回末端元素 
  void pop_back() {
    --finish;
    destroy(finish);	
  }
  // 将position处元素消除 
  iterator erase(iterator position) {
  	/*
	如果position不是末端，那么要把后面元素往前移	  
  */
    if (position + 1 != end()) 
      copy(position + 1, finish, position); 
 
    --finish;  
    destroy(finish);
    return position;
  }
  //移除两个迭代器之间的元素 
  iterator erase(iterator first, iterator last) {
  	//把last后的元素向前移 
    iterator i = copy(last, finish, first);
    destroy(i, finish);	// 析构移动后多余的元素 
    finish = finish - (last - first);
    return first;
  }
  //调整已用空间大小 
  void resize(size_type new_size, const T& x) {
    if (new_size < size()) 
    //如果把已用空间调小，那么直接擦除掉多余部分 
      erase(begin() + new_size, end());
    else
    //已用空间变大，多出来的已用空间用x初始化 
      insert(end(), new_size - size(), x);
  }
   
  void resize(size_type new_size) { resize(new_size, T()); }
  // 清除全部元素。
  void clear() { erase(begin(), end()); }
 
protected:
/*
allocate_and_fill  //功能：分配空间并初始化 
      ?
  allocate  //分配空间  <stl_alloc.h> 
	  ?
uninitialized_fill_n //初始化 <stl_uninitialized.h>
*/ 
  iterator allocate_and_fill(size_type n, const T& x) {
    iterator result = data_allocator::allocate(n); // 配置空间 
    __STL_TRY {
      // 全域函式，記憶體低階工具，將result所指之未初始化空間設定初值為 x，n個
      //全局函数，初始化已配置但未初始化空间 
      // 在 <stl_uninitialized.h>。中定义 
      uninitialized_fill_n(result, n, x);  
      return result;
    }
     // "commit or rollback" 语义：如果有一个失败，则全部释放。
    __STL_UNWIND(data_allocator::deallocate(result, n));
  }
  
/*
allocate_and_copy  //功能：分配空间并初始化 
      ?
  allocate  //分配空间  <stl_alloc.h> 
	  ?
uninitialized_copy //初始化 <stl_uninitialized.h>
*/
#ifdef __STL_MEMBER_TEMPLATES
  template <class ForwardIterator>
  iterator allocate_and_copy(size_type n,
                             ForwardIterator first, ForwardIterator last) {
    iterator result = data_allocator::allocate(n);//配置空间 
    __STL_TRY {
      uninitialized_copy(first, last, result);//初始化配置空间 
      return result;
    }
    __STL_UNWIND(data_allocator::deallocate(result, n));
  }
#else /* __STL_MEMBER_TEMPLATES */
  iterator allocate_and_copy(size_type n,
                             const_iterator first, const_iterator last) {
    iterator result = data_allocator::allocate(n);
    __STL_TRY {
      uninitialized_copy(first, last, result);
      return result;
    }
    __STL_UNWIND(data_allocator::deallocate(result, n));
  }
#endif /* __STL_MEMBER_TEMPLATES */
 
/*
两个迭代器之间的元素初始化vector。根据迭代器类型来选择使用哪种初始化方式
如果是 input_iterator_tag类型迭代器，则一个一个初始化
如果是 forward_iterator_tag（包含其派生类型？），则调用 allocate_and_copy
*/
#ifdef __STL_MEMBER_TEMPLATES
  template <class InputIterator>
  void range_initialize(InputIterator first, InputIterator last,
                        input_iterator_tag) {
    for ( ; first != last; ++first)
      push_back(*first);
  }
 
  // This function is only called by the constructor.  We have to worry
  //  about resource leaks, but not about maintaining invariants.
  template <class ForwardIterator>
  void range_initialize(ForwardIterator first, ForwardIterator last,
                        forward_iterator_tag) {
    size_type n = 0;
    distance(first, last, n);
    start = allocate_and_copy(n, first, last);
    finish = start + n;
    end_of_storage = finish;
  }
 
  template <class InputIterator>
  void range_insert(iterator pos,
                    InputIterator first, InputIterator last,
                    input_iterator_tag);
 
  template <class ForwardIterator>
  void range_insert(iterator pos,
                    ForwardIterator first, ForwardIterator last,
                    forward_iterator_tag);
 
#endif /* __STL_MEMBER_TEMPLATES */
};
//判断两个容器是否相等。两个容器相同位置的元素相等才相同 
//equal应该是泛型算法 
template <class T, class Alloc>
inline bool operator==(const vector<T, Alloc>& x, const vector<T, Alloc>& y) {
  return x.size() == y.size() && equal(x.begin(), x.end(), y.begin());
}
// lexicographical_compare泛型算法 
template <class T, class Alloc>
inline bool operator<(const vector<T, Alloc>& x, const vector<T, Alloc>& y) {
  return lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
}
#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <class T, class Alloc>
inline void swap(vector<T, Alloc>& x, vector<T, Alloc>& y) {
  x.swap(y);
}
#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */
//重载=号运算符 
template <class T, class Alloc>
vector<T, Alloc>& vector<T, Alloc>::operator=(const vector<T, Alloc>& x) {
  if (&x != this) {	// 判断是否相同，防止自身赋值 
    if (x.size() > capacity()) {		// 赋值对象x内容大于vector的capacity
										//重新开辟空间 
      iterator tmp = allocate_and_copy(x.end() - x.begin(),
                                       x.begin(), x.end());
      destroy(start, finish);	
      deallocate();		
      start = tmp;				
      end_of_storage = start + (x.end() - x.begin());
    }
    else if (size() >= x.size()) {	// vector空间够用 
      iterator i = copy(x.begin(), x.end(), begin());
      destroy(i, finish);
    }
    else {
      copy(x.begin(), x.begin() + size(), start);
      uninitialized_copy(x.begin() + size(), x.end(), finish);
    }
    finish = start + x.size();
  }
  return *this;
}
//在指定位置插入元素 
template <class T, class Alloc>
void vector<T, Alloc>::insert_aux(iterator position, const T& x) {
  if (finish != end_of_storage) {  // 还有可用空间 
    // 后移最后一个元素    
    construct(finish, *(finish - 1));
    
    ++finish;
    // x_copy好像没啥用 
    T x_copy = x;
    //后移那些元素 
    copy_backward(position, finish - 2, finish - 1);
    *position = x_copy;
  }
  else {		// 无可用空间情况 
    const size_type old_size = size();
    const size_type len = old_size != 0 ? 2 * old_size : 1;
    
    /*
	重新开辟空间。如果原有空间为0，则重新开辟空间为1
	否则新开辟空间为原有空间的2倍 。 
	*/
 
    iterator new_start = data_allocator::allocate(len); // 配置空间 
    iterator new_finish = new_start;
    __STL_TRY {
      // 原有vector内容拷贝到新开辟的空间 
      new_finish = uninitialized_copy(start, position, new_start);
      // 为新元素设定初值 
      construct(new_finish, x);
 
      ++new_finish;
      //将旧vector未用空间的内容页拷贝过来。好像没啥用处啊？ 
      new_finish = uninitialized_copy(position, finish, new_finish);
    }
 
#       ifdef  __STL_USE_EXCEPTIONS 
    catch(...) {
      // "commit or rollback" 若失败则全部回滚。
      destroy(new_start, new_finish); 
      data_allocator::deallocate(new_start, len);
      throw;
    }
#       endif /* __STL_USE_EXCEPTIONS */
 
    // 析构和释放 vector
    destroy(begin(), end());
    deallocate();
 
    // 调整迭代器 
    start = new_start;
    finish = new_finish;
    end_of_storage = new_start + len;
  }
}
 
//在position处插入n个元素，初值为x 
template <class T, class Alloc>
void vector<T, Alloc>::insert(iterator position, size_type n, const T& x) {
  if (n != 0) { // n=0的话无意义
  //可用空间够用 
    if (size_type(end_of_storage - finish) >= n) { 
      // 定义x_copy=x;看起来好像没啥用，是不是多线程或者担心引用会修改原来的值？ 
      T x_copy = x;
      // position处到尾端共有多少个元素 
      const size_type elems_after = finish - position;	
      iterator old_finish = finish;
      /*
	  要判断 position处到尾端元素个数是否大于新插入元素个数n
	  因为在finish之前的内存是已经初始化的，finishing之后的内存是为构建的 
	  */
      if (elems_after > n) { //position处到尾端元素个数大于新插入元素个数n 
        
        uninitialized_copy(finish - n, finish, finish);
        finish += n;	// 將vector 尾端標記後移
        copy_backward(position, old_finish - n, old_finish);
        fill(position, position + n, x_copy);	// 從安插點開始填入新值
      }
      else {	//position处到尾端元素个数小于新插入元素个数n
        
        uninitialized_fill_n(finish, n - elems_after, x_copy);
        finish += n - elems_after;
        uninitialized_copy(position, old_finish, finish);
        finish += elems_after;
        fill(position, old_finish, x_copy);
      }
    }
    else {
      
      /*
	  空间不够用，则开辟新空间。新空间大小不是简单的乘以2，因为乘以2也
	  未必容得下新加入的元素 
	  */
      const size_type old_size = size();        
      const size_type len = old_size + max(old_size, n);
  
      iterator new_start = data_allocator::allocate(len);
      iterator new_finish = new_start;
      __STL_TRY {
        
        new_finish = uninitialized_copy(start, position, new_start);
       
        new_finish = uninitialized_fill_n(new_finish, n, x);
        
        new_finish = uninitialized_copy(position, finish, new_finish);
      }
#         ifdef  __STL_USE_EXCEPTIONS 
      catch(...) {
       
        destroy(new_start, new_finish);
        data_allocator::deallocate(new_start, len);
        throw;
      }
#         endif /* __STL_USE_EXCEPTIONS */
     
      destroy(start, finish);
      deallocate();
      
      start = new_start;
      finish = new_finish;
      end_of_storage = new_start + len;
    }
  }
}
//在position处插入两个迭代器之间的元素。迭代器不同，实现方法不同，上面已经讲过。 
#ifdef __STL_MEMBER_TEMPLATES
template <class T, class Alloc> template <class InputIterator>
void vector<T, Alloc>::range_insert(iterator pos,
                                    InputIterator first, InputIterator last,
                                    input_iterator_tag) {
  for ( ; first != last; ++first) {
    pos = insert(pos, *first);
    ++pos;
  }
}
 
template <class T, class Alloc> template <class ForwardIterator>
void vector<T, Alloc>::range_insert(iterator position,
                                    ForwardIterator first,
                                    ForwardIterator last,
                                    forward_iterator_tag) {
  if (first != last) {
    size_type n = 0;
    distance(first, last, n);
    if (size_type(end_of_storage - finish) >= n) {
      const size_type elems_after = finish - position;
      iterator old_finish = finish;
      if (elems_after > n) {
        uninitialized_copy(finish - n, finish, finish);
        finish += n;
        copy_backward(position, old_finish - n, old_finish);
        copy(first, last, position);
      }
      else {
        ForwardIterator mid = first;
        advance(mid, elems_after);
        uninitialized_copy(mid, last, finish);
        finish += n - elems_after;
        uninitialized_copy(position, old_finish, finish);
        finish += elems_after;
        copy(first, mid, position);
      }
    }
    else {
      const size_type old_size = size();
      const size_type len = old_size + max(old_size, n);
      iterator new_start = data_allocator::allocate(len);
      iterator new_finish = new_start;
      __STL_TRY {
        new_finish = uninitialized_copy(start, position, new_start);
        new_finish = uninitialized_copy(first, last, new_finish);
        new_finish = uninitialized_copy(position, finish, new_finish);
      }
#         ifdef __STL_USE_EXCEPTIONS
      catch(...) {
        destroy(new_start, new_finish);
        data_allocator::deallocate(new_start, len);
        throw;
      }
#         endif /* __STL_USE_EXCEPTIONS */
      destroy(start, finish);
      deallocate();
      start = new_start;
      finish = new_finish;
      end_of_storage = new_start + len;
    }
  }
}
 
#else /* __STL_MEMBER_TEMPLATES */
//在position处插入两个迭代器之间的元素，和在position处插入n个元素类似 
template <class T, class Alloc>
void vector<T, Alloc>::insert(iterator position, 
                              const_iterator first, 
                              const_iterator last) {
  if (first != last) {
    size_type n = 0;
    distance(first, last, n);
    if (size_type(end_of_storage - finish) >= n) {
      const size_type elems_after = finish - position;
      iterator old_finish = finish;
      if (elems_after > n) {
        uninitialized_copy(finish - n, finish, finish);
        finish += n;
        copy_backward(position, old_finish - n, old_finish);
        copy(first, last, position);
      }
      else {
        uninitialized_copy(first + elems_after, last, finish);
        finish += n - elems_after;
        uninitialized_copy(position, old_finish, finish);
        finish += elems_after;
        copy(first, first + elems_after, position);
      }
    }
    else {
      const size_type old_size = size();
      const size_type len = old_size + max(old_size, n);
      iterator new_start = data_allocator::allocate(len);
      iterator new_finish = new_start;
      __STL_TRY {
        new_finish = uninitialized_copy(start, position, new_start);
        new_finish = uninitialized_copy(first, last, new_finish);
        new_finish = uninitialized_copy(position, finish, new_finish);
      }
#         ifdef __STL_USE_EXCEPTIONS
      catch(...) {
        destroy(new_start, new_finish);
        data_allocator::deallocate(new_start, len);
        throw;
      }
#         endif /* __STL_USE_EXCEPTIONS */
      destroy(start, finish);
      deallocate();
      start = new_start;
      finish = new_finish;
      end_of_storage = new_start + len;
    }
  }
}
 
#endif /* __STL_MEMBER_TEMPLATES */
 
#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#endif
 
__STL_END_NAMESPACE 
 
#endif /* __SGI_STL_INTERNAL_VECTOR_H */
 
// Local Variables:
// mode:C++
// End:
 
