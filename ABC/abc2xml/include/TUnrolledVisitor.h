/*
  Copyright © Grame 2003

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

    Grame Research Laboratory, 9, rue du Garet 69001 Lyon - France
    grame@grame.fr

*/

#ifndef __TUnrolledVisitor__
#define __TUnrolledVisitor__

#include "exports.h"
#include "TRolledVisitor.h"
#include "LibMusicXML.h"
#include <list>

using namespace std;

namespace MusicXML 
{

/*!
\addtogroup visitors
@{
*/

/*!
\brief A section is a pair (beginning measure, end measure)
*/

typedef pair<long,long> TSection;

/*!
\brief A visitor that "unroll" the score

  To "unroll" the score, structural information denoted by \b repeat, 
  \b ending, \b coda and \b segno signs is interpreted and the score
  is visited similarly to a musician that would play the score ie:
  for example a section repeated twice is visited twice.

\todo Management of multiple jump and sound \b ForwardRepeat attribute.
*/

class VEXP TUnrolledVisitor : public TRolledVisitor {

    private:
		
		vvector<SPWMeasure>::const_iterator fCodaIt;
		vvector<SPWMeasure>::const_iterator fSegnoIt;
		vvector<SPWMeasure>::const_iterator fFineIt;
		vvector<SPWMeasure>::const_iterator fBeginMeasureIt;	// Beginning of the section before the repeated section
		vvector<SPWMeasure>::const_iterator fCurMeasureIt;		// End of the section before the repeated section
		vvector<SPWMeasure>::const_iterator fFirstMeasureIt;	// First mesure of the part
		
		list<TSection> fSectionList;
		 
        bool IsRepeatType (SPWMeasure& elt, int type);
 		bool IsEndingType (SPWMeasure& elt, int type);
        long GetEndingNum (SPWMeasure& elt);
		bool IsRepeatType (SMusicData& musicData, int type);
		bool IsEndingType (SMusicData& musicData, int type);
		long GetRepeatTimes (SPWMeasure& elt);
	
		void RepeatSection (vvector<SPWMeasure>::const_iterator begin, vvector<SPWMeasure>::const_iterator end);
		void AddSection (vvector<SPWMeasure>::const_iterator begin, vvector<SPWMeasure>::const_iterator end);
 
    public:
    
        TUnrolledVisitor(TNodeVisitor* visitor);
		virtual ~TUnrolledVisitor();
		
		virtual void visite ( SPWMeasure& elt );
		virtual void visite ( SPWPart& elt );
  		virtual void visite ( SSound& elt );
		
		const list<TSection>& GetSectionList() {return fSectionList;}
};

/*! @} */

} // namespace MusicXML


#endif
