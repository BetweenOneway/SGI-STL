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
 
#ifndef __SGI_STL_INTERNAL_LIST_H
#define __SGI_STL_INTERNAL_LIST_H
 
__STL_BEGIN_NAMESPACE
 
#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
	#pragma set woff 1174
#endif
 
template <class T>struct __list_node 
{
	typedef void* void_pointer;
	void_pointer next; 
	void_pointer prev;
	T data;
};
 
 
//list是一个双向链表，其迭代器可以向前移、向后移
//因此迭代器类型为bidirectional_iterator_tag
template<class T, class Ref, class Ptr> struct __list_iterator 
{
	typedef __list_iterator<T, T&, T*>             iterator;
	typedef __list_iterator<T, const T&, const T*> const_iterator;
	typedef __list_iterator<T, Ref, Ptr>           self;
 
	typedef bidirectional_iterator_tag iterator_category;	 // (1)
	typedef T value_type; 			// (2)
	typedef Ptr pointer; 			// (3)
	typedef Ref reference; 			// (4)
	typedef __list_node<T>* link_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type; // (5)
 
	link_type node;
 
	//用于link_type到iterator的转换
	__list_iterator(link_type x) : node(x) {}
	__list_iterator() {}
	__list_iterator(const iterator& x) : node(x.node) {}
 
	// 迭代器需要重载的运算符，为了支持标准算法STL
	bool operator==(const self& x) const { return node == x.node; }
	bool operator!=(const self& x) const { return node != x.node; }
  
	//对迭代器dereference，取的是迭代器所维护的结点的值
	reference operator*() const 
	{ 
		return (*node).data; 
	}	

	//如果支持->操作 
	#ifndef __SGI_STL_NO_ARROW_OPERATOR 
		pointer operator->() const 
		{ 
			return &(operator*()); 
		}
	#endif /* __SGI_STL_NO_ARROW_OPERATOR */
 
	//迭代器前进、后退的支持
	self& operator++() 
	{ 
		node = (link_type)((*node).next);  	
		return *this;
	}
	self operator++(int) 
	{ 
		self tmp = *this;
		++*this;
		return tmp;
	}
  
	self& operator--() 
	{ 
		node = (link_type)((*node).prev); 
		return *this;
	}
	self operator--(int) 
	{ 
		self tmp = *this;
		--*this;
		return tmp;
	}
};
//如果编译器不支持partial specialization偏特性化
#ifndef __STL_CLASS_PARTIAL_SPECIALIZATION
	template <class T, class Ref, class Ptr>inline bidirectional_iterator_tag iterator_category(const __list_iterator<T, Ref, Ptr>&) 
	{
		return bidirectional_iterator_tag();
	}
 
	template <class T, class Ref, class Ptr>inline T* value_type(const __list_iterator<T, Ref, Ptr>&) 
	{
		return 0;
	}
 
	template <class T, class Ref, class Ptr>inline ptrdiff_t* distance_type(const __list_iterator<T, Ref, Ptr>&) 
	{
		return 0;
	}
#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */
 
