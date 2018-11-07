/*
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
 *
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
 */
 
/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */
 
#ifndef __SGI_STL_INTERNAL_HASHTABLE_H
#define __SGI_STL_INTERNAL_HASHTABLE_H
 
// Hashtable class 用来作 hashed associative containers
// hash_set, hash_map, hash_multiset, 和 hash_multimap.
 
#include <stl_algobase.h>
#include <stl_alloc.h>
#include <stl_construct.h>
#include <stl_tempbuf.h>
#include <stl_algo.h>
#include <stl_uninitialized.h>
#include <stl_function.h>
#include <stl_vector.h>
#include <stl_hash_fun.h>
 
__STL_BEGIN_NAMESPACE
//hash table中节点的定义，都是public
template <class Value>
struct __hashtable_node
{
 /*
	用vector来做hash table，为什么还需要next指针？
	因为SGI 实现的hash table使用了开链法/链接法。
	hash table中的节点可能代表一系列节点。它们以链形式连接。
	这是所谓的 separate chaining 技巧。
 */
  __hashtable_node* next;	
  Value val;
};  
//先声明 hash table，在 iterator中有用到。
template <class Value, class Key, class HashFcn,class ExtractKey, class EqualKey, class Alloc = alloc> class hashtable;
 
// 由与 __hashtable_iterator 和 __hashtable_const_iterator 两者会
// 互相使用，因此必须在下面先做声明，否则编译出错。
template <class Value, class Key, class HashFcn,class ExtractKey, class EqualKey, class Alloc>struct __hashtable_iterator;
 
template <class Value, class Key, class HashFcn,class ExtractKey, class EqualKey, class Alloc>struct __hashtable_const_iterator;
 
//hash table中的迭代器
template <class Value, class Key, class HashFcn,class ExtractKey, class EqualKey, class Alloc>struct __hashtable_iterator 
{
	typedef hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc> hashtable;
	typedef __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc> iterator;
	typedef __hashtable_const_iterator<Value, Key, HashFcn,ExtractKey, EqualKey, Alloc> const_iterator;
	typedef __hashtable_node<Value> node;
 
	//迭代器类型，只能向前
	typedef forward_iterator_tag iterator_category;
	typedef Value value_type;
	typedef ptrdiff_t difference_type;
	typedef size_t size_type;
	typedef Value& reference;
	typedef Value* pointer;
 
	node* cur;		// 迭代器目前所指之节点
	hashtable* ht;	// 保持对容器的连接关系，因为可能需要从bucket跳到bucket
 
	__hashtable_iterator(node* n, hashtable* tab) : cur(n), ht(tab) {}
	//默认构造函数什么也没做
	__hashtable_iterator() {}
	reference operator*() const { return cur->val; }
	#ifndef __SGI_STL_NO_ARROW_OPERATOR
	pointer operator->() const { return &(operator*()); }
	#endif /* __SGI_STL_NO_ARROW_OPERATOR */
	iterator& operator++();
	iterator operator++(int);
	bool operator==(const iterator& it) const { return cur == it.cur; }
	bool operator!=(const iterator& it) const { return cur != it.cur; }
};
 
 
template <class Value, class Key, class HashFcn,class ExtractKey, class EqualKey, class Alloc>struct __hashtable_const_iterator 
{
	typedef hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc> hashtable;
	typedef __hashtable_iterator<Value, Key, HashFcn,ExtractKey, EqualKey, Alloc> iterator;
	typedef __hashtable_const_iterator<Value, Key, HashFcn,ExtractKey, EqualKey, Alloc> const_iterator;
	typedef __hashtable_node<Value> node;
 
	typedef forward_iterator_tag iterator_category;	// 注意
	typedef Value value_type;
	typedef ptrdiff_t difference_type;
	typedef size_t size_type;
	typedef const Value& reference;
	typedef const Value* pointer;
 
	const node* cur;
	const hashtable* ht;
 
	__hashtable_const_iterator(const node* n, const hashtable* tab)
	: cur(n), ht(tab) {}
	__hashtable_const_iterator() {}
	__hashtable_const_iterator(const iterator& it) : cur(it.cur), ht(it.ht) {}
	reference operator*() const { return cur->val; }
	#ifndef __SGI_STL_NO_ARROW_OPERATOR
	pointer operator->() const { return &(operator*()); }
	#endif /* __SGI_STL_NO_ARROW_OPERATOR */
	const_iterator& operator++();
	const_iterator operator++(int);
	bool operator==(const const_iterator& it) const { return cur == it.cur; }
	bool operator!=(const const_iterator& it) const { return cur != it.cur; }
};
 
// 注意：假设 long 至少有 32 bits。
//定义28个素数（大概是2倍关系增长），用来做hash table的大小
static const int __stl_num_primes = 28;
static const unsigned long __stl_prime_list[__stl_num_primes] =
{
  53,         97,           193,         389,       769,
  1543,       3079,         6151,        12289,     24593,
  49157,      98317,        196613,      393241,    786433,
  1572869,    3145739,      6291469,     12582917,  25165843,
  50331653,   100663319,    201326611,   402653189, 805306457, 
  1610612741, 3221225473ul, 4294967291ul
};
 
//找出28个素数中，最接近n且大于n的那个数
inline unsigned long __stl_next_prime(unsigned long n)
{
	const unsigned long* first = __stl_prime_list;
	const unsigned long* last = __stl_prime_list + __stl_num_primes;
	const unsigned long* pos = lower_bound(first, last, n);
	// 以上，lower_bound() 是泛型算法
	// 使用 lower_bound()，序列需先排序。上述数组以排序
	return pos == last ? *(last - 1) : *pos;
}
/*
Value 节点的实值类型
Key   节点的键值类型
HashFcn hash function的类型
EqualKey从节点中取出键值的方法（函数或仿函数）
EqualKey判断键值是否相同的方法（函数或仿函数）
*/
template <class Value, class Key, class HashFcn,
          class ExtractKey, class EqualKey,
          class Alloc> 	// 最上面已经说明：默认使用 alloc 空间配置器。
class hashtable 
{
	public:
		//为 template  类型参数重新定义一个名称（貌似没必要）
		typedef Key key_type;
		typedef Value value_type;
		typedef HashFcn hasher;//hash函数
		typedef EqualKey key_equal;
 
		typedef size_t            size_type;
		typedef ptrdiff_t         difference_type;
		typedef value_type*       pointer;
		typedef const value_type* const_pointer;
		typedef value_type&       reference;
		typedef const value_type& const_reference;
 
		hasher hash_funct() const { return hash; }
		key_equal key_eq() const { return equals; }
 
	private:
		//以下三个都是 function objects。。、<stl_hash_fun.h>中定义了几个
		//标准类型(如int,c-style string等)的 hasher。
		hasher hash;	
		key_equal equals;
		ExtractKey get_key;
 
		typedef __hashtable_node<Value> node;
		typedef simple_alloc<node, Alloc> node_allocator;
 
		vector<node*,Alloc> buckets;	// 以 vector 完成
		size_type num_elements;//hash table中节点的个数
 
	public:
		typedef __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey,Alloc> iterator;
		typedef __hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey,Alloc> const_iterator;
 
		friend struct __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>;
		friend struct __hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>;
 
	public:
		//没有默认的构造函数
		hashtable(size_type n,const HashFcn& hf,const EqualKey& eql,const ExtractKey& ext) : hash(hf), equals(eql), get_key(ext), num_elements(0)
		{
			initialize_buckets(n);
		}
 
		hashtable(size_type n,const HashFcn& hf,const EqualKey& eql) : hash(hf), equals(eql), get_key(ExtractKey()), num_elements(0)
		{
			initialize_buckets(n);
		}
 
		hashtable(const hashtable& ht): hash(ht.hash), equals(ht.equals), get_key(ht.get_key), num_elements(0)
		{
			copy_from(ht);
		}
 
		hashtable& operator= (const hashtable& ht)
		{
			//防止自身赋值
			if (&ht != this) 
			{	
				clear();		// 先清除自己
				hash = ht.hash;	// 以下三个动作，将三份data members 复制过来。
				equals = ht.equals;
				get_key = ht.get_key;
				copy_from(ht);	// 完整赋值整个 hash table的内容。
			}
			return *this;
		}
 
		~hashtable() { clear(); }
 
		size_type size() const { return num_elements; }
		size_type max_size() const { return size_type(-1); }
		bool empty() const { return size() == 0; }
 
		void swap(hashtable& ht)
		{
			__STD::swap(hash, ht.hash);
			__STD::swap(equals, ht.equals);
			__STD::swap(get_key, ht.get_key);
			buckets.swap(ht.buckets);
			__STD::swap(num_elements, ht.num_elements);
		}
 
		iterator begin()
		{ 
			for (size_type n = 0; n < buckets.size(); ++n)
			{
				//找出第一个被使用的节点，此即 begin iterator。
				if (buckets[n])
				{
					return iterator(buckets[n], this);
				}
			}
			return end();
		}
 
