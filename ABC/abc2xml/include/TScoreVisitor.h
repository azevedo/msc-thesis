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

#ifndef __TScoreVisitor__
#define __TScoreVisitor__

#include <vector>
#include "smartclasses.h"
#include "smartpointer.h"

namespace MusicXML 
{
/*!
\addtogroup visitors
@{
*/

/*!
\brief A base class to implement MusicXML score visitors.

	TScoreVisitor implements the concept of visitors as defined in
	<em>Design Patterns - Elements of Reusable Object-Oriented Software</em>
	E. Gamma, R.Helm, R.Johnson, J.Vlissides - Addison-Wesley 1999
*/
class TScoreVisitor {

    public:
		TScoreVisitor() {}
		virtual ~TScoreVisitor() {}

		virtual void visite ( SAccidental& elt ) {}
		virtual void visite ( SAccidentalMark& elt ) {}
		virtual void visite ( SArpeggiate& elt ) {}
		virtual void visite ( SNonArpeggiate& elt ) {}
		virtual void visite ( SArticulationElement& elt ) {}
		virtual void visite ( SArticulations& elt ) {}
		virtual void visite ( SAttributes& elt ) {}
		virtual void visite ( SBackup& elt ) {}
		virtual void visite ( SBarline& elt ) {}
		virtual void visite ( SBass& elt ) {}
		virtual void visite ( SBeam& elt ) {}
		virtual void visite ( SBeatRepeat& elt ) {}
		virtual void visite ( SBendWithBar& elt ) {}
		virtual void visite ( SBend& elt ) {}
		virtual void visite ( SBracket& elt ) {}
		virtual void visite ( SChord& elt ) {}
		virtual void visite ( SClef& elt ) {}
		virtual void visite ( SCoda& elt ) {}
		virtual void visite ( SCreator& elt ) {}
		virtual void visite ( SDashes& elt ) {}
		virtual void visite ( SDegree& elt ) {}
		virtual void visite ( SDirection& elt ) {}
		virtual void visite ( SDirectionType& elt ) {}
		virtual void visite ( SDirective& elt ) {}
		virtual void visite ( SDynamic& elt ) {}
		virtual void visite ( SEncoding& elt ) {}
		virtual void visite ( SEnding& elt ) {}
		virtual void visite ( SExpression& elt ) {}
		virtual void visite ( SFermata& elt ) {}
		virtual void visite ( SFigure& elt ) {}
		virtual void visite ( SFeature& elt ) {}
		virtual void visite ( SFiguredBass& elt ) {}
		virtual void visite ( SFingering& elt ) {}
		virtual void visite ( SFootnote& elt ) {}
		virtual void visite ( SFrame& elt ) {}
		virtual void visite ( SFrameNote& elt ) {}
		virtual void visite ( SFunction& elt ) {}
		virtual void visite ( SForward& elt ) {}
		virtual void visite ( SGlissando& elt ) {}
		virtual void visite ( SGraceNote& elt ) {}
		virtual void visite ( SGraphNote& elt ) {}
		virtual void visite ( SGrouping& elt ) {}
		virtual void visite ( SHammerPull& elt ) {}
		virtual void visite ( SHarmonic& elt ) {}
		virtual void visite ( SHarmony& elt ) {}
		virtual void visite ( SHarmonyChord& elt ) {}
		virtual void visite ( SIdentification& elt ) {}
		virtual void visite ( SKey& elt ) {}
		virtual void visite ( SLevel& elt ) {}
		virtual void visite ( SLyric& elt ) {}
		virtual void visite ( SMeasure& elt ) {}
		virtual void visite ( SMeasureRepeat& elt ) {}
		virtual void visite ( SMeasureStyle& elt ) {}
		virtual void visite ( SMeasureStyleElement& elt ) {}
		virtual void visite ( SMetronome& elt ) {}
		virtual void visite ( SMidiDevice& elt ) {}
		virtual void visite ( SMidiInstrument& elt ) {}
		virtual void visite ( SMiscField& elt ) {}
		virtual void visite ( SMiscId& elt ) {}
		virtual void visite ( SMordentOrnament& elt ) {}
		virtual void visite ( SMultipleLyricPart& elt ) {}
		virtual void visite ( SMultipleRest& elt ) {}
		virtual void visite ( SNotation& elt ) {}
		virtual void visite ( SNote& elt ) {}
		virtual void visite ( SNoteHead& elt ) {}
		virtual void visite ( SOctaveShift& elt ) {}
		virtual void visite ( SOrnament& elt ) {}
		virtual void visite ( SOrnaments& elt ) {}
		virtual void visite ( SOtherArticulation& elt ) {}
		virtual void visite ( SOtherDirection& elt ) {}
		virtual void visite ( SOtherDirections& elt ) {}
		virtual void visite ( SOtherNotation& elt ) {}
		virtual void visite ( SOtherOrnament& elt ) {}
		virtual void visite ( SPWMeasure& elt ) {}
		virtual void visite ( SPWPart& elt ) {}
		virtual void visite ( SPart& elt ) {}
		virtual void visite ( SPartGroup& elt ) {}
		virtual void visite ( SPartList& elt ) {}
		virtual void visite ( SPCData& elt ) {}
		virtual void visite ( SPedal& elt ) {}
		virtual void visite ( SPitch& elt ) {}
		virtual void visite ( SPrint& elt ) {}
		virtual void visite ( SRehearsal& elt ) {}
		virtual void visite ( SRepeat& elt ) {}
		virtual void visite ( SRest& elt ) {}
		virtual void visite ( SRights& elt ) {}
		virtual void visite ( SRoot& elt ) {}
		virtual void visite ( SScoreHeader& elt ) {}
		virtual void visite ( SScoreInstrument& elt ) {}
		virtual void visite ( SScorePart& elt ) {}
		virtual void visite ( SScorePartwise& elt ) {}
		virtual void visite ( SScoreTimewise& elt ) {}
		virtual void visite ( SSegno& elt ) {}
		virtual void visite ( SSlash& elt ) {}
		virtual void visite ( SSlide& elt ) {}
		virtual void visite ( SSlur& elt ) {}
		virtual void visite ( SStaffDetails& elt ) {}
		virtual void visite ( SStaffTuning& elt ) {}
		virtual void visite ( SSupport& elt ) {}
		virtual void visite ( SSound& elt ) {}
		virtual void visite ( SStrongAccent& elt ) {}
		virtual void visite ( STechnical& elt ) {}
		virtual void visite ( STechnicals& elt ) {}
		virtual void visite ( STWMeasure& elt ) {}
		virtual void visite ( STWPart& elt ) {}
		virtual void visite ( STie& elt ) {}
		virtual void visite ( STimeModification& elt ) {}
		virtual void visite ( STimeSign& elt ) {}
		virtual void visite ( STranspose& elt ) {}
		virtual void visite ( STrillOrnament& elt ) {}
		virtual void visite ( STuplet& elt ) {}
		virtual void visite ( STupletDesc& elt ) {}
		virtual void visite ( SUnpitched& elt ) {}
		virtual void visite ( SVoice& elt ) {}
		virtual void visite ( SWavyLine& elt ) {}
		virtual void visite ( SWedge& elt ) {}
		virtual void visite ( SWords& elt ) {}
		virtual void visite ( SWork& elt ) {}

		virtual void visite ( TOrientation* elt ) {}
		virtual void visite ( TPlacement* elt ) {}
		virtual void visite ( TPosition* elt ) {}
		virtual void visite ( TTrillSound* elt ) {}
		virtual void visite ( TBendSound* elt ) {}
		virtual void visite ( TFont* elt ) {}
		virtual void visite ( TPrintout* elt ) {}
		virtual void visite ( TEditorial* elt ) {}
		virtual void visite ( TBezier* elt ) {}
};

/*!
\brief interface for visitable objects.
*/
class visitable {
	public:
		virtual ~visitable(){}
        virtual void accept(TScoreVisitor& visitor) = 0;
};
typedef SMARTP<visitable> Svisitable;

/*!
\brief A visitable vector

	relay the accept method to all the vector elements.
*/
template <typename T> 
class vvector: public std::vector<T>, public visitable {
    
    public:
        vvector() {}
        virtual ~vvector(){}
    
        void accept(TScoreVisitor& visitor) {
			typename std::vector<T>::iterator iter;
			for (iter = this->begin(); iter != this->end(); iter++) 
				(*iter)->accept(visitor);
		}
};

/*! @} */


} // namespace MusicXML


#endif