template <class T, class Alloc = alloc> class list 
{
	protected:
		typedef void* void_pointer;
		typedef __list_node<T> list_node;
		typedef simple_alloc<list_node, Alloc> list_node_allocator;
		public:      
		typedef T value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef list_node* link_type;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
 
	public:
		typedef __list_iterator<T, T&, T*>             iterator;
		typedef __list_iterator<T, const T&, const T*> const_iterator;
 
		#ifdef __STL_CLASS_PARTIAL_SPECIALIZATION
			typedef reverse_iterator<const_iterator> const_reverse_iterator;
			typedef reverse_iterator<iterator> reverse_iterator;
		#else /* __STL_CLASS_PARTIAL_SPECIALIZATION */
			typedef reverse_bidirectional_iterator<const_iterator, value_type,const_reference, difference_type> const_reverse_iterator;
			typedef reverse_bidirectional_iterator<iterator, value_type, reference,difference_type> reverse_iterator; 
		#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */
 
	protected:
		// 配置一个结点（未初始化），返回其指针
		link_type get_node() 
		{ 
			return list_node_allocator::allocate(); 
		}
		// 释放一个结点
		void put_node(link_type p) 
		{ 
			list_node_allocator::deallocate(p);
		}
 
		// 配置一个结点，并用x初始化
		link_type create_node(const T& x) 
		{
			link_type p = get_node();
			__STL_TRY 
			{
				construct(&p->data, x);	// 全局函数
			}
			__STL_UNWIND(put_node(p));
			return p;
		}
		// 销毁一个结点
		void destroy_node(link_type p) 
		{
			destroy(&p->data); 		//全局函数
			put_node(p);
		}
 
	protected:
		void empty_initialize() 
		{ 
			node = get_node();	
			node->next = node;	
			node->prev = node;
		}
		//初始化长为n的链表，值都为value
		void fill_initialize(size_type n, const T& value) 
		{
			empty_initialize();
			__STL_TRY
			{
				insert(begin(), n, value);
			}
			__STL_UNWIND(clear(); put_node(node));
		}
 
		#ifdef __STL_MEMBER_TEMPLATES
			//以迭代器的区间初始化一个链表
			template <class InputIterator>
			void range_initialize(InputIterator first, InputIterator last)
			{
				empty_initialize();
				__STL_TRY 
				{
					insert(begin(), first, last);
				}
				//commit or rollback
				__STL_UNWIND(clear(); put_node(node));
			}
		#else  /* __STL_MEMBER_TEMPLATES */
			void range_initialize(const T* first, const T* last) 
			{
				empty_initialize();
				__STL_TRY 
				{
					insert(begin(), first, last);
				}
				__STL_UNWIND(clear(); put_node(node));
			}
			void range_initialize(const_iterator first, const_iterator last) 
			{
				empty_initialize();
				__STL_TRY
				{
					insert(begin(), first, last);
				}
				__STL_UNWIND(clear(); put_node(node));
			}
		#endif /* __STL_MEMBER_TEMPLATES */
 
	protected:
		link_type node; // 可以认为它是哨兵结点（在算法导论中有讲哨兵结点）
 
	public:
		list() 
		{ 
			empty_initialize(); 
		}  
		//指向头结点的迭代器
		iterator begin() 
		{ 
			return (link_type)((*node).next); 
		}
		const_iterator begin() const 
		{ 
			return (link_type)((*node).next); 
		}
 
		//指向尾结点下一个位置的迭代器，所以返回node
		iterator end() 
		{ 
			return node; 
		}	
		const_iterator end() const 
		{ 
			return node; 
		}
		reverse_iterator rbegin() 
		{ 
			return reverse_iterator(end()); 
		}
		const_reverse_iterator rbegin() const 
		{ 
			return const_reverse_iterator(end()); 
		}
		reverse_iterator rend() 
		{ 
			return reverse_iterator(begin()); 
		}
		const_reverse_iterator rend() const 
		{ 
			return const_reverse_iterator(begin());
		} 
		//链表只有node结点时为空链表
		bool empty() const
		{ 
			return node->next == node;
		}
		size_type size() const 
		{
			size_type result = 0;
			distance(begin(), end(), result);  // 在<stl_iterator.h>定义，result是引用传递
			return result;
		}
		//链表最大容量。没什么意义吧？
		size_type max_size() const 
		{ 
			return size_type(-1); 
		}
		// 取链表头结点的内容
		reference front() 
		{
			return *begin(); 
		}  
		const_reference front() const 
		{ 
			return *begin(); 
		}
		// 取链表尾结点的内容
		reference back() 
		{ 
			return *(--end()); 
		} 
		const_reference back() const 
		{
			return *(--end()); 
		}
		//交换两个链表
		void swap(list<T, Alloc>& x) 
		{ 
			__STD::swap(node, x.node); 
		}
 
		//这个插入是插在position之前，如果position是n，那么新节点是n-1
		iterator insert(iterator position, const T& x) 
		{
			link_type tmp = create_node(x); 
			//设置新节点的指针
			tmp->next = position.node;
			tmp->prev = position.node->prev;
			//交出原来的位置
			(link_type(position.node->prev))->next = tmp;
			position.node->prev = tmp;
			return tmp;
		}
		//在迭代器 position 所指位置前插入一个结点，其值为T的默认值，这也说明List的元素要有默认构造函数
		iterator insert(iterator position) 
		{ 
			return insert(position, T()); 
		}
 
		//在position所指位置前插入多个元素
		#ifdef __STL_MEMBER_TEMPLATES
			template <class InputIterator>void insert(iterator position, InputIterator first, InputIterator last);
		#else /* __STL_MEMBER_TEMPLATES */
			void insert(iterator position, const T* first, const T* last);
			void insert(iterator position,const_iterator first, const_iterator last);
		#endif /* __STL_MEMBER_TEMPLATES */
		void insert(iterator pos, size_type n, const T& x);
		void insert(iterator pos, int n, const T& x) 
		{
			insert(pos, (size_type)n, x);
		}
		void insert(iterator pos, long n, const T& x)
		{
			insert(pos, (size_type)n, x);
		}
 
		// 在头结点前插入元素
		void push_front(const T& x) 
		{ 
			insert(begin(), x); 
		}
		// 在尾结点后插入元素
		void push_back(const T& x) 
		{ 
			insert(end(), x); 
		}
 
		// 移除迭代器 position 所指结点
		iterator erase(iterator position) 
		{
			link_type next_node = link_type(position.node->next);
			link_type prev_node = link_type(position.node->prev);
			prev_node->next = next_node;
			next_node->prev = prev_node;
			destroy_node(position.node);
			return iterator(next_node);
		}
		iterator erase(iterator first, iterator last);
		void resize(size_type new_size, const T& x);
		void resize(size_type new_size) 
		{ 
			resize(new_size, T()); 
		}
		void clear();
 
		// 移除头结点
		void pop_front() 
		{
			erase(begin()); 
		}
		// 移除尾结点
		void pop_back() 
		{ 
			iterator tmp = end();
			erase(--tmp);
		}
		//几个构造函数
		list(size_type n, const T& value) 
		{ 
			fill_initialize(n, value); 
		}
		list(int n, const T& value) 
		{ 
			fill_initialize(n, value); 
		}
		list(long n, const T& value) 
		{ 
			fill_initialize(n, value); 
		}
		explicit list(size_type n) 
		{ 
			fill_initialize(n, T()); 
		}
 
		//用迭代器区间初始化List
		#ifdef __STL_MEMBER_TEMPLATES
			template <class InputIterator> list(InputIterator first, InputIterator last) 
			{
				range_initialize(first, last);
			}
		#else /* __STL_MEMBER_TEMPLATES */
			list(const T* first, const T* last) 
			{ 
				range_initialize(first, last); 
			}
			list(const_iterator first, const_iterator last)
			{
				range_initialize(first, last);
			}
		#endif /* __STL_MEMBER_TEMPLATES */
		//用一个List初始化
		list(const list<T, Alloc>& x) 
		{
			range_initialize(x.begin(), x.end());
		}
		~list() 
		{
			clear();//清除所有结点，哨兵结点除外
			put_node(node);//释放唯一的一个结点
		}
		list<T, Alloc>& operator=(const list<T, Alloc>& x);
 
	protected:
		// 将[first,last) 內的所有元素搬移到position 前，不包括last元素。
		void transfer(iterator position, iterator first, iterator last)
		{
			if (position != last) 
			{
				(*(link_type((*last.node).prev))).next = position.node;	
				(*(link_type((*first.node).prev))).next = last.node;
				(*(link_type((*position.node).prev))).next = first.node; 
				link_type tmp = link_type((*position.node).prev);
				(*position.node).prev = (*last.node).prev;
				(*last.node).prev = (*first.node).prev; 
				(*first.node).prev = tmp;
			}
		}
 
	public:
		// 將 x 链表插入到 position 所指位置之前。x 不是 *this。
		void splice(iterator position, list& x) 
		{
			if (!x.empty())
			{
				transfer(position, x.begin(), x.end());
			}
		}
		// 將 i 所指元素插入到 position 所指位置之前。position 和i 可在同一个list。
		void splice(iterator position, list&, iterator i) 
		{
			iterator j = i;
			++j;
			if (position == i || position == j)
			{
				return;
			}
			transfer(position, i, j);
		}
		// 將 [first,last) 內的所有元素插入到 position 所指位置之前。
		// position 和[first,last)可指在同一个list，
		// 但position不能位于[first,last)之內。
		void splice(iterator position, list&, iterator first, iterator last) 
		{
			if (first != last)
			{
				transfer(position, first, last);
			}
		}
		void remove(const T& value);
		void unique();
		void merge(list& x);
		void reverse();
		void sort();
 
		#ifdef __STL_MEMBER_TEMPLATES
			template <class Predicate> void remove_if(Predicate);
			template <class BinaryPredicate> void unique(BinaryPredicate);
			template <class StrictWeakOrdering> void merge(list&, StrictWeakOrdering);
			template <class StrictWeakOrdering> void sort(StrictWeakOrdering);
		#endif /* __STL_MEMBER_TEMPLATES */
 
		friend bool operator== __STL_NULL_TMPL_ARGS (const list& x, const list& y);
};
 
//判断2个链表是否相同
template <class T, class Alloc> inline bool operator==(const list<T,Alloc>& x, const list<T,Alloc>& y)
{
	typedef typename list<T,Alloc>::link_type link_type;
	link_type e1 = x.node;
	link_type e2 = y.node;
	link_type n1 = (link_type) e1->next;
	link_type n2 = (link_type) e2->next;
	for (; n1 != e1 && n2 != e2; n1 = (link_type)n1->next, n2 = (link_type)n2->next)
	{
		if (n1->data != n2->data)
		{
			return false;
		}
	}

	return n1 == e1 && n2 == e2;
}
 
//lexicographical_compare是STL算法
template <class T, class Alloc>inline bool operator<(const list<T, Alloc>& x, const list<T, Alloc>& y) 
{
	return lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
}
#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <class T, class Alloc>
	//交换两个链表
	inline void swap(list<T, Alloc>& x, list<T, Alloc>& y) 
	{
		x.swap(y);
	}
#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */
#ifdef __STL_MEMBER_TEMPLATES
	//在position之前插入迭代器区间的元素
	template <class T, class Alloc> template <class InputIterator>
	void list<T, Alloc>::insert(iterator position,InputIterator first, InputIterator last) 
	{
		for ( ; first != last; ++first)
		{
			insert(position, *first);
		}
	}
#else /* __STL_MEMBER_TEMPLATES */
	template <class T, class Alloc>void list<T, Alloc>::insert(iterator position, const T* first, const T* last)
	{
		for (; first != last; ++first)
		{
			insert(position, *first);
		}
	}
	template <class T, class Alloc> void list<T, Alloc>::insert(iterator position,const_iterator first, const_iterator last)
	{
		for (; first != last; ++first)
		{
			insert(position, *first);
		}
	}
#endif /* __STL_MEMBER_TEMPLATES */
//在position位置之前插入n个元素x
template <class T, class Alloc>void list<T, Alloc>::insert(iterator position, size_type n, const T& x) 
{
	for ( ; n > 0; --n)
	{
		insert(position, x);
	} 
}
//擦除两个迭代器区间之间的元素
template <class T, class Alloc>list<T,Alloc>::iterator list<T, Alloc>::erase(iterator first, iterator last) 
{
	while (first != last) 
	{ 
		erase(first++); 
	}
	return last;
}
/*
重新调整链表大小为 new_size
如果new_size大于原来的链表，则在链表末尾插入x
如果new_size小于原来的链表，则在末尾直接擦除多余的元素
*/
template <class T, class Alloc> void list<T, Alloc>::resize(size_type new_size, const T& x)
{
  iterator i = begin();
  size_type len = 0;
  for ( ; i != end() && len < new_size; ++i, ++len)
    ;
  if (len == new_size)
    erase(i, end());
  else                          // i == end()
    insert(end(), new_size - len, x);
}
 
// 清除所有结点，（哨兵结点除外）
template <class T, class Alloc> void list<T, Alloc>::clear()
{
	link_type cur = (link_type) node->next; // begin()
	while (cur != node) 
	{	
		link_type tmp = cur;
		cur = (link_type) cur->next;
		destroy_node(tmp); 	
	}
	// 恢复哨兵结点，链表此时为空链表
	node->next = node;
	node->prev = node;
}
//重载赋值=操作符
template <class T, class Alloc>list<T, Alloc>& list<T, Alloc>::operator=(const list<T, Alloc>& x) 
{
	if (this != &x) 
	{
		//防止自身赋值
		iterator first1 = begin();
		iterator last1 = end();
		const_iterator first2 = x.begin();
		const_iterator last2 = x.end();
		//通过更改结点的值来赋值
		while (first1 != last1 && first2 != last2)
		{
			*first1++ = *first2++;
		}
		/*
		如果x链表小于this链表，擦除多余的，否则在this后面插入
		*/
		if (first2 == last2)
		{
			erase(first1, last1);
		}
		else
		{
			insert(last1, first2, last2);
		}
	}
	return *this;
}
 
// 将数值为value的结点移除
template <class T, class Alloc> void list<T, Alloc>::remove(const T& value)
{
	iterator first = begin();
	iterator last = end();
	while (first != last) 
	{
		iterator next = first;
		++next;
		if (*first == value) erase(first); 	// 找到就移除
		{
			first = next;
		}
	}
}
 
// 移除数值相同的连续元素
template <class T, class Alloc> void list<T, Alloc>::unique() 
{
	iterator first = begin();
	iterator last = end();
	if (first == last) return;
	iterator next = first;
	while (++next != last) 
	{
		if (*first == *next)//如果数值相同，则移除后面的那个
		{
			erase(next);
		}
		else
		{
			first = next;
		}
		next = first;
	}
}
 
