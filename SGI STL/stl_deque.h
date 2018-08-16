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
 
#ifndef __SGI_STL_INTERNAL_DEQUE_H
#define __SGI_STL_INTERNAL_DEQUE_H
 
/* Class的恒长特性（invariants）：
 对于任何nonsingular iterator I（非退化的迭代器I）
 i.node是 map array 中的某个元素的地址。
i.node 所指内容是一个指针，指向某个缓冲区的起始位置。
i.first=*(i.node)
i.last=i.first+node_size(即buffer_size())
i.cur是一个指针，指向[i.first i.last)之间。注意：
这意味着i.cur永远是一个dereferenceable pointer,
纵使i是一个 past-the-end iterator.
Start 和 Finish总是 nonsingular iterator(非退化迭代器)。注意：这意味着
empty deque 一定会有一个node, 而一个具有N个元素的deque（N表示缓冲区大小），
一定会有两个nodes。
对于start.node 和 finish.node 以外的每一个node， 其中每一个元素都是一个经过初始化的。
如果start.node==finish.nod，那么[start.cur finish.cur)都是经过初始化，而范围以外的元素
都是未初始化的空间。否则，[start.cur start.last)和[finish.first finish.cur)是经过初始化的，而
[start.first start.cur)和[finish.cur finish.last)是未初始化的空间。
[map map+map_size)是一个有效的、non-empty的范围。
[start.node finish.node]是一个幼小的范围，包含在[map map+map_size)之内。
范围[map map+map_size)内的任何一个指针会指向一个经过配置的node,当且仅当
该指针在范围[start.node finish.node]之内。
 */
 
 
__STL_BEGIN_NAMESPACE 
 
#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
	#pragma set woff 1174
#endif
 
 
/*
此函数用来计算缓冲区的大小
 如果n不等于0，那么返回n，开发者自己决定
    否则：如果sz小于512，返回512/sz
		  如果sz大于512，返回1
*/
/*
sz是存储的内容大小
一个缓冲区是512字节，这里计算的是一个缓冲区能存储几个元素
*/
inline size_t __deque_buf_size(size_t n, size_t sz)
{
	return n != 0 ? n : (sz < 512 ? size_t(512 / sz) : size_t(1));
}
 
 
//deque的迭代器，它没有继承std::iterator
#ifndef __STL_NON_TYPE_TMPL_PARAM_BUG
template <class T, class Ref, class Ptr, size_t BufSiz>struct __deque_iterator 
{ 	
	typedef __deque_iterator<T, T&, T*, BufSiz>      iterator;
	typedef __deque_iterator<T, const T&, const T*, BufSiz> const_iterator;
	static size_t buffer_size() 
	{
		return __deque_buf_size(BufSiz, sizeof(T)); 
	}
#else /* __STL_NON_TYPE_TMPL_PARAM_BUG */
template <class T, class Ref, class Ptr>struct __deque_iterator 
{ 	
	typedef __deque_iterator<T, T&, T*>             iterator;
	typedef __deque_iterator<T, const T&, const T*> const_iterator;
	static size_t buffer_size() 
	{
		return __deque_buf_size(0, sizeof(T)); 
	}
#endif
 
	//没有继承std::iterator，自己定义5个迭代器相应的类型。
  
	//其占用内存连续（部分连续）迭代器类型是random_access_iterator_tag
	typedef random_access_iterator_tag iterator_category; // (1)
	typedef T value_type; 				// (2)
	typedef Ptr pointer; 				// (3)
	typedef Ref reference; 				// (4)
	typedef size_t size_type;
	typedef ptrdiff_t difference_type; 	// (5)
	typedef T** map_pointer;	//注意，是指针的指针
	//map_pointer指向中控器，中控器的存储的是指针，指向node-buf结点缓冲区
 
	typedef __deque_iterator self;
	/*
	关于下面4个元素的意义以及和map中控器、缓冲区buffer的关系，见图（2）
	*/
 
	T* cur;	// 迭代器所指元素
	T* first;	// 迭代器所指元素所在缓冲区的开头
	T* last;	// 迭代器所指元素所在缓冲区的结尾（结尾包含在缓冲区内）
	map_pointer node;//指向中控器的结点，这个结点指向迭代器所指元素所在的缓冲区
  
	//迭代器的构造函数
 
	//x是迭代器所指结点，y为中控器中的结点的值，指向x所指缓冲区
	__deque_iterator(T* x, map_pointer y) : cur(x), first(*y), last(*y + buffer_size()), node(y) {}
	//默认构造函数
	__deque_iterator() : cur(0), first(0), last(0), node(0) {}
	//用一个迭代器x初始化本迭代器
	__deque_iterator(const iterator& x) : cur(x.cur), first(x.first), last(x.last), node(x.node) {}
 
	//迭代器需要重载的运算符
	reference operator*() const 
	{ 
		return *cur; 
	}
	#ifndef __SGI_STL_NO_ARROW_OPERATOR
		//重载箭头是返回地址
		pointer operator->() const 
		{ 
			return &(operator*()); 
		}
	#endif /* __SGI_STL_NO_ARROW_OPERATOR */
	/*
	两个迭代器之间的距离。这两个迭代器可能不在同一个buffer上。
	*/
	difference_type operator-(const self& x) const 
	{
		return difference_type(buffer_size()) * (node - x.node - 1) +(cur - first) + (x.last - x.cur);
	}
  
	// 參考 More Effective C++, item6: Distinguish between prefix and
	// postfix forms of increment and decrement operators.
	/*
	迭代器前进一步。
	先++cur，再判断cur==last。说明cur不会指向last的。last所指空间不存内容
	*/
	self& operator++() 
	{
		++cur;				// 前进一步
		if (cur == last) 
		{		// 到了所在缓冲区的尾端了
			set_node(node + 1);	// 切换到下一个缓冲区
			cur = first;			//   的第一个元素
		}
		return *this; 
	}
	self operator++(int) 
	{
		self tmp = *this;
		++*this;
		return tmp;
	}
	//迭代器往回走一步。
	self& operator--() 
	{
		if (cur == first) 
		{	// 如果在所在缓冲区的头部
			set_node(node - 1);	// 切换到前一个缓冲区
			cur = last;			//   的最后一个元素
		}
		--cur;				// 直接往回走一步
		return *this;
	}
	self operator--(int) 
	{
		self tmp = *this;
		--*this;
		return tmp;
	}
 
	/*
	迭代器向前进或后退n步（取决于n的正负）。这是支持random access iterator 所必须的操作。
	如果这个操作不会是迭代器走出当前所在缓冲区，直接更改cur即可。
	如果这个操作使迭代器走出当前所在缓冲区，要计算出操作后在哪个缓冲区的哪个位置。
	*/
	self& operator+=(difference_type n) 
	{
		difference_type offset = n + (cur - first);
		if (offset >= 0 && offset < difference_type(buffer_size()))
		{
			// 不会走出当前所在缓冲区
			cur += n;
		}
		else 
		{
			// 走出了当前所在缓冲区
			difference_type node_offset =
			offset > 0 ? offset / difference_type(buffer_size())
			: -difference_type((-offset - 1) / buffer_size()) - 1;
			// 切换缓冲区
			set_node(node + node_offset);
			// 找到切换缓冲区后，迭代器所指向的元素
			cur = first + (offset - node_offset * difference_type(buffer_size()));
		}
		return *this;
	}
	self operator+(difference_type n) const 
	{
		self tmp = *this;
		return tmp += n; // 调用operator+=
	}
	//调用operator+=
	self& operator-=(difference_type n) 
	{
		return *this += -n; 
	} 
	self operator-(difference_type n) const
	{
		self tmp = *this;
		return tmp -= n; // 调用operator-=
	}
 
	reference operator[](difference_type n) const 
	{ 
		return *(*this + n); 
	}
	// 以上调用了operator*, operator+
 
	/*迭代器关于比较的运算符的重载*/
	bool operator==(const self& x) const 
	{ 
		return cur == x.cur; 
	}
	bool operator!=(const self& x) const 
	{ 
		return !(*this == x); 
	}
	bool operator<(const self& x) const 
	{
		return (node == x.node) ? (cur < x.cur) : (node < x.node);
	}
 
	void set_node(map_pointer new_node) 
	{
		node = new_node;
		first = *new_node;
		//将last移动到缓冲区末尾
		last = first + difference_type(buffer_size());
	}
};
 
//编译器不支持片特性话partial specialization
#ifndef __STL_CLASS_PARTIAL_SPECIALIZATION
 
	#ifndef __STL_NON_TYPE_TMPL_PARAM_BUG
 
		template <class T, class Ref, class Ptr, size_t BufSiz> 
		inline random_access_iterator_tag iterator_category(const __deque_iterator<T, Ref, Ptr, BufSiz>&) 
		{
			return random_access_iterator_tag();
		}
 
		template <class T, class Ref, class Ptr, size_t BufSiz>
		inline T* value_type(const __deque_iterator<T, Ref, Ptr, BufSiz>&) 
		{
			return 0;
		}
 
		template <class T, class Ref, class Ptr, size_t BufSiz>
		inline ptrdiff_t* distance_type(const __deque_iterator<T, Ref, Ptr, BufSiz>&) 
		{
			return 0;
		}
 
	#else /* __STL_NON_TYPE_TMPL_PARAM_BUG */
 
		template <class T, class Ref, class Ptr>
		inline random_access_iterator_tag iterator_category(const __deque_iterator<T, Ref, Ptr>&) 
		{
			return random_access_iterator_tag();
		}
 
		template <class T, class Ref, class Ptr>
		inline T* value_type(const __deque_iterator<T, Ref, Ptr>&) 
		{ 
			return 0; 
		}
 
		template <class T, class Ref, class Ptr>
		inline ptrdiff_t* distance_type(const __deque_iterator<T, Ref, Ptr>&) 
		{
			return 0;
		}
 
	#endif /* __STL_NON_TYPE_TMPL_PARAM_BUG */
 
#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */
 
/*
deque的定义，默认使用alloc配置器
*/
template <class T, class Alloc = alloc, size_t BufSiz = 0> class deque 
{
	public:                         // Basic types
		typedef T value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
 
	public:                         // 迭代器
		#ifndef __STL_NON_TYPE_TMPL_PARAM_BUG
			typedef __deque_iterator<T, T&, T*, BufSiz>              iterator;
			typedef __deque_iterator<T, const T&, const T&, BufSiz>  const_iterator;
		#else /* __STL_NON_TYPE_TMPL_PARAM_BUG */
			typedef __deque_iterator<T, T&, T*>                      iterator;
			typedef __deque_iterator<T, const T&, const T*>          const_iterator;
		#endif /* __STL_NON_TYPE_TMPL_PARAM_BUG */
 
		#ifdef __STL_CLASS_PARTIAL_SPECIALIZATION
			typedef reverse_iterator<const_iterator> const_reverse_iterator;
			typedef reverse_iterator<iterator> reverse_iterator;
		#else /* __STL_CLASS_PARTIAL_SPECIALIZATION */
			typedef reverse_iterator<const_iterator, value_type, const_reference, difference_type> const_reverse_iterator;
			typedef reverse_iterator<iterator, value_type, reference, difference_type> reverse_iterator; 
		#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */
 
	protected:                      // Internal typedefs
		// 指向中控器，是指针的指针（pointer of pointer of T）
		typedef pointer* map_pointer;	
		// 空间配置器，用来配置缓冲区
		typedef simple_alloc<value_type, Alloc> data_allocator;
		// 空间配置器，用来配置中控器
		typedef simple_alloc<pointer, Alloc> map_allocator;
 
		static size_type buffer_size() 
		{
			return __deque_buf_size(BufSiz, sizeof(value_type));
		}
		//默认中控器大小为8
		static size_type initial_map_size()
		{ 
			return 8; 
		}
 
	protected:                      // Data members
		iterator start;		// start.cur指向deque的第一个结点
		iterator finish;	// finish.cur指向迭代器deque的最后一个结点的后一个元素
		map_pointer map;	// 指向中控器。其实是指向中控器的第一个结点。
		//	中控器是连续的，map_size定义了中控器的大小。          
		size_type map_size;	// 中控器的大小。
 
	public:                         // 对外的接口
		iterator begin() { return start; }
		iterator end() { return finish; }
		const_iterator begin() const { return start; }
		const_iterator end() const { return finish; }
		reverse_iterator rbegin() { return reverse_iterator(finish); }
		reverse_iterator rend() { return reverse_iterator(start); }
		const_reverse_iterator rbegin() const 
		{
			return const_reverse_iterator(finish);
		}
		const_reverse_iterator rend() const 
		{
			return const_reverse_iterator(start);
		}
 
		reference operator[](size_type n) 
		{
			return start[difference_type(n)]; // 调用 __deque_iterator<>::operator[]
		}
		const_reference operator[](size_type n) const 
		{
			return start[difference_type(n)];
		}
 
		reference front() { return *start; } // 调用 __deque_iterator<>::operator*
  
		//取出最后一个元素
		reference back() 
		{
			iterator tmp = finish;	
			--tmp;	// 调用 __deque_iterator<>::operator--
			return *tmp; 	// 调用 __deque_iterator<>::operator*
		}
  
		//返回第一个元素，并不删除
		const_reference front() const { return *start; }
		const_reference back() const 
		{
			const_iterator tmp = finish;
			--tmp;
			return *tmp;
		}
 
		//deque中元素个数。后面有两个分号。迭代器调用了iterator::operator-
		size_type size() const 
		{ 
			return finish - start;; 
		} 
		//deque最大容量。
		size_type max_size() const 
		{ 
			return size_type(-1); 
		}
		//下面调用了operator::iterator==
		bool empty() const 
		{ 
			return finish == start; 
		}
 
	public:    
		// Constructor, destructor.
   
		//默认构造函数
		// 以上 start() 和 finish() 调用 iterator（亦即 __deque_iterator）
		// 的 default constructor，令 cur, first, last, node 都为0。
		deque(): start(), finish(), map(0), map_size(0)
		{
			create_map_and_nodes(0);
		}
		//用一个deque构建新的deque
		deque(const deque& x): start(), finish(), map(0), map_size(0)
		{
			create_map_and_nodes(x.size());
			__STL_TRY 
			{
				uninitialized_copy(x.begin(), x.end(), start);
			}
			//commit or rollback
			__STL_UNWIND(destroy_map_and_nodes());
		}
		//构建大小为n，元素值为value的deque
		deque(size_type n, const value_type& value)	: start(), finish(), map(0), map_size(0)
		{
			fill_initialize(n, value);
		}
 
		deque(int n, const value_type& value) : start(), finish(), map(0), map_size(0)
		{
			fill_initialize(n, value);
		}
 
		deque(long n, const value_type& value) : start(), finish(), map(0), map_size(0)
		{
			fill_initialize(n, value);
		}
		//构建大小为n的deque，默认值为T(),说明deque容器的元素要有默认构造函数
		explicit deque(size_type n)	: start(), finish(), map(0), map_size(0)
		{
			fill_initialize(n, value_type());
		}
 
		/*用一段元素构建的确 */
		#ifdef __STL_MEMBER_TEMPLATES
			template <class InputIterator>	deque(InputIterator first, InputIterator last): start(), finish(), map(0), map_size(0)
			{
				range_initialize(first, last, iterator_category(first));
			}
		#else /* __STL_MEMBER_TEMPLATES */
			deque(const value_type* first, const value_type* last): start(), finish(), map(0), map_size(0)
			{
				create_map_and_nodes(last - first);
				__STL_TRY 
				{
					uninitialized_copy(first, last, start);
				}
				__STL_UNWIND(destroy_map_and_nodes());
			}
			deque(const_iterator first, const_iterator last): start(), finish(), map(0), map_size(0)
			{
				create_map_and_nodes(last - first);
				__STL_TRY 
				{
					uninitialized_copy(first, last, start);
				}
				__STL_UNWIND(destroy_map_and_nodes());
			}
		#endif /* __STL_MEMBER_TEMPLATES */
 
		~deque() 
		{
			destroy(start, finish);
			destroy_map_and_nodes();
		}
 
		deque& operator= (const deque& x) 
		{
			const size_type len = size();
			if (&x != this) 
			{
				if (len >= x.size())
				{
					erase(copy(x.begin(), x.end(), start), finish);
				}
				else
				{
					const_iterator mid = x.begin() + difference_type(len);
					copy(x.begin(), mid, start);
					insert(finish, mid, x.end());
				}
			}
			return *this;
		}        
 
		void swap(deque& x) 
		{
			__STD::swap(start, x.start);
			__STD::swap(finish, x.finish);
			__STD::swap(map, x.map);
			__STD::swap(map_size, x.map_size);
		}
 
	public:                         // push_* and pop_*
		//在deque末尾添加元素
		void push_back(const value_type& t) 
		{
			if (finish.cur != finish.last - 1) 
			{ 
				// 当前缓冲区还有空间
				construct(finish.cur, t);	// 直接在可用空间构建
				++finish.cur;	// 调整finish迭代器
			}
			else  // 当前缓冲区无可用空间（last不能存储元素用）
			{
				push_back_aux(t);
			}
		}
 
		//在deque头添加元素
		void push_front(const value_type& t) 
		{
			// 当前缓冲区还有空间 因为是在头里插入数据
			if (start.cur != start.first) 
			{ 	
				construct(start.cur - 1, t); 
				--start.cur;		
			}
			else 
			{
				// 当前缓冲区无空间可用了
				push_front_aux(t);
			}
		}
  
		//删掉末尾元素
		void pop_back()
		{
			if (finish.cur != finish.first) 
			{
				//最后一个缓冲区（finish指的缓冲区）有多于一个元素（含一个）
				--finish.cur;
				destroy(finish.cur);
			}
			else
			{
				// 最后一个缓冲区无元素
				pop_back_aux();		// 这里会进行缓冲区的释放工作
			}
		}
 
		//在deque头删除元素
		void pop_front() 
		{
			if (start.cur != start.last - 1) 
			{
				// start.node所指缓冲区有多余一个元素（不含一个）
				destroy(start.cur);	
				++start.cur;	
			}
			else
			{
				// start.node所指缓冲区只有一个元素
				pop_front_aux();		// 这里会进行缓冲区释放工作
			}
		}
 
	public:                         // Insert
		/*在position处插入一个元素
		如果position是deque的最前端，则调用push_front()。
		如果position是deque的最末端，则调用push_back()。
		在两个元素之间插入的话，就调用insert_aux。
		*/
		// 在position 处安插一個元素，其值为 x
		iterator insert(iterator position, const value_type& x) 
		{
			if (position.cur == start.cur) 
			{	
				push_front(x);	
				return start;
			}
			else if (position.cur == finish.cur) 
			{ 
				push_back(x);
				iterator tmp = finish;
				--tmp;
				return tmp;
			}
			else 
			{
				return insert_aux(position, x);	 	// 交给insert_aux 去做
			}
		}
		// 在position 处安插一個元素，其值为T()
		iterator insert(iterator position) { return insert(position, value_type()); }
 
		void insert(iterator pos, size_type n, const value_type& x); 
 
		void insert(iterator pos, int n, const value_type& x) 
		{
			insert(pos, (size_type) n, x);
		}
		void insert(iterator pos, long n, const value_type& x) 
		{
			insert(pos, (size_type) n, x);
		}
 
		#ifdef __STL_MEMBER_TEMPLATES  
			template <class InputIterator>
			void insert(iterator pos, InputIterator first, InputIterator last) 
			{
				insert(pos, first, last, iterator_category(first));
			}
 		#else /* __STL_MEMBER_TEMPLATES */
			void insert(iterator pos, const value_type* first, const value_type* last);
			void insert(iterator pos, const_iterator first, const_iterator last);
		#endif /* __STL_MEMBER_TEMPLATES */
	/*
	调整deque的大小。
	如果deque变小，直接擦除掉多余的元素。
	如果deque变大，则在deque后面插入元素补充，元素值为x/T()
	*/
	void resize(size_type new_size, const value_type& x)
	{
		const size_type len = size();
		if (new_size < len)
		{
			erase(start + new_size, finish);
		}
		else
		{
			insert(finish, new_size - len, x);
		}
	}
 
	void resize(size_type new_size) { resize(new_size, value_type()); }
 
	public:                         
		// 清除 pos 所指的元素。
		/*判断pos距离头近还是距离尾近，距离那个位置近就移动那个位置的元素，保证移动元素个数最少*/
		iterator erase(iterator pos) 
		{
			iterator next = pos;
			++next;
			difference_type index = pos - start;	// pos和deque开头元素的个数
			if (index < (size() >> 1)) 
			{			// size() >> 1为size()/2。
				//如果pos距离deque头比较近的话，deque的开头到pos元素向后移
				copy_backward(start, pos, next);	
				pop_front();				// 移动后，删除第一个元素
			}
			else 
			{					
				// 否则pos+1到结尾元素向前移，
				copy(next, finish, pos);	
				pop_back();				
			}
			return start + index;
		}
 
		iterator erase(iterator first, iterator last);
		void clear(); 
 
	protected:                        // Internal construction/destruction
 
		void create_map_and_nodes(size_type num_elements);
		void destroy_map_and_nodes();
		void fill_initialize(size_type n, const value_type& value);
 
		#ifdef __STL_MEMBER_TEMPLATES  
			template <class InputIterator>
			void range_initialize(InputIterator first, InputIterator last,input_iterator_tag);
 
			template <class ForwardIterator>
			void range_initialize(ForwardIterator first, ForwardIterator last,forward_iterator_tag);
		#endif /* __STL_MEMBER_TEMPLATES */
 
	protected:                        // Internal push_* and pop_*
		void push_back_aux(const value_type& t);
		void push_front_aux(const value_type& t);
		void pop_back_aux();
		void pop_front_aux();
 
	protected:                        // Internal insert functions
 
		#ifdef __STL_MEMBER_TEMPLATES  
			template <class InputIterator>
			void insert(iterator pos, InputIterator first, InputIterator last,input_iterator_tag);
 
			template <class ForwardIterator>
			void insert(iterator pos, ForwardIterator first, ForwardIterator last,forward_iterator_tag);
		#endif /* __STL_MEMBER_TEMPLATES */
 
		iterator insert_aux(iterator pos, const value_type& x);
		void insert_aux(iterator pos, size_type n, const value_type& x);
 
		#ifdef __STL_MEMBER_TEMPLATES  
			template <class ForwardIterator>
			void insert_aux(iterator pos, ForwardIterator first, ForwardIterator last,size_type n);
		#else /* __STL_MEMBER_TEMPLATES */
			void insert_aux(iterator pos,const value_type* first, const value_type* last,size_type n);
			void insert_aux(iterator pos, const_iterator first, const_iterator last,size_type n);
		#endif /* __STL_MEMBER_TEMPLATES */
		//在队列头或者尾预留n个位置，如果缓冲区不够则开辟新缓冲区。
		iterator reserve_elements_at_front(size_type n)
		{
			size_type vacancies = start.cur - start.first;
			if (n > vacancies)
			{
				new_elements_at_front(n - vacancies);
			}
			return start - difference_type(n);
		}
 
		iterator reserve_elements_at_back(size_type n) 
		{
			size_type vacancies = (finish.last - finish.cur) - 1;
			if (n > vacancies)
			{
				new_elements_at_back(n - vacancies);
			}
			return finish + difference_type(n);
		}
 
		void new_elements_at_front(size_type new_elements);
		void new_elements_at_back(size_type new_elements);
 
		void destroy_nodes_at_front(iterator before_start);
		void destroy_nodes_at_back(iterator after_finish);
 
	protected:                      // Allocation of map and nodes
		// Makes sure the map has space for new nodes.  Does not actually
		//  add the nodes.  Can invalidate map pointers.  (And consequently, 
		//  deque iterators.)
		//在map尾添加缓冲区
		void reserve_map_at_back (size_type nodes_to_add = 1) 
		{
			if (nodes_to_add + 1 > map_size - (finish.node - map))
			{
				//map空间不够用，则开辟新的map空间，把原来map内容拷贝过来。释放原来的
				reallocate_map(nodes_to_add, false);
			}
		}
 
		//在map头添加缓冲区
		void reserve_map_at_front (size_type nodes_to_add = 1) 
		{
			if (nodes_to_add > start.node - map)
			{
				reallocate_map(nodes_to_add, true);
			}
		}
 
		void reallocate_map(size_type nodes_to_add, bool add_at_front);
		//配置新的缓冲区
		pointer allocate_node() 
		{ 
			return data_allocator::allocate(buffer_size()); 
		}
		//释放缓冲区
		void deallocate_node(pointer n) 
		{
			data_allocator::deallocate(n, buffer_size());
		}
		//重载比较运算符
		#ifdef __STL_NON_TYPE_TMPL_PARAM_BUG
	public:
			bool operator==(const deque<T, Alloc, 0>& x) const 
			{
				return size() == x.size() && equal(begin(), end(), x.begin());
			}
			bool operator!=(const deque<T, Alloc, 0>& x) const 
			{
				return size() != x.size() || !equal(begin(), end(), x.begin());
			}
			bool operator<(const deque<T, Alloc, 0>& x) const 
			{
				return lexicographical_compare(begin(), end(), x.begin(), x.end());
			}
		#endif /* __STL_NON_TYPE_TMPL_PARAM_BUG */
};
 
// Non-inline member functions
//不是内联函数
 
//在pos处插入n个元素，元素值为n
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::insert(iterator pos,size_type n, const value_type& x)
{
	if (pos.cur == start.cur) 
	{
		//pos是deque的头
		iterator new_start = reserve_elements_at_front(n);
		uninitialized_fill(new_start, start, x);
		start = new_start;
	}
	else if (pos.cur == finish.cur) 
	{
		//pos是deque的尾
		iterator new_finish = reserve_elements_at_back(n);
		uninitialized_fill(finish, new_finish, x);
		finish = new_finish;
	}
	else
	{
		//中间位置
		insert_aux(pos, n, x);
	}
}
 
#ifndef __STL_MEMBER_TEMPLATES  
	//两个迭代器之间的元素插入到pos处
	template <class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::insert(iterator pos,const value_type* first,const value_type* last)
	{
		size_type n = last - first;
		if (pos.cur == start.cur) 
		{
			//deque的头
			//先预留好位置
			iterator new_start = reserve_elements_at_front(n);
			__STL_TRY 
			{
				//在未初始化内存上直接构造
				uninitialized_copy(first, last, new_start);
				start = new_start;
			}
			__STL_UNWIND(destroy_nodes_at_front(new_start));
		}
		else if (pos.cur == finish.cur) 
		{
			iterator new_finish = reserve_elements_at_back(n);
			__STL_TRY 
			{
				uninitialized_copy(first, last, finish);
				finish = new_finish;
			}
			//commi or rollback
			__STL_UNWIND(destroy_nodes_at_back(new_finish));
		}
		else
		{
			insert_aux(pos, first, last, n);
		}
	}
 
	template <class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::insert(iterator pos,const_iterator first,const_iterator last)
	{
		size_type n = last - first;
		if (pos.cur == start.cur) 
		{
			iterator new_start = reserve_elements_at_front(n);
			__STL_TRY 
			{
				uninitialized_copy(first, last, new_start);
				start = new_start;
			}
			__STL_UNWIND(destroy_nodes_at_front(new_start));
		}
		else if (pos.cur == finish.cur) 
		{
			iterator new_finish = reserve_elements_at_back(n);
			__STL_TRY 
			{
				uninitialized_copy(first, last, finish);
				finish = new_finish;
			}
			__STL_UNWIND(destroy_nodes_at_back(new_finish));
		}
		else
		{
			insert_aux(pos, first, last, n);
		}
	}
#endif /* __STL_MEMBER_TEMPLATES */
 
//擦除两个迭代器之间的元素
template <class T, class Alloc, size_t BufSize>
deque<T, Alloc, BufSize>::iterator 
deque<T, Alloc, BufSize>::erase(iterator first, iterator last) 
{
	if (first == start && last == finish) 
	{
		// 如果是清除整个 deque
		clear();							// 直接调用 clear() 即可
		return finish;
	}
	else 
	{
		difference_type n = last - first;			// 擦除区间长度
		difference_type elems_before = first - start;	// 擦除区间前方元素的个数
		if (elems_before < (size() - n) / 2) 
		{		
			// 如果前方的元素比更少，
			copy_backward(start, first, last);		// 前方元素向后移（覆盖擦除区间）
			iterator new_start = start + n;			// deque 的新起点
			destroy(start, new_start);				// 多于元素析构
			// 释放多于元素所占内存
			for (map_pointer cur = start.node; cur < new_start.node; ++cur)
			{
				data_allocator::deallocate(*cur, buffer_size());
			}
			start = new_start;	
		}
		else 
		{	
			// 后方元素更少
			copy(last, finish, first);			
			iterator new_finish = finish - n;	
			destroy(new_finish, finish);		
			for (map_pointer cur = new_finish.node + 1; cur <= finish.node; ++cur)
			{
				data_allocator::deallocate(*cur, buffer_size());
			}
			finish = new_finish;	
		}
		return start + elems_before;
	}
}
 
//清空deque。最后保留了一个缓冲区，这是deque的策略，也是其初始状态
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::clear() 
{
	//下面针对头尾以外的缓冲区，它们肯定是满的。
	for (map_pointer node = start.node + 1; node < finish.node; ++node) 
	{
		// 先析构
		destroy(*node, *node + buffer_size());
		// 再释放
		data_allocator::deallocate(*node, buffer_size());
	}
 
	if (start.node != finish.node) 
	{	
		// 至少有2个以上（含）缓冲区
		destroy(start.cur, start.last);	// 头缓冲区元素析构
		destroy(finish.first, finish.cur); // 尾缓冲区元素析构
		// 释放尾缓冲区，保留了头缓冲区
		data_allocator::deallocate(finish.first, buffer_size());
	}
	else	
	{
		// 只有一个缓冲区
		destroy(start.cur, finish.cur);	// 析构，但是不释放
	}
 
	finish = start;	// 调整迭代器，deque为空
}
 
//创建map，num_elements为元素个数
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::create_map_and_nodes(size_type num_elements) 
{
	//需要多少个缓冲区，即对应多少个map_node，如果刚好整除，会多出一个来
	size_type num_nodes = num_elements / buffer_size() + 1;
 
	//创建map结构,申请map空间 initial_map_size()=8
	map_size = max(initial_map_size(), num_nodes + 2);
	map = map_allocator::allocate(map_size);
 
	/*下面是nstart和nfinish指向map结构的最中间，这样可以使前后添加能力一样大。*/
	map_pointer nstart = map + (map_size - num_nodes) / 2;
	map_pointer nfinish = nstart + num_nodes - 1;
    
	map_pointer cur;
	__STL_TRY 
	{
		//为map内已用结点配置缓冲区。
		for (cur = nstart; cur <= nfinish; ++cur)
		{
			*cur = allocate_node();
		}
	}
	#ifdef  __STL_USE_EXCEPTIONS 
		catch(...) 
		{
			// "commit or rollback"
			for (map_pointer n = nstart; n < cur; ++n)
			{
				deallocate_node(*n);
			}
			map_allocator::deallocate(map, map_size);
			throw;
		}
	#endif /* __STL_USE_EXCEPTIONS */
 
	//为deque的迭代器start和end设定正确内容
	start.set_node(nstart);
	finish.set_node(nfinish);
	start.cur = start.first;		// first, cur都是public
	finish.cur = finish.first + num_elements % buffer_size();
	//正如前面所说，如果刚好整除会多出一个map_node。此时cur指向多出的这个缓冲区的起始位置。
}
 
// This is only used as a cleanup function in catch clauses.
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::destroy_map_and_nodes()
{
	for (map_pointer cur = start.node; cur <= finish.node; ++cur)
	{
		deallocate_node(*cur);
	}
	map_allocator::deallocate(map, map_size);
}
  
//分配n个结点，用value初始化
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::fill_initialize(size_type n,const value_type& value) 
{
	create_map_and_nodes(n);	 // 把deque的结构都安排好
	map_pointer cur;
	__STL_TRY 
	{
		// 每个结点缓冲区设定初始值。
		for (cur = start.node; cur < finish.node; ++cur)
		{
			uninitialized_fill(*cur, *cur + buffer_size(), value);
		}
		//最后一个结点有点不一样，因为尾端可能有未用空间，不必设置初始值
		uninitialized_fill(finish.first, finish.cur, value);
	}
	#ifdef __STL_USE_EXCEPTIONS
		catch(...) 
		{
			// "commit or rollback"
			for (map_pointer n = start.node; n < cur; ++n)
			{
				destroy(*n, *n + buffer_size());
			}
			destroy_map_and_nodes();
			throw;
		}
	#endif /* __STL_USE_EXCEPTIONS */
}
//用两个迭代器之间的元素初始化。迭代器类型不同，实现方法不同。
//input_iterator_tag类型迭代器一个一个初始化
//forward_iterator_tag(含其派生类型）内存处理工具初始化。
#ifdef __STL_MEMBER_TEMPLATES  
 
	template <class T, class Alloc, size_t BufSize>
	template <class InputIterator>
	void deque<T, Alloc, BufSize>::range_initialize(InputIterator first,InputIterator last,input_iterator_tag) 
	{
		create_map_and_nodes(0);//不分配结点
		for (; first != last; ++first)//一个一个初始化
		{
			push_back(*first);
		}
	}
 
	template <class T, class Alloc, size_t BufSize>
	template <class ForwardIterator>
	void deque<T, Alloc, BufSize>::range_initialize(ForwardIterator first,ForwardIterator last,forward_iterator_tag) 
	{
		size_type n = 0;
		distance(first, last, n);
		create_map_and_nodes(n);//分配好结点
		__STL_TRY 
		{
			uninitialized_copy(first, last, start);
		}
		//commit or rollback
		__STL_UNWIND(destroy_map_and_nodes());
	}
#endif /* __STL_MEMBER_TEMPLATES */
 
//只有当finish.cur == finish.last – 1才有调用
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::push_back_aux(const value_type& t) 
{
	value_type t_copy = t;
	/*调整map，如果map空间不够，则会在此配置新的map空间，并将旧的map数据复制到新的空间中*/
	reserve_map_at_back();	
	// 配置新缓冲区，并关系到map中
	*(finish.node + 1) = allocate_node();	
	__STL_TRY 
	{
		construct(finish.cur, t_copy);		// 设置值
		finish.set_node(finish.node + 1);	// 改变finish，令其指向新结点
		finish.cur = finish.first;			// 设置 finish 的状态
	}
	__STL_UNWIND(deallocate_node(*(finish.node + 1)));
}
 
// 只有当start.cur == start.first才会调用。
// 第一个缓冲区没有未用空间时才会调用。和上面实现类似
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::push_front_aux(const value_type& t) 
{
	value_type t_copy = t;
	reserve_map_at_front();		
	*(start.node - 1) = allocate_node();	
	__STL_TRY 
	{
		start.set_node(start.node - 1);		
		start.cur = start.last - 1;			
		construct(start.cur, t_copy);		
	}
	#ifdef __STL_USE_EXCEPTIONS
	catch(...) 
	{
		// "commit or rollback" 
		start.set_node(start.node + 1);
		start.cur = start.first;
		deallocate_node(*(start.node - 1));
		throw;
	}
	#endif /* __STL_USE_EXCEPTIONS */
} 
 
// 只有当finish.cur == finish.first才会调用
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::pop_back_aux() 
{
	deallocate_node(finish.first);	
	finish.set_node(finish.node - 1);	
	finish.cur = finish.last - 1;		
	destroy(finish.cur);		
}
 
// 只有当start.cur == start.last - 1时才会调用
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::pop_front_aux()
{
	destroy(start.cur);				
	deallocate_node(start.first);	
	start.set_node(start.node + 1);
	start.cur = start.first;
}      
 
#ifdef __STL_MEMBER_TEMPLATES  
	template <class T, class Alloc, size_t BufSize>
	template <class InputIterator>
	void deque<T, Alloc, BufSize>::insert(iterator pos,InputIterator first, InputIterator last,input_iterator_tag) 
	{
		copy(first, last, inserter(*this, pos));
	}
	//在pos处插入[first last)元素，对应迭代器类型为forward_iterator_tag(含派生)
	template <class T, class Alloc, size_t BufSize>
	template <class ForwardIterator>
	void deque<T, Alloc, BufSize>::insert(iterator pos,ForwardIterator first,ForwardIterator last,forward_iterator_tag) 
	{
		size_type n = 0;
		distance(first, last, n);
		if (pos.cur == start.cur) 
		{
			iterator new_start = reserve_elements_at_front(n);
			__STL_TRY 
			{
				uninitialized_copy(first, last, new_start);
				start = new_start;
			}
			__STL_UNWIND(destroy_nodes_at_front(new_start));
		}
		else if (pos.cur == finish.cur) 
		{
			iterator new_finish = reserve_elements_at_back(n);
			__STL_TRY 
			{
				uninitialized_copy(first, last, finish);
				finish = new_finish;
			}
			__STL_UNWIND(destroy_nodes_at_back(new_finish));
		}
		else
		{
			insert_aux(pos, first, last, n);
		}
	}
#endif /* __STL_MEMBER_TEMPLATES */
//在pos处插入一个元素，值为x。要判断插入点距头更近还是尾更近……
template <class T, class Alloc, size_t BufSize>
typename deque<T, Alloc, BufSize>::iterator
deque<T, Alloc, BufSize>::insert_aux(iterator pos, const value_type& x) 
{
	difference_type index = pos - start;	
	value_type x_copy = x;
	if (index < size() / 2) 
	{
		push_front(front());		
		iterator front1 = start;
		++front1;
		iterator front2 = front1;
		++front2;
		pos = start + index;
		iterator pos1 = pos;
		++pos1;
		copy(front2, pos1, front1);
	}
	else 
	{						
		push_back(back());		
		iterator back1 = finish;
		--back1;
		iterator back2 = back1;
		--back2;
		pos = start + index;
		copy_backward(pos, back2, back1);	
	}
	*pos = x_copy;	
	return pos;
}
//在pos处插入n个元素，值为x
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::insert_aux(iterator pos, size_type n, const value_type& x) 
{
	const difference_type elems_before = pos - start;
	size_type length = size();
	value_type x_copy = x;
	if (elems_before < length / 2) 
	{
		iterator new_start = reserve_elements_at_front(n);
		iterator old_start = start;
		pos = start + elems_before;
		__STL_TRY 
		{
			if (elems_before >= difference_type(n)) 
			{
				iterator start_n = start + difference_type(n);
				uninitialized_copy(start, start_n, new_start);
				start = new_start;
				copy(start_n, pos, old_start);
				fill(pos - difference_type(n), pos, x_copy);
			}
			else 
			{
				__uninitialized_copy_fill(start, pos, new_start, start, x_copy);
				start = new_start;
				fill(old_start, pos, x_copy);
			}
		}
		__STL_UNWIND(destroy_nodes_at_front(new_start));
	}
	else 
	{
		iterator new_finish = reserve_elements_at_back(n);
		iterator old_finish = finish;
		const difference_type elems_after = difference_type(length) - elems_before;
		pos = finish - elems_after;
		__STL_TRY 
		{
			if (elems_after > difference_type(n)) 
			{
				iterator finish_n = finish - difference_type(n);
				uninitialized_copy(finish_n, finish, finish);
				finish = new_finish;
				copy_backward(pos, finish_n, old_finish);
				fill(pos, pos + difference_type(n), x_copy);
			}
			else 
			{
				__uninitialized_fill_copy(finish, pos + difference_type(n),x_copy,pos, finish);
				finish = new_finish;
				fill(pos, old_finish, x_copy);
			}
		}
		__STL_UNWIND(destroy_nodes_at_back(new_finish));
	}
}
 
#ifdef __STL_MEMBER_TEMPLATES  
	//在pos处插入n个元素，n个元素值为[first last)
	template <class T, class Alloc, size_t BufSize>
	template <class ForwardIterator>
	void deque<T, Alloc, BufSize>::insert_aux(iterator pos,ForwardIterator first,ForwardIterator last,size_type n)
	{
	  const difference_type elems_before = pos - start;
	  size_type length = size();
	  if (elems_before < length / 2) {
		iterator new_start = reserve_elements_at_front(n);
		iterator old_start = start;
		pos = start + elems_before;
		__STL_TRY {
		  if (elems_before >= difference_type(n)) {
			iterator start_n = start + difference_type(n); 
			uninitialized_copy(start, start_n, new_start);
			start = new_start;
			copy(start_n, pos, old_start);
			copy(first, last, pos - difference_type(n));
		  }
		  else {
			ForwardIterator mid = first;
			advance(mid, difference_type(n) - elems_before);
			__uninitialized_copy_copy(start, pos, first, mid, new_start);
			start = new_start;
			copy(mid, last, old_start);
		  }
		}
		__STL_UNWIND(destroy_nodes_at_front(new_start));
	  }
	  else {
		iterator new_finish = reserve_elements_at_back(n);
		iterator old_finish = finish;
		const difference_type elems_after = difference_type(length) - elems_before;
		pos = finish - elems_after;
		__STL_TRY {
		  if (elems_after > difference_type(n)) {
			iterator finish_n = finish - difference_type(n);
			uninitialized_copy(finish_n, finish, finish);
			finish = new_finish;
			copy_backward(pos, finish_n, old_finish);
			copy(first, last, pos);
		  }
		  else {
			ForwardIterator mid = first;
			advance(mid, elems_after);
			__uninitialized_copy_copy(mid, last, pos, finish, finish);
			finish = new_finish;
			copy(first, mid, pos);
		  }
		}
		__STL_UNWIND(destroy_nodes_at_back(new_finish));
	  }
	}
#else /* __STL_MEMBER_TEMPLATES */
 
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::insert_aux(iterator pos,const value_type* first,const value_type* last,size_type n)
{
	const difference_type elems_before = pos - start;
	size_type length = size();
	if (elems_before < length / 2) 
	{
		iterator new_start = reserve_elements_at_front(n);
		iterator old_start = start;
		pos = start + elems_before;
		__STL_TRY 
		{
			if (elems_before >= difference_type(n)) 
			{
				iterator start_n = start + difference_type(n);
				uninitialized_copy(start, start_n, new_start);
				start = new_start;
				copy(start_n, pos, old_start);
				copy(first, last, pos - difference_type(n));
			}
			else 
			{
				const value_type* mid = first + (difference_type(n) - elems_before);
				__uninitialized_copy_copy(start, pos, first, mid, new_start);
				start = new_start;
				copy(mid, last, old_start);
			}
		}
		__STL_UNWIND(destroy_nodes_at_front(new_start));
	}
	else 
	{
		iterator new_finish = reserve_elements_at_back(n);
		iterator old_finish = finish;
		const difference_type elems_after = difference_type(length) - elems_before;
		pos = finish - elems_after;
		__STL_TRY 
		{
			if (elems_after > difference_type(n)) 
			{
				iterator finish_n = finish - difference_type(n);
				uninitialized_copy(finish_n, finish, finish);
				finish = new_finish;
				copy_backward(pos, finish_n, old_finish);
				copy(first, last, pos);
			}
			else 
			{
				const value_type* mid = first + elems_after;
				__uninitialized_copy_copy(mid, last, pos, finish, finish);
				finish = new_finish;
				copy(first, mid, pos);
			}
		}
		__STL_UNWIND(destroy_nodes_at_back(new_finish));
	}
}
 
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::insert_aux(iterator pos,const_iterator first,const_iterator last,size_type n)
{
	const difference_type elems_before = pos - start;
	size_type length = size();
	if (elems_before < length / 2) 
	{
		iterator new_start = reserve_elements_at_front(n);
		iterator old_start = start;
		pos = start + elems_before;
		__STL_TRY 
		{
			if (elems_before >= n) 
			{
				iterator start_n = start + n;
				uninitialized_copy(start, start_n, new_start);
				start = new_start;
				copy(start_n, pos, old_start);
				copy(first, last, pos - difference_type(n));
			}
			else 
			{
				const_iterator mid = first + (n - elems_before);
				__uninitialized_copy_copy(start, pos, first, mid, new_start);
				start = new_start;
				copy(mid, last, old_start);
			}
		}
		__STL_UNWIND(destroy_nodes_at_front(new_start));
	}
	else 
	{
		iterator new_finish = reserve_elements_at_back(n);
		iterator old_finish = finish;
		const difference_type elems_after = length - elems_before;
		pos = finish - elems_after;
		__STL_TRY 
		{
			if (elems_after > n) 
			{
				iterator finish_n = finish - difference_type(n);
				uninitialized_copy(finish_n, finish, finish);
				finish = new_finish;
				copy_backward(pos, finish_n, old_finish);
				copy(first, last, pos);
			}
			else 
			{
				const_iterator mid = first + elems_after;
				__uninitialized_copy_copy(mid, last, pos, finish, finish);
				finish = new_finish;
				copy(first, mid, pos);
			}
		}
		__STL_UNWIND(destroy_nodes_at_back(new_finish));
	}
}
 
#endif /* __STL_MEMBER_TEMPLATES */
//在deque头分配新的结点
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::new_elements_at_front(size_type new_elements) 
{
	size_type new_nodes = (new_elements + buffer_size() - 1) / buffer_size();
	reserve_map_at_front(new_nodes);
	size_type i;
	__STL_TRY 
	{
		for (i = 1; i <= new_nodes; ++i)
		{
			*(start.node - i) = allocate_node();
		}
	}
	#ifdef __STL_USE_EXCEPTIONS
		catch(...) 
		{
			for (size_type j = 1; j < i; ++j)
			{
				deallocate_node(*(start.node - j));
			}
			throw;
		}
	#endif /* __STL_USE_EXCEPTIONS */
}
//在deque尾分配新的结点
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::new_elements_at_back(size_type new_elements) 
{
	size_type new_nodes = (new_elements + buffer_size() - 1) / buffer_size();
	reserve_map_at_back(new_nodes);
	size_type i;
	__STL_TRY 
	{
		for (i = 1; i <= new_nodes; ++i)
		{
			*(finish.node + i) = allocate_node();
		}
	}
	#ifdef __STL_USE_EXCEPTIONS
	  catch(...) 
		{
			for (size_type j = 1; j < i; ++j)
			{
				deallocate_node(*(finish.node + j));
			}
			throw;
		}
	#endif /* __STL_USE_EXCEPTIONS */
}
//释放[before_start statr)
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::destroy_nodes_at_front(iterator before_start) 
{
	for (map_pointer n = before_start.node; n < start.node; ++n)
	{
		deallocate_node(*n);
	}
}
//释放(finish.node after_finish]
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::destroy_nodes_at_back(iterator after_finish) 
{	
	for (map_pointer n = after_finish.node; n > finish.node; --n)
	{
		deallocate_node(*n);
	}
}
//添加map结点，指向新的缓冲区，add_at_front=true添加在map头，否则添加在尾
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::reallocate_map(size_type nodes_to_add,bool add_at_front) 
{
	size_type old_num_nodes = finish.node - start.node + 1;
	size_type new_num_nodes = old_num_nodes + nodes_to_add;
 
	map_pointer new_nstart;
	if (map_size > 2 * new_num_nodes) 
	{
		new_nstart = map + (map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);
		if (new_nstart < start.node)
		{
			copy(start.node, finish.node + 1, new_nstart);
		}
		else
		{
			copy_backward(start.node, finish.node + 1, new_nstart + old_num_nodes);
		}
	}
	else 
	{
		size_type new_map_size = map_size + max(map_size, nodes_to_add) + 2;
		//申请新的map空间
		map_pointer new_map = map_allocator::allocate(new_map_size);
		new_nstart = new_map + (new_map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);
		// 把原map 內容拷贝
		copy(start.node, finish.node + 1, new_nstart);
		// 释放放原map
		map_allocator::deallocate(map, map_size);
		// 设置新map起始位置和大小
		map = new_map;
		map_size = new_map_size;
	}
	// 重新设置迭代器 start 和 finish
	start.set_node(new_nstart);
	finish.set_node(new_nstart + old_num_nodes - 1);
}
 
 
// Nonmember functions.
//非成员函数，标准STL算法
#ifndef __STL_NON_TYPE_TMPL_PARAM_BUG
	template <class T, class Alloc, size_t BufSiz>
	bool operator==(const deque<T, Alloc, BufSiz>& x,const deque<T, Alloc, BufSiz>& y) 
	{
		return x.size() == y.size() && equal(x.begin(), x.end(), y.begin());
	}
 
	template <class T, class Alloc, size_t BufSiz>
	bool operator<(const deque<T, Alloc, BufSiz>& x,const deque<T, Alloc, BufSiz>& y)
	{
		return lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
	}
#endif /* __STL_NON_TYPE_TMPL_PARAM_BUG */
#if defined(__STL_FUNCTION_TMPL_PARTIAL_ORDER) && \ !defined(__STL_NON_TYPE_TMPL_PARAM_BUG)
	template <class T, class Alloc, size_t BufSiz>
	inline void swap(deque<T, Alloc, BufSiz>& x, deque<T, Alloc, BufSiz>& y) 
	{
		x.swap(y);
	}
#endif
#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#endif
          
__STL_END_NAMESPACE 
  
#endif /* __SGI_STL_INTERNAL_DEQUE_H */
// Local Variables:
// mode:C++
// End:
