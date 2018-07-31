G++ 2.91.57，cygnus\cygwin-b20\include\g++\stl_alloc.h 完整列表
/*
 * Copyright (c) 1996-1997
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
 
#ifndef __SGI_STL_INTERNAL_ALLOC_H
#define __SGI_STL_INTERNAL_ALLOC_H
 
#ifdef __SUNPRO_CC
#  define __PRIVATE public
   // Extra access restrictions prevent us from really making some things
   // private.
#else
#  define __PRIVATE private
#endif
 
#ifdef __STL_STATIC_TEMPLATE_MEMBER_BUG
#  define __USE_MALLOC
#endif
 
 
 
#if 0
#   include <new>
#   define __THROW_BAD_ALLOC throw bad_alloc
#elif !defined(__THROW_BAD_ALLOC)//定义内存申请出错处理
#   include <iostream.h>
#   define __THROW_BAD_ALLOC cerr << "out of memory" << endl; exit(1)
#endif
 
#ifndef __ALLOC
#   define __ALLOC alloc
#endif
#ifdef __STL_WIN32THREADS
#   include <windows.h>
#endif
 
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifndef __RESTRICT
#  define __RESTRICT
#endif
/*
如果编译器不支持多线程，那么就不用多线程
__STL_PTHREADS gcc编译器，POSIX接口
__STL_WIN32THREADS msvc编译器
*/
#if !defined(__STL_PTHREADS) && !defined(_NOTHREADS) \
 && !defined(__STL_SGI_THREADS) && !defined(__STL_WIN32THREADS)
#   define _NOTHREADS  //不支持多线程
#endif
/*
如果是gcc编译器，那么添加对互斥锁的支持
*/
# ifdef __STL_PTHREADS
    // POSIX Threads
    // This is dubious, since this is likely to be a high contention
    // lock.   Performance may not be adequate.
#   include <pthread.h>
#   define __NODE_ALLOCATOR_LOCK \
        if (threads) pthread_mutex_lock(&__node_allocator_lock)
#   define __NODE_ALLOCATOR_UNLOCK \
        if (threads) pthread_mutex_unlock(&__node_allocator_lock)
#   define __NODE_ALLOCATOR_THREADS true
#   define __VOLATILE volatile  // Needed at -O3 on SGI
# endif
/*
msvc编译器
*/
# ifdef __STL_WIN32THREADS
    // The lock needs to be initialized by constructing an allocator
    // objects of the right type.  We do that here explicitly for alloc.
#   define __NODE_ALLOCATOR_LOCK \
        EnterCriticalSection(&__node_allocator_lock)
#   define __NODE_ALLOCATOR_UNLOCK \
        LeaveCriticalSection(&__node_allocator_lock)
#   define __NODE_ALLOCATOR_THREADS true
#   define __VOLATILE volatile  // may not be needed
# endif /* WIN32THREADS */
/*SGI专用*/
# ifdef __STL_SGI_THREADS
    // This should work without threads, with sproc threads, or with
    // pthreads.  It is suboptimal in all cases.
    // It is unlikely to even compile on nonSGI machines.
 
    extern "C" {
      extern int __us_rsthread_malloc;
    }
	// The above is copied from malloc.h.  Including <malloc.h>
	// would be cleaner but fails with certain levels of standard
	// conformance.
#   define __NODE_ALLOCATOR_LOCK if (threads && __us_rsthread_malloc) \
                { __lock(&__node_allocator_lock); }
#   define __NODE_ALLOCATOR_UNLOCK if (threads && __us_rsthread_malloc) \
                { __unlock(&__node_allocator_lock); }
#   define __NODE_ALLOCATOR_THREADS true
#   define __VOLATILE volatile  // Needed at -O3 on SGI
# endif
/*不支持多线程*/
# ifdef _NOTHREADS
//  Thread-unsafe
#   define __NODE_ALLOCATOR_LOCK
#   define __NODE_ALLOCATOR_UNLOCK
#   define __NODE_ALLOCATOR_THREADS false
#   define __VOLATILE
# endif
 