		//最后被使用节点的下个位置，所以使用0来初始化迭代器
		iterator end() { return iterator(0, this); }
 
		const_iterator begin() const
		{
			for (size_type n = 0; n < buckets.size(); ++n)
			{
				if (buckets[n])
				{
					return const_iterator(buckets[n], this);
				}
			}
			return end();
		}
 
		const_iterator end() const { return const_iterator(0, this); }
 
		friend bool operator== __STL_NULL_TMPL_ARGS (const hashtable&, const hashtable&);
 
	public:
 
		// bucket 个数即 buckets vector 的大小
		size_type bucket_count() const 
		{ 
			return buckets.size(); 
		}
 
		//以目前情况（不重建表格），总共可以有多少个 buckets
		size_type max_bucket_count() const
		{
			return __stl_prime_list[__stl_num_primes - 1]; 
		} 
 
		// 某一个 bucket （内含一个list） 容纳多少个元素
		size_type elems_in_bucket(size_type bucket) const
		{
			size_type result = 0;
			for (node* cur = buckets[bucket]; cur; cur = cur->next)
			{
				result += 1;
			}
			return result;
		}
 
		//安插元素，不允许重复
		pair<iterator, bool> insert_unique(const value_type& obj)
		{
			resize(num_elements + 1); 	// 判断是否需要重建表格，如果需要就填充
			return insert_unique_noresize(obj);
		}
 
		// 安插元素，允许重复
		iterator insert_equal(const value_type& obj)
		{
			resize(num_elements + 1);
			return insert_equal_noresize(obj);
		}
 
		pair<iterator, bool> insert_unique_noresize(const value_type& obj);
		iterator insert_equal_noresize(const value_type& obj);
 
#ifdef __STL_MEMBER_TEMPLATES
//插入两个迭代器之间的元素[f l)
  template <class InputIterator>
  void insert_unique(InputIterator f, InputIterator l)
  {
    insert_unique(f, l, iterator_category(f));
  }
 
  template <class InputIterator>
  void insert_equal(InputIterator f, InputIterator l)
  {
    insert_equal(f, l, iterator_category(f));
  }
 
  template <class InputIterator>
  void insert_unique(InputIterator f, InputIterator l,
                     input_iterator_tag)
  {
    for ( ; f != l; ++f)
      insert_unique(*f);
  }
 
  template <class InputIterator>
  void insert_equal(InputIterator f, InputIterator l,
                    input_iterator_tag)
  {
    for ( ; f != l; ++f)
      insert_equal(*f);
  }
 
  template <class ForwardIterator>
  void insert_unique(ForwardIterator f, ForwardIterator l,
                     forward_iterator_tag)
  {
    size_type n = 0;
    distance(f, l, n);//判断两个迭代器的距离，n是引用传递
    resize(num_elements + n);		// 判断（并实施）表格的重建
    for ( ; n > 0; --n, ++f)
      insert_unique_noresize(*f);	// 一一安插新元素
  }
 
  template <class ForwardIterator>
  void insert_equal(ForwardIterator f, ForwardIterator l,
                    forward_iterator_tag)
  {
    size_type n = 0;
    distance(f, l, n);
    resize(num_elements + n);
    for ( ; n > 0; --n, ++f)
      insert_equal_noresize(*f);		// 一一安插新元素
  }
 
#else /* __STL_MEMBER_TEMPLATES */
	void insert_unique(const value_type* f, const value_type* l)
	{
		//可以直接计算迭代器之间的距离，应该是rando access iterator
		size_type n = l - f;
		resize(num_elements + n);
		for (; n > 0; --n, ++f)
		{
			insert_unique_noresize(*f);
		}
	}
 
  void insert_equal(const value_type* f, const value_type* l)
  {
    size_type n = l - f;
    resize(num_elements + n);
    for ( ; n > 0; --n, ++f)
      insert_equal_noresize(*f);
  }
 
  void insert_unique(const_iterator f, const_iterator l)
  {
    size_type n = 0;
    distance(f, l, n);
    resize(num_elements + n);
    for ( ; n > 0; --n, ++f)
      insert_unique_noresize(*f);
  }
 
  void insert_equal(const_iterator f, const_iterator l)
  {
    size_type n = 0;
    distance(f, l, n);
    resize(num_elements + n);
    for ( ; n > 0; --n, ++f)
      insert_equal_noresize(*f);
  }
#endif /*__STL_MEMBER_TEMPLATES */
 
  reference find_or_insert(const value_type& obj);
 
