/*

  MusicXML Library
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

#ifndef __TWork__
#define __TWork__

#include "exports.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"
#include <string>

namespace MusicXML 
{

/*!
\brief Represents works and movements.
 
	Works and movements are optionally 
	identified by number and title as in MuseData. 
	The work element also may indicate a link to the opus document 
	that composes multiple movements into a collection.

\todo
	opus support is to be implemented
*/
class EXP TWork : public visitable, public smartable {
    
    public:
        EXP friend SMARTP<TWork> newWork();
	
        virtual void accept(TScoreVisitor& visitor);
   
		//! the optionnal work title
        void 	setTitle(const std::string& title);
        //! the optionnal work number
        void 	setNum(const std::string& num);

        const std::string& 	getTitle() const		{ return fTitle; }
        const std::string& 	getNum() const			{ return fNum; }

    protected:
		TWork() {}
        virtual ~TWork() {}
    private:
		std::string 	fTitle;
        std::string 	fNum;
// opus is ignored: to be implemented
//       string  fOpus;
};
typedef SMARTP<TWork> SWork;

} // namespace MusicXML


#endif