__STL_BEGIN_NAMESPACE
 
#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#endif
 
// malloc-based allocator. 通常比稍後介紹的 default alloc 速度慢，
// 一般而言是 thread-safe，並且對於空間的運用比較高效（efficient）。
#ifdef __STL_STATIC_TEMPLATE_MEMBER_BUG
# ifdef __DECLARE_GLOBALS_HERE
    void (* __malloc_alloc_oom_handler)() = 0;
    // g++ 2.7.2 不支持 static template data members.
# else
    extern void (* __malloc_alloc_oom_handler)();
# endif
#endif
 
/*
下面就是第一级配置器，没有template参数，inst没有用
*/
template <int inst>
class __malloc_alloc_template {
 
private:
/*
oom是指out of memory
定义函数指针，用来处理内存申请失败的情况，C++ 的 set_new_handler()
*/
static void *oom_malloc(size_t);
 
static void *oom_realloc(void *, size_t);
/*
在下面set_malloc_handler函数设置，用于内存申请失败时的处理，C++ 的 set_new_handler()
*/
#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG//如果编译器支持静态模板类
    static void (* __malloc_alloc_oom_handler)();
#endif
 
public:
 
static void * allocate(size_t n)
{
    void *result = malloc(n);	// 第一级配置器直接用malloc申请内存
    //当malloc申请失败，使用oom_malloc
    if (0 == result) result = oom_malloc(n);
    return result;
}
 
static void deallocate(void *p, size_t /* n */)
{
    free(p);	// 第一级配置器直接用free释放内存
}
 
static void * reallocate(void *p, size_t /* old_sz */, size_t new_sz)
{
    void * result = realloc(p, new_sz);	// 第一级配置器直接使用 realloc()
    if (0 == result) result = oom_realloc(p, new_sz);
    return result;
}
 
/*
设置内存申请失败的错误处理函数，类似 C++ 的 set_new_handler()。
这个是客端设置的，而不是编译器设置。如果不设置，则内存配置失败马上终止
*/
static void (* set_malloc_handler(void (*f)()))()
{
    void (* old)() = __malloc_alloc_oom_handler;
    __malloc_alloc_oom_handler = f;
    return(old);
}
 
};
 
// malloc_alloc out-of-memory handling，初始值为0
#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
template <int inst>
void (* __malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;
#endif
 
template <int inst>
void * __malloc_alloc_template<inst>::oom_malloc(size_t n)
{
    void (* my_malloc_handler)();
    void *result;
 
    for (;;) {	// 不断尝试释放、配置、再释放、再配置……
        my_malloc_handler = __malloc_alloc_oom_handler;
        //如果没有设置out-of-memory handling处理函数，则抛出异常，终止
        if (0 == my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*my_malloc_handler)();		// 调用处理函数，企图释放内存
        result = malloc(n);			//再次尝试配置内存
        if (result) return(result);
    }
}
//和上面的函数类似
template <int inst>
void * __malloc_alloc_template<inst>::oom_realloc(void *p, size_t n)
{
    void (* my_malloc_handler)();
    void *result;
 
    for (;;) {	// 不断尝试释放、配置、再释放、再配置……
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*my_malloc_handler)();	
        result = realloc(p, n);	
        if (result) return(result);
    }
}
//将参数inst设为0
typedef __malloc_alloc_template<0> malloc_alloc;
/*
无论alloc是第一级配置器还是第二级配置器，SGI还为它包装一个接口，使之
符合STL规范
*/
template<class T, class Alloc>
class simple_alloc {
 
public:
/*
下面四个函数都是单纯的转调用，调用传递给配置器（可能是第一级，也可能是第二级），
根据sizeof(T)或n*sizeof(T)的大小
*/
    static T *allocate(size_t n)
                { return 0 == n? 0 : (T*) Alloc::allocate(n * sizeof (T)); }
    static T *allocate(void)
                { return (T*) Alloc::allocate(sizeof (T)); }
    static void deallocate(T *p, size_t n)
                { if (0 != n) Alloc::deallocate(p, n * sizeof (T)); }
    static void deallocate(T *p)
                { Alloc::deallocate(p, sizeof (T)); }
};
 
