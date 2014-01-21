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

#ifndef __TScoreHeader__
#define __TScoreHeader__

#include "TIdentification.h"
#include "TPartList.h"
#include "TScoreVisitor.h"
#include "TWork.h"
#include "smartpointer.h"

namespace MusicXML 
{

/*!
\brief Contains basic score meta-data plus the part list.

	Meta-data includes work and movement name, movement-wide identification data. 
    It corresponds to the MusicXML \e score-header entity as defined in score.dtd.
*/
class EXP TScoreHeader : public visitable, public smartable {
    
    public:
        
        EXP friend SMARTP<TScoreHeader> newScoreHeader();

        virtual void accept(TScoreVisitor& visitor);

        void 	setMovementNum(const string& num);
        void 	setMovementTitle(const string& title);
        const string& 	getMovementNum()		{ return fMovementNum; }
        const string& 	getMovementTitle()		{ return fMovementTitle; }

        SWork& 			work()					{ return fWork; }
        SIdentification& identification()		{ return fIdentification; }
        SPartList& 		partList()				{ return fPartList; }

    protected:
        TScoreHeader() {}
        virtual ~TScoreHeader() {}
    private:
        SWork 			fWork;
        string 			fMovementNum;
        string 			fMovementTitle;
        SIdentification fIdentification;
        //! the part list must not be empty
        SPartList 		fPartList;
};
typedef SMARTP<TScoreHeader> SScoreHeader;

} // namespace MusicXML


#endif
