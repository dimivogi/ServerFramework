#ifndef		DAWN_ENGINE_GLOBAL_DEFINITIONS_HPP_
	#define	DAWN_ENGINE_GLOBAL_DEFINITIONS_HPP_



	#ifndef			NULL
		#define		NULL		0
	#endif			/* NULL */

	#ifndef			TRUE
		#define		TRUE		1
	#endif			/* TRUE */

	#ifndef			FALSE
		#define		FALSE		0
	#endif			/* FALSE */

	#ifndef			ERROR_CODE
		#define		ERROR_CODE	-1
	#endif			/* ERROR_CODE */

	#ifndef			__WFILE__
		#define		WIDEN2(x)	L ## x
		#define		WIDEN(x)	WIDEN2(x)
		#define		__WFILE__	WIDEN(__FILE__)
	#endif			/* __WFILE  */

	#ifndef			WIN32_LEAN_AND_MEAN
		#define		WIN32_LEAN_AND_MEAN
		#define		WIN64_LEAN_AND_MEAN
	#endif			/* WIN32_LEAN_AND_MEAN */
	
	#ifndef			NOMINMAX
		#define		NOMINMAX
	#endif			/* NOMINMAX */


	#ifdef			_DEBUG
		
		#ifndef		_CRTDBG_MAP_ALLOC
			#define		_CRTDBG_MAP_ALLOC
		#endif		/* _CRTDBG_MAP_ALLOC */

		#include	<stdlib.h>
		#include	<crtdbg.h>

	#endif			/* _DEBUG */


#endif		/* DAWN_ENGINE_GLOBAL_DEFINITIONS_HPP_ */