// Allocator adaptor to check size arguments for debugging.
// Reports errors using assert.  Checking can be disabled with
// NDEBUG, but it's far better to just use the underlying allocator
// instead when no checking is desired.
// There is some evidence that this can confuse Purify.
template <class Alloc>
class debug_alloc {
 
private:
/*
extra用于记录配置内存大小，同时保证字节对齐
*/
enum {extra = 8};       // Size of space used to store size.  Note
                        // that this must be large enough to preserve
                        // alignment.
 
public:
 
static void * allocate(size_t n)
{
    char *result = (char *)Alloc::allocate(n + extra);
    *(size_t *)result = n;//内存分配前面记录分配内存的大小
    return result + extra;//返回内存不包括extra大小
}
 
static void deallocate(void *p, size_t n)
{
    char * real_p = (char *)p - extra;//释放时，要加上extra大小内存
    assert(*(size_t *)real_p == n);//判断前面的数据是否被修改，如果修改则说明有越界
    Alloc::deallocate(real_p, n + extra);
}
 
static void * reallocate(void *p, size_t old_sz, size_t new_sz)
{
    char * real_p = (char *)p - extra;
    assert(*(size_t *)real_p == old_sz);
    char * result = (char *)
                  Alloc::reallocate(real_p, old_sz + extra, new_sz + extra);
    *(size_t *)result = new_sz;
    return result + extra;
}
 
 
};
 
 
# ifdef __USE_MALLOC
 
typedef malloc_alloc alloc;	// 令 alloc 为第一级配置器
typedef malloc_alloc single_client_alloc;
 
# else
 
 
// Default node allocator.
// With a reasonable compiler, this should be roughly as fast as the
// original STL class-specific allocators, but with less fragmentation.
// Default_alloc_template parameters are experimental and MAY
// DISAPPEAR in the future.  Clients should just use alloc for now.
/*
翻译：默认的内存配置器。在合适的编译器上，它的性能（SGI版本的）应该和STL原版
的配置器性能大致相同，但是SGi版本的使内存碎片更少。
默认的内存配置器只是实验性的且以后可能会消失。客端现在应该只是用alloc
*/
//
// Important implementation properties:
// 1. If the client request an object of size > __MAX_BYTES, the resulting
//    object will be obtained directly from malloc.
// 2. In all other cases, we allocate an object of size exactly
//    ROUND_UP(requested_size).  Thus the client has enough size
//    information that we can return the object to the proper free list
//    without permanently losing part of the object.
//
/*
翻译：实现中的特性
1、当客端请求内存大小size>__MAX_BYTES时，对象直接调用malloc
2、否则，把size ROUND_UP为8的整数倍，从free list中
*/
// The first template parameter specifies whether more than one thread
// may use this allocator.  It is safe to allocate an object from
// one instance of a default_alloc and deallocate it with another
// one.  This effectively transfers its ownership to the second one.
// This may have undesirable effects on reference locality.
// The second parameter is unreferenced and serves only to allow the
// creation of multiple default_alloc instances.
// Node that containers built on different allocator instances have
// different types, limiting the utility of this approach.
/*
翻译：模板的第一个参数来指定是否有多于一个线程在使用这个alloctor。
在一个实例中配置内存，在另一个实例中释放是安全的。这样可以有效的转换内存
使用权。这可能会在引用区域产生意想不到的影响。
第二个参数是非引用的，仅用于创建多个default_alloc实例。
注意：使用不同的allocator创建的容器有不同特性，这限制了通用性。
*/
#ifdef __SUNPRO_CC
// breaks if we make these template class members:
  enum {__ALIGN = 8};			// 小型区块的上上调界
  enum {__MAX_BYTES = 128};		// 小型区块的上限
  enum {__NFREELISTS = __MAX_BYTES/__ALIGN};	// free-lists 个数，共16个