  iterator find(const key_type& key) 
  {
    size_type n = bkt_num_key(key);	// 首先找到落在哪个bucket内
    node* first;
	//从bucket list的头开始，一一对比每个元素的键值。
    for ( first = buckets[n];	
          first && !equals(get_key(first->val), key);
          first = first->next)
      {}
    return iterator(first, this);
  } 
 
  const_iterator find(const key_type& key) const
  {
    size_type n = bkt_num_key(key);
    const node* first;
    for ( first = buckets[n];
          first && !equals(get_key(first->val), key);
          first = first->next)
      {}
    return const_iterator(first, this);
  } 
 
  //查看hash table中含有多少个值为key的元素
  size_type count(const key_type& key) const
  {
    const size_type n = bkt_num_key(key);
    size_type result = 0;
 
    // 以下，从bucket list 的头开始，一一比对每个素的键值。比对成功就累加1。
    for (const node* cur = buckets[n]; cur; cur = cur->next)
      if (equals(get_key(cur->val), key))
        ++result;
    return result;
  }
 
  pair<iterator, iterator> equal_range(const key_type& key);
  pair<const_iterator, const_iterator> equal_range(const key_type& key) const;
 
  size_type erase(const key_type& key);
  void erase(const iterator& it);
  void erase(iterator first, iterator last);
 
  void erase(const const_iterator& it);
  void erase(const_iterator first, const_iterator last);
 
  void resize(size_type num_elements_hint);
  void clear();
 
private:
  // 寻找STL中提供的下一个质数
  size_type next_size(size_type n) const { return __stl_next_prime(n); }
 
  // 注意，hash_vec 和 hash_map 都將其底層的 hash table 的初始大小預設為 100
  // hash_vec 和 hash_map 都将底层的 hash table初始化大小预设为100
  void initialize_buckets(size_type n)
  {
    //例如：传入100,返回193。以下首先保留193个元素空间，然后将其全部填0。
	//例如：传入50,返回53。以下首先保留53个元素空间，然后将其全部填0。
    const size_type n_buckets = next_size(n); 
    
    buckets.reserve(n_buckets);
    buckets.insert(buckets.end(), n_buckets, (node*) 0);
    num_elements = 0;
  }
 
  size_type bkt_num_key(const key_type& key) const
  {
    return bkt_num_key(key, buckets.size());
  }
 
  size_type bkt_num(const value_type& obj) const
  {
    return bkt_num_key(get_key(obj));
  }
 
  size_type bkt_num_key(const key_type& key, size_t n) const
  {
    return hash(key) % n;
  }
 
  size_type bkt_num(const value_type& obj, size_t n) const
  {
    return bkt_num_key(get_key(obj), n);
  }
 
	node* new_node(const value_type& obj)
	{
		node* n = node_allocator::allocate();//配置空间
		n->next = 0;//指针next设置为NULL
		__STL_TRY 
		{
			construct(&n->val, obj);//构建元素
			return n;
		}
		//commit or rollback
		__STL_UNWIND(node_allocator::deallocate(n));
	}
  
  void delete_node(node* n)
  {
    destroy(&n->val);//析构
    node_allocator::deallocate(n);//释放空间
  }
 
  void erase_bucket(const size_type n, node* first, node* last);
  void erase_bucket(const size_type n, node* last);
 
  void copy_from(const hashtable& ht);
 
};
 
template <class V, class K, class HF, class ExK, class EqK, class A>
__hashtable_iterator<V, K, HF, ExK, EqK, A>&
__hashtable_iterator<V, K, HF, ExK, EqK, A>::operator++()
{
  const node* old = cur;
  cur = cur->next;	// 如果存在，就是它。否则进入以下 if 流程
  if (!cur) {
	// 根据原值，重新定位。从该位置(bucket)的下一个位置找起。
    size_type bucket = ht->bkt_num(old->val);
    while (!cur && ++bucket < ht->buckets.size())	// 注意，prefix operator++
      cur = ht->buckets[bucket];
  }
  return *this;
}
 
template <class V, class K, class HF, class ExK, class EqK, class A>
inline __hashtable_iterator<V, K, HF, ExK, EqK, A>
__hashtable_iterator<V, K, HF, ExK, EqK, A>::operator++(int)
{
  iterator tmp = *this;
  ++*this;	// 调用 operator++()
  return tmp;
}
 
