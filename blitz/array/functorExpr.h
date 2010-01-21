// -*- C++ -*-
/***************************************************************************
 * blitz/array/functorExpr.h   User-defined functors for arrays
 *
 * $Id$
 *
 * Copyright (C) 1997-2001 Todd Veldhuizen <tveldhui@oonumerics.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Suggestions:          blitz-dev@oonumerics.org
 * Bugs:                 blitz-bugs@oonumerics.org
 *
 * For more information, please see the Blitz++ Home Page:
 *    http://oonumerics.org/blitz/
 *
 ****************************************************************************/
/* This header file is designed to allow the use of Blitz++ with 
   functors (classes defining an operator()) and more general member
   functions. It works best if you have access to the class source code;
   there is limited support for classes that cannot be modified. The best
   approach in that case is usually to write an adapter class.

   This works with class methods that take one, two or three arguments.

   If you have a functor, add the following to your (public) class declaration:

   BZ_DECLARE_FUNCTOR(classname)   // for one argument functors
   BZ_DECLARE_FUNCTOR2(classname)  // for two argument functors
   BZ_DECLARE_FUNCTOR3(classname)  // for three argument functors
   
   or

   BZ_DECLARE_FUNCTOR_RET(classname, returnType)
   BZ_DECLARE_FUNCTOR2_RET(classname, returnType)
   BZ_DECLARE_FUNCTOR3_RET(classname, returnType)

   for classes whose operator() has a return type that is not what you would
   deduce from the usual C++ promotion rules (e.g., takes two doubles and
   returns a bool).

   You can then use your class in Blitz++ expressions and no temporaries will
   be generated. For example, assuming that your class is named T, and that
   A, B and C are Arrays, you can write

   T classInstance( ... );
   A = C + classInstance(B * tensor::i);
   A = C + classInstance(tensor::i, tensor::j)

   It also works for member functions:
    
   BZ_DECLARE_MEMBER_FUNCTION(classname, funcname)
   BZ_DECLARE_MEMBER_FUNCTION2(classname, funcname)
   BZ_DECLARE_MEMBER_FUNCTION3(classname, funcname)
    
   or
    
   BZ_DECLARE_MEMBER_FUNCTION_RET(classname, funcname, returnType)
   BZ_DECLARE_MEMBER_FUNCTION2_RET(classname, funcname, returnType)
   BZ_DECLARE_MEMBER_FUNCTION3_RET(classname, funcname, returnType)

   allows you to write stuff like
    
   A = C + classInstance.funcname(B * tensor::i);
   A = C + classInstance.funcname(tensor::i, tensor::j)
    
   All the member functions to be applied must be declared const.
     
   There is also some support for classes where the source code is not
   available or not to be tampered with.  For example,
     
   A = C + applyFunctor(classInstance, B * tensor::i);
   A = C + applyFunctor(classInstance, tensor::i, tensor::j);
    
   This approach does not work for arbitrary member functions.  The
   class must be a proper functor with an operator().  

*/

#ifndef BZ_ARRAY_FUNCTOREXPR_H
#define BZ_ARRAY_FUNCTOREXPR_H

#ifndef BZ_ARRAY_H
 #error <blitz/array/functorExpr.h> must be included via <blitz/array.h>
#endif

#include <blitz/prettyprint.h>
#include <blitz/shapecheck.h>
#include <blitz/tinyvec.h>

BZ_NAMESPACE(blitz)

template<typename P_functor, typename P_expr, typename P_result>
class _bz_FunctorExpr {
public:
    typedef P_functor T_functor;
    typedef P_expr T_expr;
    typedef _bz_typename T_expr::T_numtype T_numtype1;
    typedef P_result T_numtype;
    typedef T_expr    T_ctorArg1;
    typedef int       T_ctorArg2;    // dummy
    typedef int       T_ctorArg3;    // dummy
  typedef _bz_FunctorExpr<P_functor, _bz_typename P_expr::T_range_result, 
			  P_result> T_range_result;

    static const int 
        numArrayOperands = T_expr::numArrayOperands,
	numIndexPlaceholders = T_expr::numIndexPlaceholders,
	rank = T_expr::rank;
    
    _bz_FunctorExpr(const _bz_FunctorExpr<P_functor,P_expr,P_result>& a)
        : f_(a.f_), iter_(a.iter_)
    { }
    
    _bz_FunctorExpr(BZ_ETPARM(T_functor) f, BZ_ETPARM(T_expr) a)
        : f_(f), iter_(a)
    { }

    _bz_FunctorExpr(BZ_ETPARM(T_functor) f, _bz_typename T_expr::T_ctorArg1 a)
        : f_(f), iter_(a)
    { }

#if BZ_TEMPLATE_CTOR_DOESNT_CAUSE_HAVOC
    template<typename T1>
    explicit _bz_FunctorExpr(BZ_ETPARM(T_functor) f, BZ_ETPARM(T1) a)
        : f_(f), iter_(a)
    { }
#endif

    T_numtype operator*() const
    { return f_(*iter_); }

#ifdef BZ_ARRAY_EXPR_PASS_INDEX_BY_VALUE
    template<int N_rank>
    T_numtype operator()(TinyVector<int,N_rank> i) const
    { return f_(iter_(i)); }
#else
    template<int N_rank>
    T_numtype operator()(const TinyVector<int,N_rank>& i) const
    { return f_(iter_(i)); }
#endif

  T_range_result operator()(RectDomain<rank> d) const
  {
    return T_range_result(f_, iter_(d));
  }

    int ascending(const int rank) const { return iter_.ascending(rank); }
    int ordering(const int rank)  const { return iter_.ordering(rank);  }
    int lbound(const int rank)    const { return iter_.lbound(rank);    }
    int ubound(const int rank)    const { return iter_.ubound(rank);    }
    RectDomain<rank> domain() const { return iter_.domain(); }

    void push(const int position) { iter_.push(position); }

    void pop(const int position)  { iter_.pop(position); }

    void advance() { iter_.advance(); }

    void advance(const int n) { iter_.advance(n); }

    void loadStride(const int rank) { iter_.loadStride(rank); }

    bool isUnitStride(const int rank) const { return iter_.isUnitStride(rank); }

    void advanceUnitStride() { iter_.advanceUnitStride(); }
  
    bool canCollapse(const int outerLoopRank, const int innerLoopRank) const
    { 
        return iter_.canCollapse(outerLoopRank, innerLoopRank); 
    }

    T_numtype operator[](const int i) const
    { return f_(iter_[i]); }

    T_numtype fastRead(const int i) const
    { return f_(iter_.fastRead(i)); }

    // this is needed for the stencil expression fastRead to work
    void _bz_offsetData(size_t i)
    { iter_._bz_offsetData(i); }

    // and these are needed for stencil expression shift to work
    void _bz_offsetData(size_t offset, int dim)
    { iter_._bz_offsetData(offset, dim);}
  
    void _bz_offsetData(size_t offset1, int dim1, size_t offset2, int dim2)
    { iter_._bz_offsetData(offset1, dim1, offset2, dim2);}

    int suggestStride(const int rank) const
    { return iter_.suggestStride(rank); }

    bool isStride(const int rank,const int stride) const
    { return iter_.isStride(rank,stride); }

    void prettyPrint(BZ_STD_SCOPE(string) &str, 
        prettyPrintFormat& format) const
    {
        str += BZ_DEBUG_TEMPLATE_AS_STRING_LITERAL(T_functor);
        str += "(";
        iter_.prettyPrint(str, format);
        str += ")";
    }

    template<typename T_shape>
    bool shapeCheck(const T_shape& shape) const
    { return iter_.shapeCheck(shape); }

    template<int N_rank>
    void moveTo(const TinyVector<int,N_rank>& i)
    {
        iter_.moveTo(i);
    }

    T_numtype shift(int offset, int dim) const
    {
      return f_(iter_.shift(offset, dim));
    }

    T_numtype shift(int offset1, int dim1,int offset2, int dim2) const
    {
      return f_(iter_.shift(offset1, dim1, offset2, dim2));
    }

  // sliceinfo for expressions
  template<typename T1, typename T2 = nilArraySection, 
	   class T3 = nilArraySection, typename T4 = nilArraySection, 
	   class T5 = nilArraySection, typename T6 = nilArraySection, 
	   class T7 = nilArraySection, typename T8 = nilArraySection, 
	   class T9 = nilArraySection, typename T10 = nilArraySection, 
	   class T11 = nilArraySection>
  class SliceInfo {
  public:
    typedef typename T_expr::template SliceInfo<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>::T_slice T_slice1;
    typedef _bz_FunctorExpr<T_functor, T_slice1, T_numtype> T_slice;
};

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
        typename T7, typename T8, typename T9, typename T10, typename T11>
    typename SliceInfo<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11>::T_slice
    operator()(T1 r1, T2 r2, T3 r3, T4 r4, T5 r5, T6 r6, T7 r7, T8 r8, T9 r9, T10 r10, T11 r11) const
    {
      return typename SliceInfo<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11>::T_slice
	(f_,iter_(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11));
    }

protected:
    _bz_FunctorExpr() { }

    T_functor f_;
    T_expr iter_;
};

template<typename P_functor, typename P_expr1, typename P_expr2, typename P_result>
class _bz_FunctorExpr2 
{
public:
    typedef P_functor T_functor;
    typedef P_expr1 T_expr1;
    typedef P_expr2 T_expr2;
    typedef _bz_typename T_expr1::T_numtype T_numtype1;
    typedef _bz_typename T_expr2::T_numtype T_numtype2;
    typedef P_result T_numtype;
    typedef T_expr1 T_ctorArg1;
    typedef T_expr1 T_ctorArg2;
    typedef int T_ctorArg3;  // dummy
  typedef _bz_FunctorExpr2<P_functor, 
			  _bz_typename P_expr1::T_range_result, 
			  _bz_typename P_expr2::T_range_result, 
			  P_result> T_range_result;

    static const int 
        numArrayOperands = T_expr1::numArrayOperands
                         + T_expr2::numArrayOperands,
	numIndexPlaceholders = T_expr1::numIndexPlaceholders
	                     + T_expr2::numIndexPlaceholders,
	rank = T_expr1::rank > T_expr2::rank
             ? T_expr1::rank : T_expr2::rank;
  
    _bz_FunctorExpr2(const _bz_FunctorExpr2<P_functor, P_expr1, P_expr2,
        P_result>& a) 
        : f_(a.f_), iter1_(a.iter1_), iter2_(a.iter2_)
    { }

    _bz_FunctorExpr2(BZ_ETPARM(T_functor) f, BZ_ETPARM(T_expr1) a,
        BZ_ETPARM(T_expr2) b)
        : f_(f), iter1_(a), iter2_(b)
    { }

    template<typename T1, typename T2>
    _bz_FunctorExpr2(BZ_ETPARM(T_functor) f, BZ_ETPARM(T1) a, BZ_ETPARM(T2) b) 
        : f_(f), iter1_(a), iter2_(b)
    { }
  
    T_numtype operator*() const
    { return f_(*iter1_, *iter2_); }

#ifdef BZ_ARRAY_EXPR_PASS_INDEX_BY_VALUE
    template<int N_rank>
    T_numtype operator()(TinyVector<int, N_rank> i) const
    { return f_(iter1_(i), iter2_(i)); }
#else
    template<int N_rank>
    T_numtype operator()(const TinyVector<int, N_rank>& i) const
    { return f_(iter1_(i), iter2_(i)); }
#endif

  T_range_result operator()(RectDomain<rank> d) const
  {
    return T_range_result(f_, iter1_(d), iter2_(d));
  }

    int ascending(const int rank) const {
        return bounds::compute_ascending(rank, iter1_.ascending(rank),
            iter2_.ascending(rank));
    }

    int ordering(const int rank) const {
        return bounds::compute_ordering(rank, iter1_.ordering(rank),
            iter2_.ordering(rank));
    }
  