#endif
 
// 以下是第二级配置器。
// 注意，没有模板参数，inst没有用，第一个参数用于多线程。
template <bool threads, int inst>
class __default_alloc_template {
 
private:
/*
实际上，我们应该使用static const int x = N来取代enum { x = N }，
但是目前支持该性能的编译器不多
*/
# ifndef __SUNPRO_CC
    enum {__ALIGN = 8};
    enum {__MAX_BYTES = 128};
    enum {__NFREELISTS = __MAX_BYTES/__ALIGN};
# endif
//将bytes上调至8的整数倍
  static size_t ROUND_UP(size_t bytes) {
        return (((bytes) + __ALIGN-1) & ~(__ALIGN - 1));
  }
__PRIVATE:
/*
这是free list内的结点。
采用union，尽量减少占用内存。
如果使用free_list_link，则指向相同的union结构，这个供链表free list使用。
如果使用client_data[1]，则给客端使用
*/
  union obj {
        union obj * free_list_link;
        char client_data[1];    /* The client sees this. */
  };
private:
# ifdef __SUNPRO_CC
    static obj * __VOLATILE free_list[]; 
        // Specifying a size results in duplicate def for 4.1
# else
	//__NFREELISTS值为16，对应链表维护内存大小为8、16、…、128
    static obj * __VOLATILE free_list[__NFREELISTS]; 
# endif
//根据bytes大小，在16个链表中选取合适的那个
  static  size_t FREELIST_INDEX(size_t bytes) {
        return (((bytes) + __ALIGN-1)/__ALIGN - 1);
  }
 
  // Returns an object of size n, and optionally adds to size n free list.
  //返回一个大小为n的对象，并且可能加入大小为n的其他区块到free list
  static void *refill(size_t n);
  // Allocates a chunk for nobjs of size "size".  nobjs may be reduced
  // if it is inconvenient to allocate the requested number.
  /*
  配置一大块空间，可容纳nobjs个size大小的区块，如果配置
  nobjs个区块有所不便，nobjs可能会降低。
  */
  static char *chunk_alloc(size_t size, int &nobjs);
 
  // Chunk allocation state.
  static char *start_free;//内存池起始位置
  static char *end_free;//内存池结束位置
  static size_t heap_size;//在堆上已有内存的大小
//如果支持多SGI线程，则提供锁支持
# ifdef __STL_SGI_THREADS
    static volatile unsigned long __node_allocator_lock;
    static void __lock(volatile unsigned long *); 
    static inline void __unlock(volatile unsigned long *);
# endif
//如果支持多线程，则提供互斥锁
# ifdef __STL_PTHREADS
    static pthread_mutex_t __node_allocator_lock;
# endif
//win32多线程
# ifdef __STL_WIN32THREADS
    static CRITICAL_SECTION __node_allocator_lock;
    static bool __node_allocator_lock_initialized;
 
  public:
    __default_alloc_template() {
	// This assumes the first constructor is called before threads
	// are started.
	//假设构造函数在多线程启动前已经调用
        if (!__node_allocator_lock_initialized) {
            InitializeCriticalSection(&__node_allocator_lock);
            __node_allocator_lock_initialized = true;
        }
    }
  private:
# endif
 
    class lock {
        public:
            lock() { __NODE_ALLOCATOR_LOCK; }
            ~lock() { __NODE_ALLOCATOR_UNLOCK; }
    };
    friend class lock;
 
public:
 
