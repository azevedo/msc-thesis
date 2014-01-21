/*

  Copyright (C) 2003  Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  grame@grame.fr

*/

#ifndef __guidovisitor__
#define __guidovisitor__

#include "exports.h"
#include "smartpointer.h"

#include <string>
using namespace std;

typedef SMARTP<class guidoelement> 	Sguidoelement;
typedef SMARTP<class guidonote> 	Sguidonote;
typedef SMARTP<class guidoseq> 		Sguidoseq;
typedef SMARTP<class guidochord> 	Sguidochord;
typedef SMARTP<class guidotag> 		Sguidotag;
typedef SMARTP<class guidoparam> 	Sguidoparam;

/*!
\addtogroup guido
@{
*/

/*!
\brief an abstract generic guido visitor
*/
class guidovisitor {

    public:
		guidovisitor() {}
		virtual ~guidovisitor() {}

		virtual void visite ( Sguidoelement& elt ) = 0;
		virtual void visite ( Sguidonote& elt ) = 0;
		virtual void visite ( Sguidoseq& elt ) = 0;
		virtual void visite ( Sguidochord& elt ) = 0;
		virtual void visite ( Sguidotag& elt ) = 0;
		virtual void visite ( Sguidoparam& elt ) = 0;
};

/*!
\brief interface for visitable guido objects.
*/
class guidovisitable {
	public:
		virtual ~guidovisitable() {}
        virtual void accept(guidovisitor& visitor) = 0;
};
/*! @} */


#endif
