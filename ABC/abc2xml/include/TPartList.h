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

#ifndef __TPartList__
#define __TPartList__

#include "exports.h"
#include "smartpointer.h"
#include "TPartGroup.h"
#include "TScorePart.h"
#include "TScoreVisitor.h"

namespace MusicXML 
{

/*!
\brief Identifies the different musical parts in a movement. 
	
	There must be at least one score-part, combined as
	desired with part-group elements that indicate braces
	and brackets. Parts are ordered from top to bottom in
	a score based on the order in which they appear in the
	part-list.
\n
	TPartList corresponds to the MusicXML \e part-list element 
	as defined in score.dtd.
*/
class EXP TPartList : public visitable, public smartable {

    public:
        EXP friend SMARTP<TPartList> newPartList();
          
        virtual void accept(TScoreVisitor& visitor);

        long add(const SScorePart& part);
        long add(const SPartGroup& part);

        vvector<SPartListElement>& parts() 		{ return fPartsList; }
        
    protected:
        TPartList() {}
        virtual ~TPartList() {}
    private:
        vvector<SPartListElement> fPartsList;  
};
typedef SMARTP<TPartList> SPartList;

} // namespace MusicXML


#endif