  /* n must be > 0      */
  static void * allocate(size_t n)
  {
    obj * __VOLATILE * my_free_list;
    obj * __RESTRICT result;
 
    if (n > (size_t) __MAX_BYTES) {//如果配置内存大于__MAX_BYTES，使用第一级配置器
        return(malloc_alloc::allocate(n));
    }
    my_free_list = free_list + FREELIST_INDEX(n);//在16个free lists中找到对应的那个
    // Acquire the lock here with a constructor call.
    // This ensures that it is released in exit or during stack
    // unwinding.
#       ifndef _NOTHREADS
        /*REFERENCED*/
        lock lock_instance;
#       endif
    result = *my_free_list;
    if (result == 0) {//如果没找可用的free list，那么重新填充free list
        void *r = refill(ROUND_UP(n));
        return r;
    }
    //调整free list
    *my_free_list = result -> free_list_link;
    return (result);
  };
 
  /* p may not be 0 */
  static void deallocate(void *p, size_t n)
  {
    obj *q = (obj *)p;
    obj * __VOLATILE * my_free_list;
 
    if (n > (size_t) __MAX_BYTES) {//调用第一级配置器的释放函数
        malloc_alloc::deallocate(p, n);
        return;
    }
    //在16个free lists中找到对应的那个
    my_free_list = free_list + FREELIST_INDEX(n);
    // acquire lock
#       ifndef _NOTHREADS
        /*REFERENCED*/
        lock lock_instance;
#       endif /* _NOTHREADS */
    //回收内存到free list
    q -> free_list_link = *my_free_list;
    *my_free_list = q;
    // lock is released here
  }
 
  static void * reallocate(void *p, size_t old_sz, size_t new_sz);
 
} ;
 
typedef __default_alloc_template<__NODE_ALLOCATOR_THREADS, 0> alloc;
typedef __default_alloc_template<false, 0> single_client_alloc;
 
 
 
/* We allocate memory in large chunks in order to avoid fragmenting     */
/* the malloc heap too much.                                            */
/* We assume that size is properly aligned.                             */
/* We hold the allocation lock.                                         */
/*
分配内存时分配一大块，防止多次分配小内存造成内存碎片
假设size已经对齐
持有allocation锁
*/
//从内存池中去空间给free list，nobjs是引用调用，原因是可能会修改其值。
//当不够nobjs个区块时，可能适当调小nobjs的值
template <bool threads, int inst>
char*
__default_alloc_template<threads, inst>::chunk_alloc(size_t size, int& nobjs)
{
    char * result;
    size_t total_bytes = size * nobjs;//要配置的空间大小
    size_t bytes_left = end_free - start_free;//内存池大小
 
    if (bytes_left >= total_bytes) {//内存池空间满足需求
        result = start_free;
        start_free += total_bytes;
        return(result);
    } else if (bytes_left >= size) {
    	//内存池空间不够，但是足够供应一个（含）以上的块
        nobjs = bytes_left/size;
        total_bytes = size * nobjs;
        result = start_free;
        start_free += total_bytes;
        return(result);
    } else {
    	//内存池剩余空间大小连一个块大小都无法提供
        size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
        // Try to make use of the left-over piece.
        //尝试内存池中的参与零头还有利用价值
        if (bytes_left > 0) {
            obj * __VOLATILE * my_free_list =//找到对应的free list
                        free_list + FREELIST_INDEX(bytes_left);
						//调整free list，将内存池空间编入
            ((obj *)start_free) -> free_list_link = *my_free_list;
            *my_free_list = (obj *)start_free;
        }
        //配置heap，用来补充内存池
        start_free = (char *)malloc(bytes_to_get);
        if (0 == start_free) {
            int i;
            obj * __VOLATILE * my_free_list, *p;
            // Try to make do with what we have.  That can't
            // hurt.  We do not try smaller requests, since that tends
            // to result in disaster on multi-process machines.
            /*
            尝试用现有的，这不会造成破坏。我们不尝试配置较小的区块，
            因为这样做将会在多线程机器上造成灾难
            */
            //搜索适当free list，适当是指“尚未用区块，且足够大”的free list
            for (i = size; i <= __MAX_BYTES; i += __ALIGN) {
                my_free_list = free_list + FREELIST_INDEX(i);
                p = *my_free_list;
                if (0 != p) {//free内有尚未用区块
                	//调整free list，释出未用区块
                    *my_free_list = p -> free_list_link;
                    start_free = (char *)p;
                    end_free = start_free + i;
                    //递归调用自己，修正nobjs
                    return(chunk_alloc(size, nobjs));
                    // Any leftover piece will eventually make it to the
                    // right free list.
                    //任何参与的零头将会被编入适当地free list中备用
                }
            }
      //如果出现意外，到处无可用内存
	    end_free = 0;	// In case of exception.
	    			//调用第一级配置器，看看out-of-memory机制能否尽点力
            start_free = (char *)malloc_alloc::allocate(bytes_to_get);
            // This should either throw an
            // exception or remedy the situation.  Thus we assume it
            // succeeded.
            //这里可能会抛出异常，或内存不足情况得到改善。
        }
        heap_size += bytes_to_get;
        end_free = start_free + bytes_to_get;
        //递归调用自己，修正nobjs
        return(chunk_alloc(size, nobjs));
    }
}
 
 
/* Returns an object of size n, and optionally adds to size n free list.*/
/* We assume that n is properly aligned.                                */
/* We hold the allocation lock.                                         */
/*
这个函数和上一个差不多，没有第二个参数nobjs。这个函数在函数体中设置了其大小为20.
返回大小为n的对象，且加入到free list中
我们假设n已经对齐
持有allocation锁
*/
template <bool threads, int inst>
void* __default_alloc_template<threads, inst>::refill(size_t n)
{
    int nobjs = 20;
    char * chunk = chunk_alloc(n, nobjs);
    obj * __VOLATILE * my_free_list;
    obj * result;
    obj * current_obj, * next_obj;
    int i;
		//如果只够一个块的大小
    if (1 == nobjs) return(chunk);
    my_free_list = free_list + FREELIST_INDEX(n);
 
    /* Build free list in chunk */
    //在chunk中建立free list
      result = (obj *)chunk;
      *my_free_list = next_obj = (obj *)(chunk + n);
      for (i = 1; ; i++) {
        current_obj = next_obj;
        next_obj = (obj *)((char *)next_obj + n);
        if (nobjs - 1 == i) {
            current_obj -> free_list_link = 0;
            break;
        } else {
            current_obj -> free_list_link = next_obj;
        }
      }
    return(result);
}
/*
扩展现有内存，重新分配内存，要把旧内存内容拷贝到新内存
*/
template <bool threads, int inst>
void*
__default_alloc_template<threads, inst>::reallocate(void *p,
                                                    size_t old_sz,
                                                    size_t new_sz)
{
    void * result;
    size_t copy_sz;
		//如果就内存和新内存都大于_MAX_BYTES，直接调用realloc
    if (old_sz > (size_t) __MAX_BYTES && new_sz > (size_t) __MAX_BYTES) {
        return(realloc(p, new_sz));
    }
    //内存大小没变化（没变化是指经过上调为8的整数倍后没变化），直接返回
    if (ROUND_UP(old_sz) == ROUND_UP(new_sz)) return(p);
    result = allocate(new_sz);//分配新内存
    copy_sz = new_sz > old_sz? old_sz : new_sz;
    memcpy(result, p, copy_sz);//拷贝旧内存的数据到新内存
    deallocate(p, old_sz);//释放就内存
    return(result);
}
 
#ifdef __STL_PTHREADS
    template <bool threads, int inst>
    pthread_mutex_t
    __default_alloc_template<threads, inst>::__node_allocator_lock
        = PTHREAD_MUTEX_INITIALIZER;
#endif
 
#ifdef __STL_WIN32THREADS
    template <bool threads, int inst> CRITICAL_SECTION
    __default_alloc_template<threads, inst>::__node_allocator_lock;
 
    template <bool threads, int inst> bool
    __default_alloc_template<threads, inst>::__node_allocator_lock_initialized
	= false;
#endif
 
#ifdef __STL_SGI_THREADS
__STL_END_NAMESPACE
#include <mutex.h>
#include <time.h>
__STL_BEGIN_NAMESPACE
// Somewhat generic lock implementations.  We need only test-and-set
// and some way to sleep.  These should work with both SGI pthreads
// and sproc threads.  They may be useful on other systems.
template <bool threads, int inst>
volatile unsigned long
__default_alloc_template<threads, inst>::__node_allocator_lock = 0;
 
#if __mips < 3 || !(defined (_ABIN32) || defined(_ABI64)) || defined(__GNUC__)
#   define __test_and_set(l,v) test_and_set(l,v)
#endif
 
template <bool threads, int inst>
void 
__default_alloc_template<threads, inst>::__lock(volatile unsigned long *lock)
{
    const unsigned low_spin_max = 30;  // spin cycles if we suspect uniprocessor
    const unsigned high_spin_max = 1000; // spin cycles for multiprocessor
    static unsigned spin_max = low_spin_max;
    unsigned my_spin_max;
    static unsigned last_spins = 0;
    unsigned my_last_spins;
    static struct timespec ts = {0, 1000};
    unsigned junk;
#   define __ALLOC_PAUSE junk *= junk; junk *= junk; junk *= junk; junk *= junk
    int i;
 
    if (!__test_and_set((unsigned long *)lock, 1)) {
        return;
    }
    my_spin_max = spin_max;
    my_last_spins = last_spins;
    for (i = 0; i < my_spin_max; i++) {
        if (i < my_last_spins/2 || *lock) {
            __ALLOC_PAUSE;
            continue;
        }
        if (!__test_and_set((unsigned long *)lock, 1)) {
            // got it!
            // Spinning worked.  Thus we're probably not being scheduled
            // against the other process with which we were contending.
            // Thus it makes sense to spin longer the next time.
            last_spins = i;
            spin_max = high_spin_max;
            return;
        }
    }
    // We are probably being scheduled against the other process.  Sleep.
    spin_max = low_spin_max;
    for (;;) {
        if (!__test_and_set((unsigned long *)lock, 1)) {
            return;
        }
        nanosleep(&ts, 0);
    }
}
 
template <bool threads, int inst>
inline void
__default_alloc_template<threads, inst>::__unlock(volatile unsigned long *lock)
{
#   if defined(__GNUC__) && __mips >= 3
        asm("sync");
        *lock = 0;
#   elif __mips >= 3 && (defined (_ABIN32) || defined(_ABI64))
        __lock_release(lock);
#   else 
        *lock = 0;
        // This is not sufficient on many multiprocessors, since
        // writes to protected variables and the lock may be reordered.
#   endif
}
#endif
 
//内存池的起始地址、结束地址以及大小的初始化
template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::start_free = 0;
 
template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::end_free = 0;
 
template <bool threads, int inst>
size_t __default_alloc_template<threads, inst>::heap_size = 0;
 
template <bool threads, int inst>
__default_alloc_template<threads, inst>::obj * __VOLATILE
__default_alloc_template<threads, inst> ::free_list[
# ifdef __SUNPRO_CC
    __NFREELISTS
# else
    __default_alloc_template<threads, inst>::__NFREELISTS
# endif//free list的初始化
] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
// The 16 zeros are necessary to make version 4.1 of the SunPro
// compiler happy.  Otherwise it appears to allocate too little
// space for the array.
 
# ifdef __STL_WIN32THREADS
  // Create one to get critical section initialized.
  // We do this onece per file, but only the first constructor
  // does anything.
  static alloc __node_allocator_dummy_instance;
# endif
 
#endif /* ! __USE_MALLOC */
 
#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#endif
 
__STL_END_NAMESPACE
 
#undef __PRIVATE
 
#endif /* __SGI_STL_INTERNAL_ALLOC_H */
 
// Local Variables:
// mode:C++
// End:
