G++ 2.91.57，cygnus\cygwin-b20\include\g++\stl_heap.h 完整列表
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
 * Copyright (c) 1997
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
 
#ifndef __SGI_STL_INTERNAL_HEAP_H
#define __SGI_STL_INTERNAL_HEAP_H
 
__STL_BEGIN_NAMESPACE
 
#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1209
#endif
 
 
//向上维护
template <class RandomAccessIterator, class Distance, class T>
void __push_heap(RandomAccessIterator first, Distance holeIndex,
                 Distance topIndex, T value) {
  Distance parent = (holeIndex - 1) / 2;	// 找出父结点
  //父结点不是heap顶 且 父节点的值小于孩子结点（小于号<可以看出STL维护的是max heap）
  while (holeIndex > topIndex && *(first + parent) < value) {
    *(first + holeIndex) = *(first + parent);	// 父结点下调
    holeIndex = parent; // 维护父结点
    parent = (holeIndex - 1) / 2;	
  }    
  //已达到heap顶或者满足heap性质后，给新插入元素找到合适位置
  *(first + holeIndex) = value;	
}
 
template <class RandomAccessIterator, class Distance, class T>
inline void __push_heap_aux(RandomAccessIterator first,
                            RandomAccessIterator last, Distance*, T*) {
  __push_heap(first, Distance((last - first) - 1), Distance(0), 
              T(*(last - 1)));
  
}
 
//往heap添加元素。注意：此函数调用时，元素已经添加到末尾了，且迭代器已经加1了
template <class RandomAccessIterator>
inline void push_heap(RandomAccessIterator first, RandomAccessIterator last) {
  // 注意，此函式被呼叫時，新元素應已置於底層容器的最尾端。
  __push_heap_aux(first, last, distance_type(first), value_type(first));
}
 
//向上维护，允许用自己定义的comp函数，这是未必是max heap了
template <class RandomAccessIterator, class Distance, class T, class Compare>
void __push_heap(RandomAccessIterator first, Distance holeIndex,
                 Distance topIndex, T value, Compare comp) {
  Distance parent = (holeIndex - 1) / 2;//找到父结点的位置
  while (holeIndex > topIndex && comp(*(first + parent), value)) {
    *(first + holeIndex) = *(first + parent);
    holeIndex = parent;
    parent = (holeIndex - 1) / 2;
  }
  *(first + holeIndex) = value;
}
 
template <class RandomAccessIterator, class Compare, class Distance, class T>
inline void __push_heap_aux(RandomAccessIterator first,
                            RandomAccessIterator last, Compare comp,
                            Distance*, T*) {
  __push_heap(first, Distance((last - first) - 1), Distance(0), 
              T(*(last - 1)), comp);
}
 
template <class RandomAccessIterator, class Compare>
inline void push_heap(RandomAccessIterator first, RandomAccessIterator last,
                      Compare comp) {
  __push_heap_aux(first, last, comp, distance_type(first), value_type(first));
}
 
// 以下這個 __adjust_heap() 不允許指定「大小比較標準」
//向下维护heap
template <class RandomAccessIterator, class Distance, class T>
void __adjust_heap(RandomAccessIterator first, Distance holeIndex,
                   Distance len, T value) {
  Distance topIndex = holeIndex;
  Distance secondChild = 2 * holeIndex + 2;	// 找到右孩子的位置
  while (secondChild < len) {//还没到达末尾
    // 比较左右孩子大小， secondChild 代表其中较大者
    if (*(first + secondChild) < *(first + (secondChild - 1)))
      secondChild--;   
    // 较大的孩子放到父结点位置
    *(first + holeIndex) = *(first + secondChild);  
    holeIndex = secondChild;
    // 维护较大的孩子
    secondChild = 2 * (secondChild + 1);
  }
  // 没有右孩子，只有左孩子，只需维护一次，因为左孩子是heap的最后一个元素
  if (secondChild == len) { 
    *(first + holeIndex) = *(first + (secondChild - 1));
    holeIndex = secondChild - 1;
  }
 
  //把调整值放到合适位置，等价于*(first + holeIndex) = value;
  __push_heap(first, holeIndex, topIndex, value);
}
 
//向下维护heap
template <class RandomAccessIterator, class T, class Distance>
inline void __pop_heap(RandomAccessIterator first, RandomAccessIterator last,
                       RandomAccessIterator result, T value, Distance*) {
  *result = *first; // 把堆顶的值放到堆尾，堆尾的值保存在参数value中
                      
  __adjust_heap(first, Distance(0), Distance(last - first), value);
 
}
 