template <class V, class K, class HF, class ExK, class EqK, class A>
__hashtable_const_iterator<V, K, HF, ExK, EqK, A>&
__hashtable_const_iterator<V, K, HF, ExK, EqK, A>::operator++()
{
  const node* old = cur;
  cur = cur->next;
  if (!cur) {
    size_type bucket = ht->bkt_num(old->val);
    while (!cur && ++bucket < ht->buckets.size())
      cur = ht->buckets[bucket];
  }
  return *this;
}
 
template <class V, class K, class HF, class ExK, class EqK, class A>
inline __hashtable_const_iterator<V, K, HF, ExK, EqK, A>
__hashtable_const_iterator<V, K, HF, ExK, EqK, A>::operator++(int)
{
  const_iterator tmp = *this;
  ++*this;
  return tmp;
}
 
#ifndef __STL_CLASS_PARTIAL_SPECIALIZATION
 
template <class V, class K, class HF, class ExK, class EqK, class All>
inline forward_iterator_tag
iterator_category(const __hashtable_iterator<V, K, HF, ExK, EqK, All>&)
{
  return forward_iterator_tag();
}
 
template <class V, class K, class HF, class ExK, class EqK, class All>
inline V* value_type(const __hashtable_iterator<V, K, HF, ExK, EqK, All>&)
{
  return (V*) 0;
}
 
template <class V, class K, class HF, class ExK, class EqK, class All>
inline hashtable<V, K, HF, ExK, EqK, All>::difference_type*
distance_type(const __hashtable_iterator<V, K, HF, ExK, EqK, All>&)
{
  return (hashtable<V, K, HF, ExK, EqK, All>::difference_type*) 0;
}
 
template <class V, class K, class HF, class ExK, class EqK, class All>
inline forward_iterator_tag
iterator_category(const __hashtable_const_iterator<V, K, HF, ExK, EqK, All>&)
{
  return forward_iterator_tag();
}
 
template <class V, class K, class HF, class ExK, class EqK, class All>
inline V* 
value_type(const __hashtable_const_iterator<V, K, HF, ExK, EqK, All>&)
{
  return (V*) 0;
}
 
template <class V, class K, class HF, class ExK, class EqK, class All>
inline hashtable<V, K, HF, ExK, EqK, All>::difference_type*
distance_type(const __hashtable_const_iterator<V, K, HF, ExK, EqK, All>&)
{
  return (hashtable<V, K, HF, ExK, EqK, All>::difference_type*) 0;
}
 
#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */
//判断两个hash table是否相等（两个hasn table的 buckets相同，且
//bucket对应的list相同)
template <class V, class K, class HF, class Ex, class Eq, class A>
bool operator==(const hashtable<V, K, HF, Ex, Eq, A>& ht1,
                const hashtable<V, K, HF, Ex, Eq, A>& ht2)
{
  typedef typename hashtable<V, K, HF, Ex, Eq, A>::node node;
  if (ht1.buckets.size() != ht2.buckets.size())
    return false;
  for (int n = 0; n < ht1.buckets.size(); ++n) {
    node* cur1 = ht1.buckets[n];
    node* cur2 = ht2.buckets[n];
    for ( ; cur1 && cur2 && cur1->val == cur2->val;
          cur1 = cur1->next, cur2 = cur2->next)
      {}
    if (cur1 || cur2)//如果cur1或cur2有一个不等于0（没有指向最后位置的下一个位置）
      return false;
  }
  return true;
}  
 
#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
 
template <class Val, class Key, class HF, class Extract, class EqKey, class A>
inline void swap(hashtable<Val, Key, HF, Extract, EqKey, A>& ht1,
                 hashtable<Val, Key, HF, Extract, EqKey, A>& ht2) {
  ht1.swap(ht2);
}
 
#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */
 
//在不重建表格的情况下安插新节点。键值不允许重复。返回pair。第二个参数指出
//插入是否成功
template <class V, class K, class HF, class Ex, class Eq, class A>
pair<typename hashtable<V, K, HF, Ex, Eq, A>::iterator, bool> 
hashtable<V, K, HF, Ex, Eq, A>::insert_unique_noresize(const value_type& obj)
{
	const size_type n = bkt_num(obj);	// 決定obj位于哪个buckets中
	node* first = buckets[n];	 // 令 first 指向 bucket 对应串列头部
 
	// 如果 buckets[n] 已被占用，此时first 将不为0，于是进入以下循环，
	// 遍历bucket对应的整个链表
	for (node* cur = first; cur; cur = cur->next)
	{
		if (equals(get_key(cur->val), get_key(obj)))
		{
			// 如果发现链表中的某键值相同，就不安插，立刻回返。
			return pair<iterator, bool>(iterator(cur, this), false);
		}
	}
 
  //离开以上循环（或没进入循环），first指向指向bucket所指链表的头部节点
  node* tmp = new_node(obj);	// 生成新节点并赋值
  tmp->next = first;			//更改新节点指针
  buckets[n] = tmp; 			// 新节点称为bucket链表第一个节点
  ++num_elements;				// 节点个诉累加1
  return pair<iterator, bool>(iterator(tmp, this), true);
}
 
//在不重建表格的情况下安插新节点。键值不允许重复。
template <class V, class K, class HF, class Ex, class Eq, class A>
typename hashtable<V, K, HF, Ex, Eq, A>::iterator 
hashtable<V, K, HF, Ex, Eq, A>::insert_equal_noresize(const value_type& obj)
{
  const size_type n = bkt_num(obj); 
  node* first = buckets[n]; 
 
  // 如果 buckets[n] 已被佔用，此時first 將不為0，於是進入以下迴圈，
  // 走過 bucket 所對應的整個串列。
  for (node* cur = first; cur; cur = cur->next) 
    if (equals(get_key(cur->val), get_key(obj))) {
      // 如果发现链表中键值相同，马上插入，然后返回
	  //插到键值相同节点的后面
      node* tmp = new_node(obj);
      tmp->next = cur->next;
      cur->next = tmp;
      ++num_elements;	
      return iterator(tmp, this);	// 返回迭代器，指向新插入的节点
    }
 
  // 运行到此处，没有键值重复。
  node* tmp = new_node(obj);
  tmp->next = first;
  buckets[n] = tmp;
  ++num_elements;
  return iterator(tmp, this);
}
 
//如果存在obj节点则返回指向其节点的迭代器，否则插入
template <class V, class K, class HF, class Ex, class Eq, class A>
typename hashtable<V, K, HF, Ex, Eq, A>::reference 
hashtable<V, K, HF, Ex, Eq, A>::find_or_insert(const value_type& obj)
{
  resize(num_elements + 1);
 
  size_type n = bkt_num(obj);
  node* first = buckets[n];
 
  for (node* cur = first; cur; cur = cur->next)
    if (equals(get_key(cur->val), get_key(obj)))
      return cur->val;
 
  node* tmp = new_node(obj);
  tmp->next = first;
  buckets[n] = tmp;
  ++num_elements;
  return tmp->val;
}
 
template <class V, class K, class HF, class Ex, class Eq, class A>
pair<typename hashtable<V, K, HF, Ex, Eq, A>::iterator,
     typename hashtable<V, K, HF, Ex, Eq, A>::iterator> 
hashtable<V, K, HF, Ex, Eq, A>::equal_range(const key_type& key)
{
  typedef pair<iterator, iterator> pii;
  const size_type n = bkt_num_key(key);
 
  for (node* first = buckets[n]; first; first = first->next) {
    if (equals(get_key(first->val), key)) {
      for (node* cur = first->next; cur; cur = cur->next)
        if (!equals(get_key(cur->val), key))
          return pii(iterator(first, this), iterator(cur, this));
      for (size_type m = n + 1; m < buckets.size(); ++m)
        if (buckets[m])
          return pii(iterator(first, this),
                     iterator(buckets[m], this));
      return pii(iterator(first, this), end());
    }
  }
  return pii(end(), end());
}
//查找键值等于key的区间。pair两个元素类型都是迭代器类型
//一个指向区间起始位置，一个指向区间结束的下一个位置。
template <class V, class K, class HF, class Ex, class Eq, class A>
pair<typename hashtable<V, K, HF, Ex, Eq, A>::const_iterator, 
     typename hashtable<V, K, HF, Ex, Eq, A>::const_iterator> 
hashtable<V, K, HF, Ex, Eq, A>::equal_range(const key_type& key) const
{
  typedef pair<const_iterator, const_iterator> pii;
  const size_type n = bkt_num_key(key);//先找到在哪个 buckets
	//bucket对应的链表中查找
  for (const node* first = buckets[n] ; first; first = first->next) {
    if (equals(get_key(first->val), key)) {//找到键值为key的起始位置
      for (const node* cur = first->next; cur; cur = cur->next)
        if (!equals(get_key(cur->val), key))
          return pii(const_iterator(first, this),
                     const_iterator(cur, this));
					 
		//运行到此处，说明bucket对应的链表中尾节点也是键值为key节点
		//那么下一个位置就是 buckets中可用的bucket链表头节点
      for (size_type m = n + 1; m < buckets.size(); ++m)
        if (buckets[m])
          return pii(const_iterator(first, this),
                     const_iterator(buckets[m], this));
		//后面的 buckets都没用
      return pii(const_iterator(first, this), end());
    }
  }
  // hash table没有key值节点
  return pii(end(), end());
}
//擦除键值为key的节点
template <class V, class K, class HF, class Ex, class Eq, class A>
typename hashtable<V, K, HF, Ex, Eq, A>::size_type 
hashtable<V, K, HF, Ex, Eq, A>::erase(const key_type& key)
{
  const size_type n = bkt_num_key(key);
  node* first = buckets[n];//找到对应bucket链表头节点
  size_type erased = 0;
 
  if (first) {
    node* cur = first;//这里要保存前一个节点，因为是单向链表
    node* next = cur->next;
	//如果链表中多于一个节点
    while (next) {
      if (equals(get_key(next->val), key)) {
        cur->next = next->next;
        delete_node(next);
        next = cur->next;
        ++erased;
        --num_elements;
      }
      else {
        cur = next;
        next = cur->next;
      }
    }
	//链表中只有一个节点
    if (equals(get_key(first->val), key)) {
      buckets[n] = first->next;
      delete_node(first);
      ++erased;
      --num_elements;
    }
  }
  return erased;
}
 
template <class V, class K, class HF, class Ex, class Eq, class A>
void hashtable<V, K, HF, Ex, Eq, A>::erase(const iterator& it)
{
  if (node* const p = it.cur) {
    const size_type n = bkt_num(p->val);
    node* cur = buckets[n];
 
    if (cur == p) {
      buckets[n] = cur->next;
      delete_node(cur);
      --num_elements;
    }
    else {
      node* next = cur->next;
      while (next) {
        if (next == p) {
          cur->next = next->next;
          delete_node(next);
          --num_elements;
          break;
        }
        else {
          cur = next;
          next = cur->next;
        }
      }
    }
  }
}
//擦除两个迭代器之间的元素
template <class V, class K, class HF, class Ex, class Eq, class A>
void hashtable<V, K, HF, Ex, Eq, A>::erase(iterator first, iterator last)
{
  size_type f_bucket = first.cur ? bkt_num(first.cur->val) : buckets.size();
  size_type l_bucket = last.cur ? bkt_num(last.cur->val) : buckets.size();
 
  if (first.cur == last.cur)
    return;
  else if (f_bucket == l_bucket)
    erase_bucket(f_bucket, first.cur, last.cur);
  else {
    erase_bucket(f_bucket, first.cur, 0);
    for (size_type n = f_bucket + 1; n < l_bucket; ++n)
      erase_bucket(n, 0);
    if (l_bucket != buckets.size())
      erase_bucket(l_bucket, last.cur);
  }
}
 
template <class V, class K, class HF, class Ex, class Eq, class A>
inline void
hashtable<V, K, HF, Ex, Eq, A>::erase(const_iterator first,
                                      const_iterator last)
{
  erase(iterator(const_cast<node*>(first.cur),
                 const_cast<hashtable*>(first.ht)),
        iterator(const_cast<node*>(last.cur),
                 const_cast<hashtable*>(last.ht)));
}
 
template <class V, class K, class HF, class Ex, class Eq, class A>
inline void
hashtable<V, K, HF, Ex, Eq, A>::erase(const const_iterator& it)
{
  erase(iterator(const_cast<node*>(it.cur),
                 const_cast<hashtable*>(it.ht)));
}
//重新配置 hash table的大小
template <class V, class K, class HF, class Ex, class Eq, class A>
void hashtable<V, K, HF, Ex, Eq, A>::resize(size_type num_elements_hint)
{
  const size_type old_n = buckets.size();//原来hash table大小
  if (num_elements_hint > old_n) {	// 确定真的需要重新配置
    const size_type n = next_size(num_elements_hint);	// 找出下一个质数
	
	//下个这个判断没必要了吧？因为n>=num_elements_hint，而num_elements_hint>=old_n
    if (n > old_n) {
      vector<node*, A> tmp(n, (node*) 0);	// 设立新的 buckets
      __STL_TRY {
		// 下面处理每一个旧的bucket
        for (size_type bucket = 0; bucket < old_n; ++bucket) {
          node* first = buckets[bucket]; // 指向节点所对应链表的起始节点
          // 以下處理每一個舊bucket 所含（串列）的每一個節點
		  // 一下处理bucketliability的每一个节点
          while (first) {	// 链表没结束
            // 以下找出节点落在哪一个新bucket 內
            size_type new_bucket = bkt_num(first->val, n);
            // 以下四个动作颇为巧妙
            // (1) 令旧 bucket 指向其所对应之链表的下一个节点（以便迭代处理）
            buckets[bucket] = first->next; 
            // (2)(3) 将当前节点安插到新的bucket内，成为其对应链表的第一个节点。
            first->next = tmp[new_bucket];	
            tmp[new_bucket] = first;
            // (4) 回到旧bucket 所指的待处理链表，准备处理下一个节点
            first = buckets[bucket];          
          }
        }
        buckets.swap(tmp);	// vector::swap。新旧 buckets 对调。
        // 注意，对调两方如果大小不同，大的会变小，小的会变大。
        // tmp为局部作用域，离开其作用域自动释放
      }
#         ifdef __STL_USE_EXCEPTIONS
		//commit or rollback
      catch(...) {
        for (size_type bucket = 0; bucket < tmp.size(); ++bucket) {
          while (tmp[bucket]) {
            node* next = tmp[bucket]->next;
            delete_node(tmp[bucket]);
            tmp[bucket] = next;
          }
        }
        throw;
      }
#         endif /* __STL_USE_EXCEPTIONS */
    }
  }
}
//擦除hash table对应第n个bucket中的一段元素[first last)
template <class V, class K, class HF, class Ex, class Eq, class A>
void hashtable<V, K, HF, Ex, Eq, A>::erase_bucket(const size_type n, 
                                                  node* first, node* last)
{
  node* cur = buckets[n];
  if (cur == first)
    erase_bucket(n, last);
  else {
    node* next;
    for (next = cur->next; next != first; cur = next, next = cur->next)
      ;
	  //下面应该是 while(next!=last)吧？否则last在这没用
    while (next) {
      cur->next = next->next;
      delete_node(next);
      next = cur->next;
      --num_elements;
    }
  }
}
////擦除hash table对应第n个bucket中的一段元素[buckets[n] last)
template <class V, class K, class HF, class Ex, class Eq, class A>
void 
hashtable<V, K, HF, Ex, Eq, A>::erase_bucket(const size_type n, node* last)
{
  node* cur = buckets[n];
  while (cur != last) {
    node* next = cur->next;
    delete_node(cur);
    cur = next;
    buckets[n] = cur;
    --num_elements;
  }
}
 
template <class V, class K, class HF, class Ex, class Eq, class A>
void hashtable<V, K, HF, Ex, Eq, A>::clear()
{
  // 针对每一个 bucket.
  for (size_type i = 0; i < buckets.size(); ++i) {
    node* cur = buckets[i];
    // 将 bucket list 中的每一个节点刪除掉
    while (cur != 0) {
      node* next = cur->next;
      delete_node(cur);
      cur = next;
    }
    buckets[i] = 0; 	// 令bucket 內容为 null 指针
  }
  num_elements = 0; 	// 令总节点个数0
 
  // 注意，buckets vector 并未释放掉，仍保有原来大小。
}
 
    
template <class V, class K, class HF, class Ex, class Eq, class A>
void hashtable<V, K, HF, Ex, Eq, A>::copy_from(const hashtable& ht)
{
  // 先清除己方的buckets vector. 调用vector::clear. 
  buckets.clear();	
 
  //如果己方空间大于对方，就不懂，否则增大己方空间等于对方
  buckets.reserve(ht.buckets.size()); 
  
  //从己方的buckets vector尾端开始，安插n个元素，其值为NULL指针。
  //注意，此时buckets vector为空，所以所谓尾端就是起始处
  buckets.insert(buckets.end(), ht.buckets.size(), (node*) 0);
  __STL_TRY {
    // 针对 buckets vector 
    for (size_type i = 0; i < ht.buckets.size(); ++i) {
	  //复制 vector 的每一个元素（是个指针，指向hash table节点）
	  
	  //注意下面if语句，是先赋值再判断，它等价于
	  //const node* cur = ht.buckets[i]; if(cur)
      if (const node* cur = ht.buckets[i]) {
        node* copy = new_node(cur->val);
        buckets[i] = copy;
 
        // 针对每一个 bucket list，复制每一个节点
        for (node* next = cur->next; next; cur = next, next = cur->next) {
          copy->next = new_node(next->val);
          copy = copy->next;
        }
      }
    }
    num_elements = ht.num_elements;	// 重新设置节点个数（hashtable 的大小）
  }
  __STL_UNWIND(clear());
}
 
__STL_END_NAMESPACE
 
#endif /* __SGI_STL_INTERNAL_HASHTABLE_H */
 
// Local Variables:
// mode:C++
// End:
 