//将x合并到*this上面。两个链表都要先经过递增排序。相当于合并排序的最后一步
template <class T, class Alloc>void list<T, Alloc>::merge(list<T, Alloc>& x)
{
	iterator first1 = begin();
	iterator last1 = end();
	iterator first2 = x.begin();
	iterator last2 = x.end();
 
	//注意：此时已经假设两个链表都已经非递减排序好了
	while (first1 != last1 && first2 != last2)
	{
		if (*first2 < *first1)
		{
			iterator next = first2;
			transfer(first1, first2, ++next);
			first2 = next;
		}
		else
		{
			++first1;
		}
	}
	if (first2 != last2)
	{
		transfer(last1, first2, last2);
	}
}
 
// 将 *this 的內容逆向重置
template <class T, class Alloc>void list<T, Alloc>::reverse() 
{
	//如果链表是空，或者只有一个元素，就不做任何处理
	//不是用size()==0或size()==1来判断，因为这样比较慢
	if (node->next == node || link_type(node->next)->next == node)
	{
		return;
	}
	iterator first = begin();
	++first;
	while (first != end()) 
	{
		iterator old = first;
		++first;
		transfer(begin(), old, first);
	}
}    
 
/*
STL的sort算法只能接受迭代器类型为RamdonAccessIterator的容器，所以list无法使用
这个是什么算法？？？
*/
template <class T, class Alloc>void list<T, Alloc>::sort() 
{
	if (node->next == node || link_type(node->next)->next == node)
	{
		return;
	}
	list<T, Alloc> carry;
	list<T, Alloc> counter[64];
	int fill = 0;
	while (!empty()) 
	{
		carry.splice(carry.begin(), *this, begin());
		int i = 0;
		while(i < fill && !counter[i].empty())
		{
			counter[i].merge(carry);
			carry.swap(counter[i++]);
		}
		carry.swap(counter[i]);
		if (i == fill)
		{
			++fill;
		}
	} 
	for (int i = 1; i < fill; ++i)
	{
		counter[i].merge(counter[i - 1]);
	}
	swap(counter[fill-1]);
}
 
#ifdef __STL_MEMBER_TEMPLATES
/*
pred是一个函数，如果容器内的元素经过pred函数判断为真，则移除
*/
template <class T, class Alloc> template <class Predicate> void list<T, Alloc>::remove_if(Predicate pred) 
{
	iterator first = begin();
	iterator last = end();
	while (first != last) 
	{
		iterator next = first;
		++next;
		if (pred(*first)) 
		{ 
			erase(first); 
		}
		first = next;
	}
}
/*
根据函数binary_pred来判断是否移除两个相邻的结点
*/
template <class T, class Alloc> template <class BinaryPredicate>void list<T, Alloc>::unique(BinaryPredicate binary_pred) 
{
	iterator first = begin();
	iterator last = end();
	if (first == last)
	{
		return;
	}
	iterator next = first;
	while (++next != last) 
	{
		if (binary_pred(*first, *next))
		{
			erase(next);
		}
		else
		{
			first = next;
		}
		next = first;
	}
}
/*
假设两个链表均已经有序，用comp函数来判断如何合并两个链表
*/
template <class T, class Alloc> template <class StrictWeakOrdering> void list<T, Alloc>::merge(list<T, Alloc>& x, StrictWeakOrdering comp)
{
	iterator first1 = begin();
	iterator last1 = end();
	iterator first2 = x.begin();
	iterator last2 = x.end();
	while (first1 != last1 && first2 != last2)
	{
		if (comp(*first2, *first1))
		{
			iterator next = first2;
			transfer(first1, first2, ++next);
			first2 = next;
		}
		else
		{
			++first1;
		}
	}
	if (first2 != last2) 
	{ 
		transfer(last1, first2, last2); 
	}
}
/*
用函数comp来判断如何排序链表
*/
template <class T, class Alloc> template <class StrictWeakOrdering> void list<T, Alloc>::sort(StrictWeakOrdering comp) 
{
	if (node->next == node || link_type(node->next)->next == node) 
	{ 
		return; 
	}
	list<T, Alloc> carry;
	list<T, Alloc> counter[64];
	int fill = 0;
	while (!empty()) 
	{
		carry.splice(carry.begin(), *this, begin());
		int i = 0;
		while(i < fill && !counter[i].empty()) 
		{
			counter[i].merge(carry, comp);
			carry.swap(counter[i++]);
		}
		carry.swap(counter[i]);         
		if (i == fill) 
		{ 
			++fill; 
		}
	} 
 
	for (int i = 1; i < fill; ++i) 
	{ 
		counter[i].merge(counter[i - 1], comp); 
	}
	swap(counter[fill-1]);
}
 
#endif /* __STL_MEMBER_TEMPLATES */
 
#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#endif
 
__STL_END_NAMESPACE 
 
#endif /* __SGI_STL_INTERNAL_LIST_H */
 
// Local Variables:
// mode:C++
// End:
