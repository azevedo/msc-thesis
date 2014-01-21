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

#ifndef __TRoutedVisitor__
#define __TRoutedVisitor__

#include "exports.h"
#include "TScoreVisitor.h"

// using namespace std;

namespace MusicXML 
{

/*!
\addtogroup visitors
@{
*/

/*!
\brief A visitor that implements a traversing of a score.

	A TRoutedVisitor provides a path for traversing of a score tree.
	Each implemented \e visite method calls \e accept() for its subclasses.
	Order of these calls is the following: 
	\li subclasses representing attributes are called first, 
	\li subclasses representing elements are called in the MusicXML defined order.
*/
class VEXP TRoutedVisitor : public TScoreVisitor {

    protected:
		TRoutedVisitor();
		virtual ~TRoutedVisitor();

    public:
		virtual void visite ( SAccidentalMark& elt );
		virtual void visite ( SArpeggiate& elt );
		virtual void visite ( SNonArpeggiate& elt );
		virtual void visite ( SArticulationElement& elt );
		virtual void visite ( SArticulations& elt );
		virtual void visite ( SAttributes& elt );
		virtual void visite ( SBarline& elt );
		virtual void visite ( SBendWithBar& elt );
		virtual void visite ( SBend& elt );
		virtual void visite ( SBracket& elt );
		virtual void visite ( SChord& elt );
		virtual void visite ( SCoda& elt );
		virtual void visite ( SDashes& elt );
		virtual void visite ( SDirection& elt );
		virtual void visite ( SDirectionType& elt );
		virtual void visite ( SDynamic& elt );
		virtual void visite ( SFermata& elt );
		virtual void visite ( SFiguredBass& elt );
		virtual void visite ( SFingering& elt );
		virtual void visite ( SFrame& elt );
		virtual void visite ( SFrameNote& elt );
		virtual void visite ( SGlissando& elt );
		virtual void visite ( SGraphNote& elt );
		virtual void visite ( SGrouping& elt );
		virtual void visite ( SHammerPull& elt );
		virtual void visite ( SHarmonic& elt );
		virtual void visite ( SHarmony& elt );
		virtual void visite ( SHarmonyChord& elt );
		virtual void visite ( SIdentification& elt );
		virtual void visite ( SLyric& elt );
		virtual void visite ( SMiscId& elt );
		virtual void visite ( SMeasureStyle& elt );
		virtual void visite ( SNotation& elt );
		virtual void visite ( SNote& elt );
		virtual void visite ( SOctaveShift& elt );
		virtual void visite ( SOrnament& elt );
		virtual void visite ( SOrnaments& elt );
		virtual void visite ( SPWMeasure& elt );
		virtual void visite ( SPWPart& elt );
		virtual void visite ( SPartList& elt );
		virtual void visite ( SRehearsal& elt );
		virtual void visite ( SScoreHeader& elt );
		virtual void visite ( SScorePart& elt );
		virtual void visite ( SScorePartwise& elt );
		virtual void visite ( SScoreTimewise& elt );
		virtual void visite ( SSegno& elt );
		virtual void visite ( SSlide& elt );
		virtual void visite ( SSlur& elt );
		virtual void visite ( SStaffDetails& elt );
		virtual void visite ( SSound& elt );
		virtual void visite ( SStrongAccent& elt );
		virtual void visite ( STechnical& elt );
		virtual void visite ( STechnicals& elt );
		virtual void visite ( STWMeasure& elt );
		virtual void visite ( STWPart& elt );
		virtual void visite ( STie& elt );
		virtual void visite ( STrillOrnament& elt );
		virtual void visite ( STuplet& elt );
		virtual void visite ( SWavyLine& elt ) {}
		virtual void visite ( SWedge& elt );
		virtual void visite ( SWords& elt );

};
typedef SMARTP<TRoutedVisitor> SRoutedVisitor;
/*! @} */

} // namespace MusicXML


#endif
