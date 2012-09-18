#include	"conditionVariable.hpp"



namespace	DawnEngine
{

	namespace	Concurrency
	{

		//	 The assignment operator. It has no functionality. Exists only to prevent bit-wise copying.
		ConditionVariable	ConditionVariable::operator=( const ConditionVariable& )	{ return *this; };


		//	 The default constructor.
		ConditionVariable::ConditionVariable()											{ _initialise(); };
		//	 The copy-constructor. Since copy construction is disabled, it creates a NEW variable.
		ConditionVariable::ConditionVariable( const ConditionVariable& )				{ _initialise(); };
		//	The destructor.
		ConditionVariable::~ConditionVariable()											{ _deinitialise(); };

	}	/* Concurrency */

}	/* DawnEngine */