    int lbound(const int rank) const { 
        return bounds::compute_lbound(rank, iter1_.lbound(rank),
            iter2_.lbound(rank));
    }
  
    int ubound(const int rank) const {
        return bounds::compute_ubound(rank, iter1_.ubound(rank),
            iter2_.ubound(rank));
    }

  // defer calculation to lbound/ubound
  RectDomain<rank> domain() const 
  { 
    TinyVector<int, rank> lb, ub;
    for(int r=0; r<rank; ++r) {
      lb[r]=lbound(r); ub[r]=ubound(r); 
    }
    return RectDomain<rank>(lb,ub);
  }
  
    void push(const int position) { 
        iter1_.push(position); 
        iter2_.push(position);
    }
  
    void pop(const int position) { 
        iter1_.pop(position); 
        iter2_.pop(position);
    }
  
    void advance() { 
        iter1_.advance(); 
        iter2_.advance();
    }
  
    void advance(const int n) {
        iter1_.advance(n);
        iter2_.advance(n);
    }
  
    void loadStride(const int rank) {
        iter1_.loadStride(rank); 
        iter2_.loadStride(rank);
    }
  
    bool isUnitStride(const int rank) const
    { return iter1_.isUnitStride(rank) && iter2_.isUnitStride(rank); }
  
    void advanceUnitStride() { 
        iter1_.advanceUnitStride(); 
        iter2_.advanceUnitStride();
    }
  
    bool canCollapse(const int outerLoopRank,const int innerLoopRank) const
    { 
        return iter1_.canCollapse(outerLoopRank, innerLoopRank)
            && iter2_.canCollapse(outerLoopRank, innerLoopRank);
    } 

    T_numtype operator[](const int i) const
    { return f_(iter1_[i], iter2_[i]); }

    T_numtype fastRead(const int i) const
    { return f_(iter1_.fastRead(i), iter2_.fastRead(i)); }

    // this is needed for the stencil expression fastRead to work
    void _bz_offsetData(size_t i)
    { iter1_._bz_offsetData(i); iter2_._bz_offsetData(i); }

    // and these are needed for stencil expression shift to work
    void _bz_offsetData(size_t offset, int dim)
    { 
      iter1_._bz_offsetData(offset, dim);
      iter2_._bz_offsetData(offset, dim);
    }
  
    void _bz_offsetData(size_t offset1, int dim1, size_t offset2, int dim2)
    { 
      iter1_._bz_offsetData(offset1, dim1, offset2, dim2);
      iter2_._bz_offsetData(offset1, dim1, offset2, dim2);
    }

    int suggestStride(const int rank) const
    {
        int stride1 = iter1_.suggestStride(rank);
        int stride2 = iter2_.suggestStride(rank);
        return ( stride1>stride2 ? stride1 : stride2 );
    }
  
    bool isStride(const int rank,const int stride) const
    {
        return iter1_.isStride(rank,stride) && iter2_.isStride(rank,stride);
    }
  
    void prettyPrint(BZ_STD_SCOPE(string) &str, 
        prettyPrintFormat& format) const
    {
        str += BZ_DEBUG_TEMPLATE_AS_STRING_LITERAL(T_functor);
        str += "(";
        iter1_.prettyPrint(str, format);
        str += ",";
        iter2_.prettyPrint(str, format);
        str += ")";
    }

    template<int N_rank>
    void moveTo(const TinyVector<int,N_rank>& i)
    {
        iter1_.moveTo(i);
        iter2_.moveTo(i);
    }

    T_numtype shift(int offset, int dim) const
    {
      return f_(iter1_.shift(offset, dim),iter2_.shift(offset, dim));
    }

    T_numtype shift(int offset1, int dim1,int offset2, int dim2) const
    {
      return f_(iter1_.shift(offset1, dim1, offset2, dim2),
		iter2_.shift(offset1, dim1, offset2, dim2));
    }
  
    template<typename T_shape>
    bool shapeCheck(const T_shape& shape) const
    { return iter1_.shapeCheck(shape) && iter2_.shapeCheck(shape); }
  
  // sliceinfo for expressions
  template<typename T1, typename T2 = nilArraySection, 
	   class T3 = nilArraySection, typename T4 = nilArraySection, 
	   class T5 = nilArraySection, typename T6 = nilArraySection, 
	   class T7 = nilArraySection, typename T8 = nilArraySection, 
	   class T9 = nilArraySection, typename T10 = nilArraySection, 
	   class T11 = nilArraySection>
  class SliceInfo {
  public:
    typedef typename T_expr1::template SliceInfo<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>::T_slice T_slice1;
    typedef typename T_expr2::template SliceInfo<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>::T_slice T_slice2;
    typedef _bz_FunctorExpr2<T_functor, T_slice1, T_slice2, T_numtype> T_slice;
};

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
        typename T7, typename T8, typename T9, typename T10, typename T11>
    typename SliceInfo<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11>::T_slice
    operator()(T1 r1, T2 r2, T3 r3, T4 r4, T5 r5, T6 r6, T7 r7, T8 r8, T9 r9, T10 r10, T11 r11) const
    {
      return typename SliceInfo<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11>::T_slice
	(f_,iter1_(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11),
	 iter2_(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11));
    }

protected:
    _bz_FunctorExpr2() { }

    T_functor f_;
    T_expr1 iter1_;
    T_expr2 iter2_;
};

template<typename P_functor, typename P_expr1, typename P_expr2, typename P_expr3,
    class P_result>
class _bz_FunctorExpr3
{
public:
    typedef P_functor T_functor;
    typedef P_expr1 T_expr1;
    typedef P_expr2 T_expr2;
    typedef P_expr3 T_expr3;
    typedef _bz_typename T_expr1::T_numtype T_numtype1;
    typedef _bz_typename T_expr2::T_numtype T_numtype2;
    typedef _bz_typename T_expr3::T_numtype T_numtype3;
    typedef P_result T_numtype;
    typedef T_expr1 T_ctorArg1;
    typedef T_expr2 T_ctorArg2;
    typedef T_expr3 T_ctorArg3;
  typedef _bz_FunctorExpr3<P_functor, 
			  _bz_typename P_expr1::T_range_result, 
			  _bz_typename P_expr2::T_range_result, 
			  _bz_typename P_expr3::T_range_result, 
			  P_result> T_range_result;

    static const int 
        numArrayOperands = T_expr1::numArrayOperands
                         + T_expr2::numArrayOperands
                         + T_expr3::numArrayOperands,
	numIndexPlaceholders = T_expr1::numIndexPlaceholders
	                     + T_expr2::numIndexPlaceholders
	                     + T_expr3::numIndexPlaceholders,
	rank12 = T_expr1::rank > T_expr2::rank
	       ? T_expr1::rank : T_expr2::rank,
	rank = rank12 > T_expr3::rank ? rank12 : T_expr3::rank;
  
    _bz_FunctorExpr3(const _bz_FunctorExpr3<P_functor, P_expr1, P_expr2,
        P_expr3, P_result>& a) 
        : f_(a.f_), iter1_(a.iter1_), iter2_(a.iter2_), iter3_(a.iter3_)
    { }

    _bz_FunctorExpr3(BZ_ETPARM(T_functor) f, BZ_ETPARM(T_expr1) a,
        BZ_ETPARM(T_expr2) b, BZ_ETPARM(T_expr3) c)
        : f_(f), iter1_(a), iter2_(b), iter3_(c)
    { }

    template<typename T1, typename T2, typename T3>
    _bz_FunctorExpr3(BZ_ETPARM(T_functor) f, BZ_ETPARM(T1) a, BZ_ETPARM(T2) b,
        BZ_ETPARM(T3) c) 
        : f_(f), iter1_(a), iter2_(b), iter3_(c)
    { }
  
    T_numtype operator*() const
    { return f_(*iter1_, *iter2_, *iter3_); }

#ifdef BZ_ARRAY_EXPR_PASS_INDEX_BY_VALUE
    template<int N_rank>
    T_numtype operator()(TinyVector<int, N_rank> i) const
    { return f_(iter1_(i), iter2_(i), iter3_(i)); }
#else
    template<int N_rank>
    T_numtype operator()(const TinyVector<int, N_rank>& i) const
    { return f_(iter1_(i), iter2_(i), iter3_(i)); }
#endif

  T_range_result operator()(RectDomain<rank> d) const
  {
    return T_range_result(f_, iter1_(d), iter2_(d), iter3_(d));
  }

    int ascending(const int rank) const {
        return bounds::compute_ascending(rank, iter1_.ascending(rank),
            bounds::compute_ascending(rank, iter2_.ascending(rank),
            iter3_.ascending(rank)));
    }

    int ordering(const int rank) const {
        return bounds::compute_ordering(rank, iter1_.ordering(rank),
            bounds::compute_ordering(rank, iter2_.ordering(rank),
	    iter3_.ordering(rank)));
    }
  
    int lbound(const int rank) const {
        return bounds::compute_lbound(rank, iter1_.lbound(rank),
            bounds::compute_lbound(rank, iter2_.lbound(rank),
	    iter3_.lbound(rank)));
    }
  
    int ubound(const int rank) const {
        return bounds::compute_ubound(rank, iter1_.ubound(rank),
            bounds::compute_ubound(rank, iter2_.ubound(rank),
	    iter3_.ubound(rank)));
    }
  
  // defer calculation to lbound/ubound
  RectDomain<rank> domain() const 
  { 
    TinyVector<int, rank> lb, ub;
    for(int r=0; r<rank; ++r) {
      lb[r]=lbound(r); ub[r]=ubound(r); 
    }
    return RectDomain<rank>(lb,ub);
  }
  
    void push(const int position) { 
        iter1_.push(position); 
        iter2_.push(position);
        iter3_.push(position);
    }
  
    void pop(const int position) { 
        iter1_.pop(position); 
        iter2_.pop(position);
        iter3_.pop(position);
    }
  
    void advance() { 
        iter1_.advance(); 
        iter2_.advance();
        iter3_.advance();
    }
  
    void advance(const int n) {
        iter1_.advance(n);
        iter2_.advance(n);
        iter3_.advance(n);
    }
  
    void loadStride(const int rank) { 
        iter1_.loadStride(rank); 
        iter2_.loadStride(rank);
        iter3_.loadStride(rank);
    }
  
    bool isUnitStride(const int rank) const {
        return iter1_.isUnitStride(rank) && iter2_.isUnitStride(rank)
            && iter3_.isUnitStride(rank);
    }
  
    void advanceUnitStride() { 
        iter1_.advanceUnitStride(); 
        iter2_.advanceUnitStride();
        iter3_.advanceUnitStride();
    }
  
    bool canCollapse(const int outerLoopRank,const int innerLoopRank) const { 
        return iter1_.canCollapse(outerLoopRank, innerLoopRank)
            && iter2_.canCollapse(outerLoopRank, innerLoopRank)
            && iter3_.canCollapse(outerLoopRank, innerLoopRank);
    } 

    T_numtype operator[](const int i) const
    { return f_(iter1_[i], iter2_[i], iter3_[i]); }

    T_numtype fastRead(const int i) const
    { return f_(iter1_.fastRead(i), iter2_.fastRead(i), iter3_.fastRead(i)); }

    // this is needed for the stencil expression fastRead to work
    void _bz_offsetData(size_t i)
    { iter1_._bz_offsetData(i); iter2_._bz_offsetData(i); iter3_._bz_offsetData(i); }

    // and these are needed for stencil expression shift to work
    void _bz_offsetData(size_t offset, int dim)
    { 
      iter1_._bz_offsetData(offset, dim);
      iter2_._bz_offsetData(offset, dim);
      iter3_._bz_offsetData(offset, dim);
    }
  
    void _bz_offsetData(size_t offset1, int dim1, size_t offset2, int dim2)
    {
      iter1_._bz_offsetData(offset1, dim1, offset2, dim2);
      iter2_._bz_offsetData(offset1, dim1, offset2, dim2);
      iter3_._bz_offsetData(offset1, dim1, offset2, dim2);
    }

    int suggestStride(const int rank) const {
        int stride1 = iter1_.suggestStride(rank);
        int stride2 = iter2_.suggestStride(rank);
        int stride3 = iter3_.suggestStride(rank);
	return ( stride1 > (stride2 = (stride2>stride3 ? stride2 : stride3)) ?
            stride1 : stride2 );
    }
  
    bool isStride(const int rank,const int stride) const {
        return iter1_.isStride(rank,stride) && iter2_.isStride(rank,stride)
            && iter3_.isStride(rank,stride);
    }
  
    void prettyPrint(BZ_STD_SCOPE(string) &str, 
        prettyPrintFormat& format) const
    {
        str += BZ_DEBUG_TEMPLATE_AS_STRING_LITERAL(T_functor);
        str += "(";
        iter1_.prettyPrint(str, format);
        str += ",";
        iter2_.prettyPrint(str, format);
        str += ",";
        iter3_.prettyPrint(str, format);
        str += ")";
    }

    template<int N_rank>
    void moveTo(const TinyVector<int,N_rank>& i)
    {
        iter1_.moveTo(i);
        iter2_.moveTo(i);
        iter3_.moveTo(i);
    }
  
    T_numtype shift(int offset, int dim) const
    {
      return f_(iter1_.shift(offset, dim),
		iter2_.shift(offset, dim),
		iter3_.shift(offset, dim));
    }

    T_numtype shift(int offset1, int dim1,int offset2, int dim2) const
    {
      return f_(iter1_.shift(offset1, dim1, offset2, dim2),
		iter2_.shift(offset1, dim1, offset2, dim2),
		iter3_.shift(offset1, dim1, offset2, dim2));
    }
  
    template<typename T_shape>
    bool shapeCheck(const T_shape& shape) const
    {
        return iter1_.shapeCheck(shape) && iter2_.shapeCheck(shape)
            && iter3_.shapeCheck(shape);
    }

  // sliceinfo for expressions
  template<typename T1, typename T2 = nilArraySection, 
	   class T3 = nilArraySection, typename T4 = nilArraySection, 
	   class T5 = nilArraySection, typename T6 = nilArraySection, 
	   class T7 = nilArraySection, typename T8 = nilArraySection, 
	   class T9 = nilArraySection, typename T10 = nilArraySection, 
	   class T11 = nilArraySection>
  class SliceInfo {
  public:
    typedef typename T_expr1::template SliceInfo<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>::T_slice T_slice1;
    typedef typename T_expr2::template SliceInfo<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>::T_slice T_slice2;
    typedef typename T_expr3::template SliceInfo<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>::T_slice T_slice3;
    typedef _bz_FunctorExpr3<T_functor, T_slice1, T_slice2, T_slice3, T_numtype> T_slice;
};

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
        typename T7, typename T8, typename T9, typename T10, typename T11>
    typename SliceInfo<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11>::T_slice
    operator()(T1 r1, T2 r2, T3 r3, T4 r4, T5 r5, T6 r6, T7 r7, T8 r8, T9 r9, T10 r10, T11 r11) const
    {
      return typename SliceInfo<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11>::T_slice
	(f_,iter1_(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11),
	 iter2_(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11),
	 iter3_(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11));
    }
  
protected:
    _bz_FunctorExpr3() { }

    T_functor f_;
    T_expr1 iter1_;
    T_expr2 iter2_;
    T_expr3 iter3_;
};

template<typename P_functor, typename P_expr>
_bz_inline_et
_bz_ArrayExpr<_bz_FunctorExpr<P_functor, _bz_typename asExpr<P_expr>::T_expr,
    _bz_typename asExpr<P_expr>::T_expr::T_numtype> >
applyFunctor(const P_functor& f, const ETBase<P_expr>& a)
{
    typedef _bz_FunctorExpr<P_functor,
        _bz_typename asExpr<P_expr>::T_expr,
        _bz_typename asExpr<P_expr>::T_expr::T_numtype> f1;
    return _bz_ArrayExpr<f1>(f, a.unwrap());
}

template<typename P_functor, typename P_expr1, typename P_expr2>
_bz_inline_et
_bz_ArrayExpr<_bz_FunctorExpr2<P_functor,
    _bz_typename asExpr<P_expr1>::T_expr,
    _bz_typename asExpr<P_expr2>::T_expr,
    BZ_PROMOTE(_bz_typename asExpr<P_expr1>::T_expr::T_numtype,
               _bz_typename asExpr<P_expr2>::T_expr::T_numtype)> >
applyFunctor(const P_functor& f,
    const ETBase<P_expr1>& a, const ETBase<P_expr2>& b)
{
    typedef _bz_FunctorExpr2<P_functor,
        _bz_typename asExpr<P_expr1>::T_expr,
        _bz_typename asExpr<P_expr2>::T_expr,
        BZ_PROMOTE(_bz_typename asExpr<P_expr1>::T_expr::T_numtype,
                   _bz_typename asExpr<P_expr2>::T_expr::T_numtype)> f2;
    return _bz_ArrayExpr<f2>(f, a.unwrap(), b.unwrap());
}

template<typename P_functor, typename P_expr1, typename P_expr2, typename P_expr3>
_bz_inline_et
_bz_ArrayExpr<_bz_FunctorExpr3<P_functor,
    _bz_typename asExpr<P_expr1>::T_expr,
    _bz_typename asExpr<P_expr2>::T_expr, 
    _bz_typename asExpr<P_expr3>::T_expr,
    BZ_PROMOTE(_bz_typename asExpr<P_expr1>::T_expr::T_numtype,
	       BZ_PROMOTE(_bz_typename asExpr<P_expr2>::T_expr::T_numtype,
	                  _bz_typename asExpr<P_expr3>::T_expr::T_numtype))> >
applyFunctor(const P_functor& f, const ETBase<P_expr1>& a,
    const ETBase<P_expr2>& b, const ETBase<P_expr3>& c)
{
    typedef _bz_FunctorExpr3<P_functor,
        _bz_typename asExpr<P_expr1>::T_expr,
        _bz_typename asExpr<P_expr2>::T_expr,
        _bz_typename asExpr<P_expr3>::T_expr,
        BZ_PROMOTE(_bz_typename asExpr<P_expr1>::T_expr::T_numtype,
	    BZ_PROMOTE(_bz_typename asExpr<P_expr2>::T_expr::T_numtype,
	               _bz_typename asExpr<P_expr3>::T_expr::T_numtype))> f3;
    return _bz_ArrayExpr<f3>(f, a.unwrap(), b.unwrap(), c.unwrap());
}

BZ_NAMESPACE_END // End of stuff in namespace


#define _BZ_MAKE_FUNCTOR(classname, funcname)                             \
class _bz_Functor ## classname ## funcname                                \
{                                                                         \
public:                                                                   \
    _bz_Functor ## classname ## funcname (const classname& c)             \
        : c_(c)                                                           \
    { }                                                                   \
    template<typename T_numtype1>                                            \
    inline T_numtype1 operator()(T_numtype1 x) const                      \
    { return c_.funcname(x); }                                            \
private:                                                                  \
    const classname& c_;                                                  \
};

#define _BZ_MAKE_FUNCTOR2(classname, funcname)                            \
class _bz_Functor ## classname ## funcname                                \
{                                                                         \
public:                                                                   \
    _bz_Functor ## classname ## funcname (const classname& c)             \
        : c_(c)                                                           \
    { }                                                                   \
    template<typename T_numtype1, typename T_numtype2>                          \
    inline BZ_PROMOTE(T_numtype1, T_numtype2)                             \
    operator()(T_numtype1 x, T_numtype2 y) const                          \
    { return c_.funcname(x,y); }                                          \
private:                                                                  \
    const classname& c_;                                                  \
};

#define _BZ_MAKE_FUNCTOR3(classname, funcname)                            \
class _bz_Functor ## classname ## funcname                                \
{                                                                         \
public:                                                                   \
    _bz_Functor ## classname ## funcname (const classname& c)             \
        : c_(c)                                                           \
    { }                                                                   \
    template<typename T_numtype1, typename T_numtype2, typename T_numtype3>        \
    inline BZ_PROMOTE(BZ_PROMOTE(T_numtype1, T_numtype2), T_numtype3)     \
    operator()(T_numtype1 x, T_numtype2 y, T_numtype3 z) const            \
    { return c_.funcname(x,y,z); }                                        \
private:                                                                  \
    const classname& c_;                                                  \
};


#define _BZ_MAKE_FUNCTOR_RET(classname, funcname, ret)                    \
class _bz_Functor ## classname ## funcname                                \
{                                                                         \
public:                                                                   \
    _bz_Functor ## classname ## funcname (const classname& c)             \
        : c_(c)                                                           \
    { }                                                                   \
    template<typename T_numtype1>                                            \
    inline ret operator()(T_numtype1 x) const                             \
    { return c_.funcname(x); }                                            \
private:                                                                  \
    const classname& c_;                                                  \
};

#define _BZ_MAKE_FUNCTOR2_RET(classname, funcname, ret)                   \
class _bz_Functor ## classname ## funcname                                \
{                                                                         \
public:                                                                   \
    _bz_Functor ## classname ## funcname (const classname& c)             \
        : c_(c)                                                           \
    { }                                                                   \
    template<typename T_numtype1, typename T_numtype2>                          \
    inline ret operator()(T_numtype1 x, T_numtype2 y) const               \
    { return c_.funcname(x,y); }                                          \
private:                                                                  \
    const classname& c_;                                                  \
};

#define _BZ_MAKE_FUNCTOR3_RET(classname, funcname, ret)                   \
class _bz_Functor ## classname ## funcname                                \
{                                                                         \
public:                                                                   \
    _bz_Functor ## classname ## funcname (const classname& c)             \
        : c_(c)                                                           \
    { }                                                                   \
    template<typename T_numtype1, typename T_numtype2, typename T_numtype3>        \
    inline ret operator()(T_numtype1 x, T_numtype2 y, T_numtype3 z) const \
    { return c_.funcname(x,y,z); }                                        \
private:                                                                  \
    const classname& c_;                                                  \
};


#define BZ_DECLARE_FUNCTOR(classname)                                     \
template<typename P_expr>                                                 \
BZ_BLITZ_SCOPE(_bz_ArrayExpr)<BZ_BLITZ_SCOPE(_bz_FunctorExpr)<            \
    classname,                                                            \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr>::T_expr,                  \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr>::T_expr::T_numtype> >     \
operator()(const BZ_BLITZ_SCOPE(ETBase)<P_expr>& a) const                 \
{                                                                         \
    return BZ_BLITZ_SCOPE(_bz_ArrayExpr)<                                 \
        BZ_BLITZ_SCOPE(_bz_FunctorExpr)<classname,                        \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr>::T_expr,              \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr>::T_expr::T_numtype> > \
        (*this, a.unwrap());                                              \
}

#define BZ_DECLARE_FUNCTOR2(classname)                                    \
template<typename P_expr1, typename P_expr2>                              \
BZ_BLITZ_SCOPE(_bz_ArrayExpr)<BZ_BLITZ_SCOPE(_bz_FunctorExpr2)<           \
    classname,                                                            \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,                 \
    BZ_PROMOTE(_bz_typename                                               \
               BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr::T_numtype,        \
               _bz_typename                                               \
               BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr::T_numtype)> >     \
operator()(const BZ_BLITZ_SCOPE(ETBase)<P_expr1>& a,                      \
           const BZ_BLITZ_SCOPE(ETBase)<P_expr2>& b) const                \
{                                                                         \
    return BZ_BLITZ_SCOPE(_bz_ArrayExpr)<                                 \
        BZ_BLITZ_SCOPE(_bz_FunctorExpr2)<classname,                       \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,             \
        BZ_PROMOTE(_bz_typename                                           \
                   BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr::T_numtype,    \
                   _bz_typename                                           \
                   BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr::T_numtype)> > \
        (*this, a.unwrap(), b.unwrap());                                  \
}

#define BZ_DECLARE_FUNCTOR3(classname)                                    \
template<typename P_expr1, typename P_expr2, typename P_expr3>            \
BZ_BLITZ_SCOPE(_bz_ArrayExpr)<BZ_BLITZ_SCOPE(_bz_FunctorExpr3)<           \
    classname,                                                            \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr3>::T_expr,                 \
    BZ_PROMOTE(_bz_typename                                               \
               BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr::T_numtype,        \
    BZ_PROMOTE(_bz_typename                                               \
               BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr::T_numtype,        \
               _bz_typename                                               \
               BZ_BLITZ_SCOPE(asExpr)<P_expr3>::T_expr::T_numtype))> >    \
operator()(const BZ_BLITZ_SCOPE(ETBase)<P_expr1>& a,                      \
           const BZ_BLITZ_SCOPE(ETBase)<P_expr2>& b,                      \
           const BZ_BLITZ_SCOPE(ETBase)<P_expr3>& c) const                \
{                                                                         \
    return BZ_BLITZ_SCOPE(_bz_ArrayExpr)<                                 \
        BZ_BLITZ_SCOPE(_bz_FunctorExpr3)<classname,                       \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr3>::T_expr,             \
        BZ_PROMOTE(_bz_typename                                           \
                   BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr::T_numtype,    \
        BZ_PROMOTE(_bz_typename                                           \
                   BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr::T_numtype,    \
                   _bz_typename                                           \
                   BZ_BLITZ_SCOPE(asExpr)<P_expr3>::T_expr::T_numtype))> >\
        (*this, a.unwrap(), b.unwrap(), c.unwrap());                      \
}


#define BZ_DECLARE_FUNCTOR_RET(classname, ret)                            \
template<typename P_expr>                                                 \
BZ_BLITZ_SCOPE(_bz_ArrayExpr)<BZ_BLITZ_SCOPE(_bz_FunctorExpr)<            \
    classname,                                                            \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr>::T_expr,                  \
    ret> >                                                                \
operator()(const BZ_BLITZ_SCOPE(ETBase)<P_expr>& a) const                 \
{                                                                         \
    return BZ_BLITZ_SCOPE(_bz_ArrayExpr)<                                 \
        BZ_BLITZ_SCOPE(_bz_FunctorExpr)<classname,                        \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr>::T_expr,              \
        ret> >                                                            \
        (*this, a.unwrap());                                              \
}

#define BZ_DECLARE_FUNCTOR2_RET(classname, ret)                           \
template<typename P_expr1, typename P_expr2>                              \
BZ_BLITZ_SCOPE(_bz_ArrayExpr)<BZ_BLITZ_SCOPE(_bz_FunctorExpr2)<           \
    classname,                                                            \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,                 \
    ret> >                                                                \
operator()(const BZ_BLITZ_SCOPE(ETBase)<P_expr1>& a,                      \
           const BZ_BLITZ_SCOPE(ETBase)<P_expr2>& b) const                \
{                                                                         \
    return BZ_BLITZ_SCOPE(_bz_ArrayExpr)<                                 \
        BZ_BLITZ_SCOPE(_bz_FunctorExpr2)<classname,                       \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,             \
        ret> >                                                            \
        (*this, a.unwrap(), b.unwrap());                                  \
}

#define BZ_DECLARE_FUNCTOR3_RET(classname, ret)                           \
template<typename P_expr1, typename P_expr2, typename P_expr3>            \
BZ_BLITZ_SCOPE(_bz_ArrayExpr)<BZ_BLITZ_SCOPE(_bz_FunctorExpr3)<           \
    classname,                                                            \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr3>::T_expr,                 \
    ret> >                                                                \
operator()(const BZ_BLITZ_SCOPE(ETBase)<P_expr1>& a,                      \
           const BZ_BLITZ_SCOPE(ETBase)<P_expr2>& b,                      \
           const BZ_BLITZ_SCOPE(ETBase)<P_expr3>& c) const                \
{                                                                         \
    return BZ_BLITZ_SCOPE(_bz_ArrayExpr)<                                 \
        BZ_BLITZ_SCOPE(_bz_FunctorExpr3)<classname,                       \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr3>::T_expr,             \
        ret> >                                                            \
        (*this, a.unwrap(), b.unwrap(), c.unwrap());                      \
}


#define BZ_DECLARE_MEMBER_FUNCTION(classname, funcname)                   \
_BZ_MAKE_FUNCTOR(classname, funcname)                                     \
template<typename P_expr>                                                 \
BZ_BLITZ_SCOPE(_bz_ArrayExpr)<BZ_BLITZ_SCOPE(_bz_FunctorExpr)<            \
    _bz_Functor ## classname ## funcname,                                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr>::T_expr,                  \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr>::T_expr::T_numtype> >     \
funcname(const BZ_BLITZ_SCOPE(ETBase)<P_expr>& a) const                   \
{                                                                         \
    return BZ_BLITZ_SCOPE(_bz_ArrayExpr)<                                 \
        BZ_BLITZ_SCOPE(_bz_FunctorExpr)<                                  \
        _bz_Functor ## classname ## funcname,                             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr>::T_expr,              \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr>::T_expr::T_numtype> > \
        (*this, a.unwrap());                                              \
}

#define BZ_DECLARE_MEMBER_FUNCTION2(classname, funcname)                  \
_BZ_MAKE_FUNCTOR2(classname, funcname)                                    \
template<typename P_expr1, typename P_expr2>                              \
BZ_BLITZ_SCOPE(_bz_ArrayExpr)<BZ_BLITZ_SCOPE(_bz_FunctorExpr2)<           \
    _bz_Functor ## classname ## funcname,                                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,                 \
    BZ_PROMOTE(_bz_typename                                               \
               BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr::T_numtype,        \
               _bz_typename                                               \
               BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr::T_numtype)> >     \
funcname(const BZ_BLITZ_SCOPE(ETBase)<P_expr1>& a,                        \
         const BZ_BLITZ_SCOPE(ETBase)<P_expr2>& b) const                  \
{                                                                         \
    return BZ_BLITZ_SCOPE(_bz_ArrayExpr)<                                 \
        BZ_BLITZ_SCOPE(_bz_FunctorExpr2)<                                 \
        _bz_Functor ## classname ## funcname,                             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,             \
        BZ_PROMOTE(_bz_typename                                           \
                   BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr::T_numtype,    \
                   _bz_typename                                           \
                   BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr::T_numtype)> > \
        (*this, a.unwrap(), b.unwrap());                                  \
}

#define BZ_DECLARE_MEMBER_FUNCTION3(classname, funcname)                  \
_BZ_MAKE_FUNCTOR3(classname, funcname)                                    \
template<typename P_expr1, typename P_expr2, typename P_expr3>            \
BZ_BLITZ_SCOPE(_bz_ArrayExpr)<BZ_BLITZ_SCOPE(_bz_FunctorExpr3)<           \
    _bz_Functor ## classname ## funcname,                                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr3>::T_expr,                 \
    BZ_PROMOTE(_bz_typename                                               \
               BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr::T_numtype,        \
    BZ_PROMOTE(_bz_typename                                               \
               BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr::T_numtype,        \
               _bz_typename                                               \
               BZ_BLITZ_SCOPE(asExpr)<P_expr3>::T_expr::T_numtype))> >    \
funcname(const BZ_BLITZ_SCOPE(ETBase)<P_expr1>& a,                        \
         const BZ_BLITZ_SCOPE(ETBase)<P_expr2>& b,                        \
         const BZ_BLITZ_SCOPE(ETBase)<P_expr3>& c) const                  \
{                                                                         \
    return BZ_BLITZ_SCOPE(_bz_ArrayExpr)<                                 \
        BZ_BLITZ_SCOPE(_bz_FunctorExpr3)<                                 \
        _bz_Functor ## classname ## funcname,                             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr3>::T_expr,             \
        BZ_PROMOTE(_bz_typename                                           \
                   BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr::T_numtype,    \
        BZ_PROMOTE(_bz_typename                                           \
                   BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr::T_numtype,    \
                   _bz_typename                                           \
                   BZ_BLITZ_SCOPE(asExpr)<P_expr3>::T_expr::T_numtype))> >\
        (*this, a.unwrap(), b.unwrap(), c.unwrap());                      \
}


#define BZ_DECLARE_MEMBER_FUNCTION_RET(classname, funcname, ret)          \
_BZ_MAKE_FUNCTOR_RET(classname, funcname, ret)                            \
template<typename P_expr>                                                 \
BZ_BLITZ_SCOPE(_bz_ArrayExpr)<BZ_BLITZ_SCOPE(_bz_FunctorExpr)<            \
    _bz_Functor ## classname ## funcname,                                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr>::T_expr,                  \
    ret> >                                                                \
funcname(const BZ_BLITZ_SCOPE(ETBase)<P_expr>& a) const                   \
{                                                                         \
    return BZ_BLITZ_SCOPE(_bz_ArrayExpr)<                                 \
        BZ_BLITZ_SCOPE(_bz_FunctorExpr)<                                  \
        _bz_Functor ## classname ## funcname,                             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr>::T_expr,              \
        ret> >                                                            \
        (*this, a.unwrap());                                              \
}

#define BZ_DECLARE_MEMBER_FUNCTION2_RET(classname, funcname, ret)         \
_BZ_MAKE_FUNCTOR2_RET(classname, funcname, ret)                           \
template<typename P_expr1, typename P_expr2>                              \
BZ_BLITZ_SCOPE(_bz_ArrayExpr)<BZ_BLITZ_SCOPE(_bz_FunctorExpr2)<           \
    _bz_Functor ## classname ## funcname,                                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,                 \
    ret> >                                                                \
funcname(const BZ_BLITZ_SCOPE(ETBase)<P_expr1>& a,                        \
         const BZ_BLITZ_SCOPE(ETBase)<P_expr2>& b) const                  \
{                                                                         \
    return BZ_BLITZ_SCOPE(_bz_ArrayExpr)<                                 \
        BZ_BLITZ_SCOPE(_bz_FunctorExpr2)<                                 \
        _bz_Functor ## classname ## funcname,                             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,             \
        ret> >                                                            \
        (*this, a.unwrap(), b.unwrap());                                  \
}

#define BZ_DECLARE_MEMBER_FUNCTION3_RET(classname, funcname, ret)         \
_BZ_MAKE_FUNCTOR3_RET(classname, funcname, ret)                           \
template<typename P_expr1, typename P_expr2, typename P_expr3>            \
BZ_BLITZ_SCOPE(_bz_ArrayExpr)<BZ_BLITZ_SCOPE(_bz_FunctorExpr3)<           \
    _bz_Functor ## classname ## funcname,                                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,                 \
    _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr3>::T_expr,                 \
    ret> >                                                                \
funcname(const BZ_BLITZ_SCOPE(ETBase)<P_expr1>& a,                        \
         const BZ_BLITZ_SCOPE(ETBase)<P_expr2>& b,                        \
         const BZ_BLITZ_SCOPE(ETBase)<P_expr3>& c) const                  \
{                                                                         \
    return BZ_BLITZ_SCOPE(_bz_ArrayExpr)<                                 \
        BZ_BLITZ_SCOPE(_bz_FunctorExpr3)<                                 \
        _bz_Functor ## classname ## funcname,                             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr1>::T_expr,             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr2>::T_expr,             \
        _bz_typename BZ_BLITZ_SCOPE(asExpr)<P_expr3>::T_expr,             \
        ret> >                                                            \
        (*this, a.unwrap(), b.unwrap(), c.unwrap());                      \
}



#endif // BZ_ARRAY_FUNCTOREXPR_H