template <class RandomAccessIterator, class T>
inline void __pop_heap_aux(RandomAccessIterator first,
                           RandomAccessIterator last, T*) {
  __pop_heap(first, last-1, last-1, T(*(last-1)), distance_type(first));
  /*
	根据implicit representation heap的次序性，pop后的结果是容器的第一个元素。
	把最后一个元素放到到容器头，因此维护时维护区间是[first last-1)。
  */
}
//取出heap顶元素，按照max heap属性来维护heap
template <class RandomAccessIterator>
inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last) {
  __pop_heap_aux(first, last, value_type(first));
}
 
//向下维护heap，允许执行比较函数comp
template <class RandomAccessIterator, class Distance, class T, class Compare>
void __adjust_heap(RandomAccessIterator first, Distance holeIndex,
                   Distance len, T value, Compare comp) {
  Distance topIndex = holeIndex;
  Distance secondChild = 2 * holeIndex + 2;
  while (secondChild < len) {
    if (comp(*(first + secondChild), *(first + (secondChild - 1))))
      secondChild--;
    *(first + holeIndex) = *(first + secondChild);
    holeIndex = secondChild;
    secondChild = 2 * (secondChild + 1);
  }
  if (secondChild == len) {
    *(first + holeIndex) = *(first + (secondChild - 1));
    holeIndex = secondChild - 1;
  }
  __push_heap(first, holeIndex, topIndex, value, comp);
}
 
// 取出堆顶元素，允许定义比较函数comp
template <class RandomAccessIterator, class T, class Compare, class Distance>
inline void __pop_heap(RandomAccessIterator first, RandomAccessIterator last,
                       RandomAccessIterator result, T value, Compare comp,
                       Distance*) {
  *result = *first;
  __adjust_heap(first, Distance(0), Distance(last - first), value, comp);
}
 
template <class RandomAccessIterator, class T, class Compare>
inline void __pop_heap_aux(RandomAccessIterator first,
                           RandomAccessIterator last, T*, Compare comp) {
  __pop_heap(first, last - 1, last - 1, T(*(last - 1)), comp,
             distance_type(first));
}
//取出heap顶元素，根据comp调整heap
template <class RandomAccessIterator, class Compare>
inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last,
                     Compare comp) {
    __pop_heap_aux(first, last, value_type(first), comp);
}
 
 
template <class RandomAccessIterator, class T, class Distance>
void __make_heap(RandomAccessIterator first, RandomAccessIterator last, T*,
                 Distance*) {
  if (last - first < 2) return;	
  Distance len = last - first;
 
  Distance parent = (len - 2)/2; 
    //从heap的中间开始向下维护。一次维护[parent, parent-1,……,0]
  while (true) {
    
    __adjust_heap(first, parent, len, T(*(first + parent)));
    if (parent == 0) return;
    parent--;				
  }
}
 
// 将 [first,last) 排列成一个 heap。
template <class RandomAccessIterator>
inline void make_heap(RandomAccessIterator first, RandomAccessIterator last) {
  __make_heap(first, last, value_type(first), distance_type(first));
}
 
 
template <class RandomAccessIterator, class Compare, class T, class Distance>
void __make_heap(RandomAccessIterator first, RandomAccessIterator last,
                 Compare comp, T*, Distance*) {
  if (last - first < 2) return;
  Distance len = last - first;
  //从heap的中间开始向下维护。一次维护[parent, parent-1,……,0]
  Distance parent = (len - 2)/2;
    
  while (true) {
    __adjust_heap(first, parent, len, T(*(first + parent)), comp);
    if (parent == 0) return;
    parent--;
  }
}
// 将 [first,last) 排列成一个 heap。允许执行大小比较函数
template <class RandomAccessIterator, class Compare>
inline void make_heap(RandomAccessIterator first, RandomAccessIterator last,
                      Compare comp) {
  __make_heap(first, last, comp, value_type(first), distance_type(first));
}
 
// 排序（升序）两个迭代器之间的元素，这两个迭代器之间的元素已经满足heap性质了
template <class RandomAccessIterator>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last) {
 
  /*
	pop_heap()功能是删除heap顶元素，把heap大小减小1，heap顶元素放到末尾。
	由此可以看出，sort的结果是升序。
  */
  while (last - first > 1)
     pop_heap(first, last--); // 每執行 pop_heap() 一次，操作範圍即退縮一格。
}
 
// 排序（升序）两个迭代器之间的元素，这两个迭代器之间的元素已经满足heap性质了。
//允许指定比较函数comp
template <class RandomAccessIterator, class Compare>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last,
               Compare comp) {
  while (last - first > 1)
     pop_heap(first, last--, comp);
}
 
#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1209
#endif
 
__STL_END_NAMESPACE
 
#endif /* __SGI_STL_INTERNAL_HEAP_H */
 
// Local Variables:
// mode:C++
// End:
