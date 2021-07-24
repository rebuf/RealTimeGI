// Copyright (c) 2021 Ammar Herzallah
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.



#pragma once



#include "Core/Core.h"


#include <functional>
#include <vector>





// Basic Function Pointer.
template< class ...Args >
using FuncPtrType = void(*)(Args...);



// Member Function Pointer Type. 
template< class ClassType, class ...Args>
using MemFuncPtrType = void(ClassType::*)(Args...);




// Declare Delegate.
template< class ...Args >
class Delegate;




// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 
// - --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- - 



namespace Detail
{
	// Delegate Types.
	enum class EDelegateType
	{
		None,
		Static,
		Lambda,
		MemberRaw,
		MemberSmart
	};




	// DelegateBase:
	//   - Base class for different delegate types.
	//
	template<class ...Args>
	class DelegateBase
	{
		// No Copy.
		DelegateBase(const DelegateBase&) = delete;
		DelegateBase& operator=(const DelegateBase&) = delete;

	public:
		// The type of the delegate.
		EDelegateType type;

		// Default Construct.
		DelegateBase()
			: type(EDelegateType::None)
		{

		}

		// Destruct.
		virtual ~DelegateBase() { }

		// Return the delegate type.
		inline EDelegateType GetType() const { return type; }

		// Return ture if function is valid and can be invoked.
		virtual bool IsValid() const = 0;

		// Execute the delegate by invoking the function.
		virtual void Execute(Args...) = 0;

		// Return the address of the function pointer.
		virtual void GetFuncAddress(std::uintptr_t& funcAddress, std::uintptr_t& refAddress) const  = 0;

		// Delegate Equality.
		bool IsEqual(const DelegateBase& other) const
		{
			// Different types.
			if (type != other.type)
				return false;

			// If lambda then always false.
			if (type == EDelegateType::Lambda || other.type == EDelegateType::Lambda)
				return false;

			std::uintptr_t funcAddress, refAddress, otherFuncAddress, otherRefAddress;
			GetFuncAddress(funcAddress, refAddress);
			other.GetFuncAddress(otherFuncAddress, otherRefAddress);

			return refAddress == otherRefAddress && funcAddress == otherFuncAddress;
		}

	};



	// DelegateG:
	//    - Delegate for referencing general functions (Static or Lambda).
	//
	template< class ...Args >
	class DelegateG : public DelegateBase<Args...>
	{
	public:
		// Delegate Function Type.
		using DelegateFuncType = std::function<void(Args...)>;

		// Holds function this delegate execute.
		DelegateFuncType mFunc;

		// Return ture if function is valid and can be invoked.
		virtual bool IsValid() const override
		{
			return mFunc != nullptr;
		}

		// Execute the delegate by invoking the function.
		virtual void Execute(Args... args) override
		{
			mFunc(args...);
		}

		// Return the address of the function pointer.
		virtual void GetFuncAddress(std::uintptr_t& funcAddress, std::uintptr_t& refAddress) const override
		{
			FuncPtrType<Args...> func = *mFunc.target< FuncPtrType<Args...> >();
			funcAddress = *(reinterpret_cast<std::uintptr_t*>(reinterpret_cast<void*>(&func))); // This is fine because its only used to compare two addresses.
			refAddress = 0;
		}
	};


	// Create general delegate for static function pointer.
	template<class ...Args>
	static DelegateG<Args...>* CreateStatic(FuncPtrType<Args...> inFunc)
	{
		DelegateG<Args...>* del = new DelegateG<Args...>();
		del->mFunc = typename DelegateG<Args...>::DelegateFuncType(inFunc);
		del->type = EDelegateType::Static;
		return del;
	}

	// Create general delegate for lambda function pointer.
	template< class LambdaType, class ...Args >
	static DelegateG<Args...>* CreateLambda(LambdaType&& inFunc)
	{
		DelegateG<Args...>* del = new DelegateG<Args...>();
		del->mFunc = typename DelegateG<Args...>::DelegateFuncType(inFunc);
		del->type = EDelegateType::Lambda;
		return del;
	}

	// DelegateM:
	//    - Delegate for referencing member functions using raw pointers.
	//
	template< class RefType, class ClassType, class ...Args >
	class DelegateM : public DelegateBase<Args...>
	{
	public:
		// Delegate Function Type.
		using DelegateFuncType = std::function<void(ClassType&, Args...)>;

		// Freind Delegate
		friend class Delegate<Args...>;

		// Holds function this delegate execute.
		DelegateFuncType mFunc;

		// Reference to the object we want to invoke its function.
		RefType mRef;

		// Return Reference Pointer.
		inline ClassType* GetRef(ClassType* inRef) const { return inRef; }
		inline ClassType* GetRef(WeakPtr<ClassType> inRef) const { return inRef.lock().get(); }

		// Return ture if function is valid and can be invoked.
		virtual bool IsValid() const override
		{
			return mFunc != nullptr && GetRef(mRef) != nullptr;
		}

		// Execute the delegate by invoking the function.
		virtual void Execute(Args... args) override
		{
			mFunc(*GetRef(mRef), args...);
		}

		// Return the address of the function pointer.
		virtual void GetFuncAddress(std::uintptr_t& funcAddress, std::uintptr_t& refAddress) const override
		{
			MemFuncPtrType< ClassType, Args... > func = *mFunc.target< MemFuncPtrType<ClassType, Args...> >();
			funcAddress = *(reinterpret_cast<std::uintptr_t*>(reinterpret_cast<void*>(&func))); // This is fine because its only used to compare two addresses.
			refAddress = (std::uintptr_t)GetRef(mRef);
		}
	};



	// Create a delegate for memeber function pointer, that holds a raw pointer to the object.
	template<class ClassType, class ...Args>
	static DelegateM<ClassType*, ClassType, Args...>* CreateRaw(ClassType* obj, MemFuncPtrType<ClassType, Args...> inFunc)
	{
		DelegateM<ClassType*, ClassType, Args...>* del = new DelegateM<ClassType*, ClassType, Args...>();
		del->mFunc = typename DelegateM<ClassType*, ClassType, Args...>::DelegateFuncType(inFunc);
		del->type = EDelegateType::MemberRaw;
		del->mRef = obj;
		return del;
	}

	// Create a delegate for memeber function pointer, that holds a smart pointer to the object.
	template<class ClassType, class ...Args>
	static DelegateM<WeakPtr<ClassType>, ClassType, Args...>* CreateSmart(Ptr<ClassType> obj, MemFuncPtrType<ClassType, Args...> inFunc)
	{
		DelegateM<WeakPtr<ClassType>, ClassType, Args...>* del = new DelegateM<WeakPtr<ClassType>, ClassType, Args...>();
		del->mFunc = typename DelegateM<WeakPtr<ClassType>, ClassType, Args...>::DelegateFuncType(inFunc);
		del->type = EDelegateType::MemberSmart;
		del->mRef = obj;
		return del;
	}


} // End of DetailDelegate namespace.








// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 
// - --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- - 







// Delegate:
//    - Similar to C# Delegates, you use them to reference global functions, static functions, member
//      functions, etc... and exeucte the referenced function later on when needed, for example like
//      a callback/event.
//    
//    - You can bind a single function to a delegate with the same argument by using Delegate::Bind and 
//      execute that function by calling Delegate::Execute(Arg...).
//    
//    - Encapsulate std::function objects and provide utitiles to work with it.
//    
template< class ...Args >
class Delegate
{
	// Delegate Type.
	typedef Ptr< Detail::DelegateBase<Args...> > DelegatePtr;

public:
  // Default Constructor.
  Delegate()
  {

  }

  // Copy Constructor.
  Delegate(const Delegate& Other)
  {
    mDelFunc = Other.mDelFunc;
  }

  // Copy Operator.
  Delegate& operator=(const Delegate& Other)
  {
    mDelFunc = Other.mDelFunc;
    return *this;
  }

  // Bind this Delegate to a static/global function. 
  void BindStatic(FuncPtrType<Args...> inFunc)
  {
    CHECK(inFunc != nullptr);
		mDelFunc = DelegatePtr( Detail::CreateStatic<Args...>(inFunc) );
  }

  // Bind this Delegate to a lambda function.
  template< class LambdaType >
  void BindLambda(LambdaType&& inFunc)
  {
		CHECK(inFunc != nullptr);
		mDelFunc = DelegatePtr( Detail::CreateLambda<LambdaType, Args...>(std::forward<LambdaType>(inFunc)) );
  }

  // Bind this Delegate to a member function with a raw pointer to the object.
  template< class ClassType >
  void BindMemberRaw(ClassType* inObject, MemFuncPtrType<ClassType, Args...> inFunc)
  {
    CHECK(inObject != nullptr && inFunc != nullptr);
		mDelFunc = DelegatePtr( Detail::CreateRaw<ClassType, Args...>(inObject, inFunc) );
	}

  // Bind this Delegate to a member function with a smart pointer to the object. 
  template< class ClassType >
  void BindMemberSmart(Ptr<ClassType> inObject, MemFuncPtrType<ClassType, Args...> inFunc)
  {
    CHECK(inFunc != nullptr);
		mDelFunc = DelegatePtr( Detail::CreateSmart<ClassType, Args...>(inObject, inFunc) );
	}

  // Create Delegate for a static and global functions.
  static Delegate<Args...> CreateStatic(FuncPtrType<Args...> inFunc)
  {
    Delegate<Args...> newDel;
    newDel.BindStatic(inFunc);
    return newDel;
  }

  // Create Delegate for a member function with a raw pointer to the object.
  template< class ClassType >
  static Delegate<Args...> CreateMemberRaw(ClassType* inObject, MemFuncPtrType<ClassType, Args...> inFunc)
  {
    Delegate<Args...> newDel;
    newDel.BindMemberRaw(inObject, inFunc);
    return newDel;
  }

  // Create Delegate for a member function with a smart pointer to the object.
  template< class ClassType >
  static Delegate<Args...> CreateMemberSmart(Ptr<ClassType> inObject, MemFuncPtrType<ClassType, Args...> inFunc)
  {
    Delegate<Args...> newDel;
    newDel.BindMemberSmart(inObject, inFunc);
    return newDel;
  }

  // Create Delegate for a lambda function. 
  template< class LambdaType >
  static Delegate<Args...> CreateLambda(LambdaType&& inFunc)
  {
    Delegate<Args...> newDel;
    newDel.BindLambda( std::forward<LambdaType>(inFunc) );
    return newDel;
  }

public:
  // Return ture if delegate is bounded and valid to use.
  inline bool IsValid() const
  {
    return mDelFunc != nullptr && mDelFunc->IsValid();
  }

  // Reset The Delegate to Invalid/Unbounded delegate. 
  inline void Reset()
  {
    mDelFunc.reset();
  }

  // Execute/Call bounded function.
  inline void Execute(Args... Params)
  {
    CHECK(IsValid() && "Executing Invalid Delegate.");
    mDelFunc->Execute(Params...);
  }

  // Test Delegate Equality, Two delegates are equal if they are valid, have the same delegate type,
  //        and point to the same function and object address.
	//
  // Note: lambda typed delegates will always return false on equality.
	//
  bool IsEqual(const Delegate<Args...>& other) const
  {
    // Not Valid? can't compare non-valid delegates.
    if (!IsValid() || !other.IsValid())
      return false;

    return mDelFunc->IsEqual(*other.mDelFunc);
  }

	// Return the delegate type.
	inline Detail::EDelegateType GetType() const
	{
		return mDelFunc->type;
	}


private:
  // Delegate that reference a function to be executed.
  DelegatePtr mDelFunc;
};







// GFx::MultiDelegate:
//    a Multicast Delegate that holds multiple delegates of the same arguments, used to bind multiple functions
//    to the same event.
// 
// Note: Adding delegates is not unique, you can add the same delegate multiple times.
// 
template< class ...Args >
class MultiDelegate
{
public:
	// Construct.
	MultiDelegate()
	{

	}

	// Add a Delegate. Note: you can add the same delegate multiple times. 
	void Add(const Delegate<Args...>& inDel)
	{
		mDelegates.emplace_back(inDel);
	}

	// Remove a delegate form the multicast delegate list that is equal to InDel delegate.
	// Note: Equality of a delegate is dependent on the type of the delegate.
	void Remove(const Delegate<Args...>& inDel)
	{
		CHECK(inDel.GetType() != Detail::EDelegateType::Lambda && "Can't remove Lambda, equality is always false");

		// Find an equal Delegate...
		for (auto iter = mDelegates.begin(); iter != mDelegates.end(); ++iter)
		{
			// Found?
			if ((*iter).IsEqual(inDel))
			{
				mDelegates.erase(iter);
				break;
			}
		}
	}

	// Execute/Call all added delegates.
	inline void Execute(Args... Params)
	{
		// Execute all valid delegates.
		for (auto iter = mDelegates.begin(); iter != mDelegates.end(); ++iter)
		{
			if (!(*iter).IsValid())
				continue;

			(*iter).Execute(Params...);
		}
	}

	// Add delegate for a static and global functions.
	void AddStatic(FuncPtrType<Args...> inFunc)
	{
		Add( Delegate<Args...>::CreateStatic(inFunc) );
	}

	// Remove static function delegate.
	void RemoveStatic(FuncPtrType<Args...> inFunc)
	{
		Remove( Delegate<Args...>::CreateStatic(inFunc) );
	}

	// Add delegate for a member function with a raw pointer to the object.
	template< class ClassType >
	void AddMember(ClassType* inObject, MemFuncPtrType<ClassType, Args...> inFunc)
	{
		Add( Delegate<Args...>::CreateMemberRaw(inObject, inFunc) );
	}

	// Remove member function delegate, raw pointer.
	template< class ClassType >
	void RemoveMember(ClassType* inObject, MemFuncPtrType<ClassType, Args...> inFunc)
	{
		Remove( Delegate<Args...>::CreateMemberRaw(inObject, inFunc) );
	}

	// Create delegate for a member function with a smart pointer to the object.
	template< class ClassType >
	void AddMember(Ptr<ClassType> inObject, MemFuncPtrType<ClassType, Args...> inFunc)
	{
		Add( Delegate<Args...>::CreateMemberSmart(inObject, inFunc) );
	}

	// Remove member function delegate, smart pointer.
	template< class ClassType >
	void RemoveMember(Ptr<ClassType> inObject, MemFuncPtrType<ClassType, Args...> inFunc)
	{
		Remove( Delegate<Args...>::CreateMemberSmart(inObject, inFunc) );
	}

	// Create delegate for a lambda function. 
	template< class LambdaType >
	void AddLambda(LambdaType&& inFunc)
	{
		Add( Delegate<Args...>::CreateLambda(std::forward<LambdaType>(inFunc)) );
	}

	// Clear all currently added delegates.
	void Clear()
	{
		mDelegates.clear();
	}

private:
	// list of all the delegates with the same argument added to this multicast delegate.
	std::vector< Delegate<Args...> > mDelegates;
};


