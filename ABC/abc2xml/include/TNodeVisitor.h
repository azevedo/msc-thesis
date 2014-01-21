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

#ifndef __TNodeVisitor__
#define __TNodeVisitor__

#include <vector>
#include "smartclasses.h"
#include "smartpointer.h"

using namespace std;

namespace MusicXML 
{
/*!
\addtogroup visitors
@{
*/

/*!
\brief A node Visitor of MusicXML scores. 

    A node Visitor of MusicXML scores. Nodes visitor are to be used with a 
    \p TRolledVisitor or \p TUnrolledVisitor that does the actual score traversing. 
    A node Visitor can implement \p visiteEnter and/or \p visiteLeave methods 
    for each type of node. 
	For terminal nodes, only the visiteEnter method can be implemented.
*/

class TNodeVisitor {

    public:
		TNodeVisitor() {}
		virtual ~TNodeVisitor() {}

		virtual void visiteEnter ( SAccidental& elt ) {}
		virtual void visiteEnter ( SAccidentalMark& elt ) {}
		virtual void visiteEnter ( SArpeggiate& elt ) {}
		virtual void visiteEnter ( SNonArpeggiate& elt ) {}
		virtual void visiteEnter ( SArticulationElement& elt ) {}
		virtual void visiteEnter ( SArticulations& elt ) {}
		virtual void visiteEnter ( SAttributes& elt ) {}
		virtual void visiteEnter ( SBackup& elt ) {}
		virtual void visiteEnter ( SBarline& elt ) {}
		virtual void visiteEnter ( SBass& elt ) {}
		virtual void visiteEnter ( SBeam& elt ) {}
		virtual void visiteEnter ( SBeatRepeat& elt ) {}
		virtual void visiteEnter ( SBendWithBar& elt ) {}
		virtual void visiteEnter ( SBend& elt ) {}
		virtual void visiteEnter ( SBracket& elt ) {}
		virtual void visiteEnter ( SChord& elt ) {}
		virtual void visiteEnter ( SClef& elt ) {}
		virtual void visiteEnter ( SCoda& elt ) {}
		virtual void visiteEnter ( SCreator& elt ) {}
		virtual void visiteEnter ( SDashes& elt ) {}
		virtual void visiteEnter ( SDegree& elt ) {}
		virtual void visiteEnter ( SDirection& elt ) {}
		virtual void visiteEnter ( SDirectionType& elt ) {}
		virtual void visiteEnter ( SDirective& elt ) {}
		virtual void visiteEnter ( SDynamic& elt ) {}
		virtual void visiteEnter ( SEncoding& elt ) {}
		virtual void visiteEnter ( SEnding& elt ) {}
		virtual void visiteEnter ( SExpression& elt ) {}
		virtual void visiteEnter ( SFermata& elt ) {}
		virtual void visiteEnter ( SFeature& elt ) {}
		virtual void visiteEnter ( SFigure& elt ) {}
		virtual void visiteEnter ( SFiguredBass& elt ) {}
		virtual void visiteEnter ( SFingering& elt ) {}
		virtual void visiteEnter ( SForward& elt ) {}
		virtual void visiteEnter ( SFrame& elt ) {}
		virtual void visiteEnter ( SFrameNote& elt ) {}
		virtual void visiteEnter ( SFunction& elt ) {}
		virtual void visiteEnter ( SGlissando& elt ) {}
		virtual void visiteEnter ( SGraceNote& elt ) {}
		virtual void visiteEnter ( SGraphNote& elt ) {}
		virtual void visiteEnter ( SGrouping& elt ) {}
		virtual void visiteEnter ( SHammerPull& elt ) {}
		virtual void visiteEnter ( SHarmonic& elt ) {}
		virtual void visiteEnter ( SHarmony& elt ) {}
		virtual void visiteEnter ( SHarmonyChord& elt ) {}
		virtual void visiteEnter ( SIdentification& elt ) {}
		virtual void visiteEnter ( SKey& elt ) {}
		virtual void visiteEnter ( SLyric& elt ) {}
		virtual void visiteEnter ( SMeasure& elt ) {}
		virtual void visiteEnter ( SMeasureRepeat& elt ) {}
		virtual void visiteEnter ( SMeasureStyle& elt ) {}
		virtual void visiteEnter ( SMeasureStyleElement& elt ) {}
		virtual void visiteEnter ( SMetronome& elt ) {}
		virtual void visiteEnter ( SMidiDevice& elt ) {}
		virtual void visiteEnter ( SMidiInstrument& elt ) {}
		virtual void visiteEnter ( SMiscField& elt ) {}
		virtual void visiteEnter ( SMiscId& elt ) {}
		virtual void visiteEnter ( SMordentOrnament& elt ) {}
		virtual void visiteEnter ( SMultipleLyricPart& elt ) {}
		virtual void visiteEnter ( SMultipleRest& elt ) {}
		virtual void visiteEnter ( SNotation& elt ) {}
		virtual void visiteEnter ( SNote& elt ) {}
		virtual void visiteEnter ( SNoteHead& elt ) {}
		virtual void visiteEnter ( SOctaveShift& elt ) {}
		virtual void visiteEnter ( SOrnament& elt ) {}
		virtual void visiteEnter ( SOrnaments& elt ) {}
		virtual void visiteEnter ( SOtherArticulation& elt ) {}
		virtual void visiteEnter ( SOtherDirection& elt ) {}
		virtual void visiteEnter ( SOtherDirections& elt ) {}
		virtual void visiteEnter ( SOtherNotation& elt ) {}
		virtual void visiteEnter ( SOtherOrnament& elt ) {}
		virtual void visiteEnter ( SPWMeasure& elt ) {}
		virtual void visiteEnter ( SPWPart& elt ) {}
		virtual void visiteEnter ( SPart& elt ) {}
		virtual void visiteEnter ( SPartGroup& elt ) {}
		virtual void visiteEnter ( SPartList& elt ) {}
		virtual void visiteEnter ( SPedal& elt ) {}
		virtual void visiteEnter ( SPitch& elt ) {}
		virtual void visiteEnter ( SPrint& elt ) {}
		virtual void visiteEnter ( SRehearsal& elt ) {}
		virtual void visiteEnter ( SRepeat& elt ) {}
		virtual void visiteEnter ( SRest& elt ) {}
		virtual void visiteEnter ( SRights& elt ) {}
		virtual void visiteEnter ( SRoot& elt ) {}
		virtual void visiteEnter ( SScoreHeader& elt ) {}
		virtual void visiteEnter ( SScoreInstrument& elt ) {}
		virtual void visiteEnter ( SScorePart& elt ) {}
		virtual void visiteEnter ( SScorePartwise& elt ) {}
		virtual void visiteEnter ( SScoreTimewise& elt ) {}
		virtual void visiteEnter ( SSegno& elt ) {}
		virtual void visiteEnter ( SSlide& elt ) {}
		virtual void visiteEnter ( SSlash& elt ) {}
		virtual void visiteEnter ( SSlur& elt ) {}
		virtual void visiteEnter ( SStaffDetails& elt ) {}
		virtual void visiteEnter ( SStaffTuning& elt ) {}
		virtual void visiteEnter ( SSupport& elt ) {}
		virtual void visiteEnter ( SSound& elt ) {}
		virtual void visiteEnter ( SStrongAccent& elt ) {}
		virtual void visiteEnter ( STechnical& elt ) {}
		virtual void visiteEnter ( STechnicals& elt ) {}
		virtual void visiteEnter ( STWMeasure& elt ) {}
		virtual void visiteEnter ( STWPart& elt ) {}
		virtual void visiteEnter ( STie& elt ) {}
		virtual void visiteEnter ( STimeModification& elt ) {}
		virtual void visiteEnter ( STimeSign& elt ) {}
		virtual void visiteEnter ( STranspose& elt ) {}
		virtual void visiteEnter ( STuplet& elt ) {}
		virtual void visiteEnter ( STupletDesc& elt ) {}
		virtual void visiteEnter ( SUnpitched& elt ) {}
		virtual void visiteEnter ( SWavyLine& elt ) {}
		virtual void visiteEnter ( SWedge& elt ) {}
		virtual void visiteEnter ( SWords& elt ) {}
		virtual void visiteEnter ( SWork& elt ) {}
                
    	virtual void visiteLeave ( SArticulationElement& elt ) {}
		virtual void visiteLeave ( SAccidentalMark& elt ) {}
		virtual void visiteLeave ( SArpeggiate& elt ) {}
		virtual void visiteLeave ( SNonArpeggiate& elt ) {}
		virtual void visiteLeave ( SArticulations& elt ) {}
		virtual void visiteLeave ( SAttributes& elt ) {}
		virtual void visiteLeave ( SBarline& elt ) {}
		virtual void visiteLeave ( SBeam& elt ) {}
		virtual void visiteLeave ( SBeatRepeat& elt ) {}
		virtual void visiteLeave ( SBendWithBar& elt ) {}
		virtual void visiteLeave ( SBend& elt ) {}
		virtual void visiteLeave ( SBracket& elt ) {}
		virtual void visiteLeave ( SChord& elt ) {}
		virtual void visiteLeave ( SCoda& elt ) {}
		virtual void visiteLeave ( SDashes& elt ) {}
		virtual void visiteLeave ( SDirection& elt ) {}
		virtual void visiteLeave ( SDirectionType& elt ) {}
		virtual void visiteLeave ( SDynamic& elt ) {}
		virtual void visiteLeave ( SFermata& elt ) {}
		virtual void visiteLeave ( SFigure& elt ) {}
		virtual void visiteLeave ( SFiguredBass& elt ) {}
		virtual void visiteLeave ( SForward& elt ) {}
		virtual void visiteLeave ( SFrame& elt ) {}
		virtual void visiteLeave ( SFrameNote& elt ) {}
		virtual void visiteLeave ( SGlissando& elt ) {}
		virtual void visiteLeave ( SGraceNote& elt ) {}
		virtual void visiteLeave ( SGraphNote& elt ) {}
		virtual void visiteLeave ( SGrouping& elt ) {}
		virtual void visiteLeave ( SHammerPull& elt ) {}
		virtual void visiteLeave ( SHarmonic& elt ) {}
		virtual void visiteLeave ( SHarmony& elt ) {}
		virtual void visiteLeave ( SHarmonyChord& elt ) {}
		virtual void visiteLeave ( SIdentification& elt ) {}
		virtual void visiteLeave ( SKey& elt ) {}
		virtual void visiteLeave ( SLyric& elt ) {}
		virtual void visiteLeave ( SMeasure& elt ) {}
		virtual void visiteLeave ( SMeasureRepeat& elt ) {}
		virtual void visiteLeave ( SMeasureStyle& elt ) {}
		virtual void visiteLeave ( SMeasureStyleElement& elt ) {}
		virtual void visiteLeave ( SMetronome& elt ) {}
		virtual void visiteLeave ( SMidiDevice& elt ) {}
		virtual void visiteLeave ( SMidiInstrument& elt ) {}
		virtual void visiteLeave ( SMiscField& elt ) {}
		virtual void visiteLeave ( SMiscId& elt ) {}
		virtual void visiteLeave ( SMordentOrnament& elt ) {}
		virtual void visiteLeave ( SMultipleLyricPart& elt ) {}
		virtual void visiteLeave ( SMultipleRest& elt ) {}
		virtual void visiteLeave ( SNotation& elt ) {}
		virtual void visiteLeave ( SNote& elt ) {}
		virtual void visiteLeave ( SOctaveShift& elt ) {}
		virtual void visiteLeave ( SOrnament& elt ) {}
		virtual void visiteLeave ( SOrnaments& elt ) {}
		virtual void visiteLeave ( SOtherArticulation& elt ) {}
		virtual void visiteLeave ( SOtherOrnament& elt ) {}
		virtual void visiteLeave ( SPWMeasure& elt ) {}
		virtual void visiteLeave ( SPWPart& elt ) {}
		virtual void visiteLeave ( SPartList& elt ) {}
		virtual void visiteLeave ( SScoreHeader& elt ) {}
		virtual void visiteLeave ( SScorePart& elt ) {}
		virtual void visiteLeave ( SScorePartwise& elt ) {}
		virtual void visiteLeave ( SScoreTimewise& elt ) {}
		virtual void visiteLeave ( SSegno& elt ) {}
		virtual void visiteLeave ( SSlide& elt ) {}
		virtual void visiteLeave ( SSlash& elt ) {}
		virtual void visiteLeave ( SSlur& elt ) {}
		virtual void visiteLeave ( SStaffDetails& elt ) {}
		virtual void visiteLeave ( SStaffTuning& elt ) {}
		virtual void visiteLeave ( SSound& elt ) {}
		virtual void visiteLeave ( SStrongAccent& elt ) {}
		virtual void visiteLeave ( STechnical& elt ) {}
		virtual void visiteLeave ( STechnicals& elt ) {}
		virtual void visiteLeave ( STWMeasure& elt ) {}
		virtual void visiteLeave ( STWPart& elt ) {}
		virtual void visiteLeave ( STie& elt ) {}
		virtual void visiteLeave ( STuplet& elt ) {}
		virtual void visiteLeave ( SWavyLine& elt ) {}
		virtual void visiteLeave ( SWedge& elt ) {}
		virtual void visiteLeave ( SWords& elt ) {}

		virtual void visiteEnter ( TOrientation* elt ) {}
		virtual void visiteEnter ( TPlacement* elt ) {}
		virtual void visiteEnter ( TPosition* elt ) {}
		virtual void visiteEnter ( TTrillSound* elt ) {}
		virtual void visiteEnter ( TBendSound* elt ) {}
		virtual void visiteEnter ( TFont* elt ) {}
		virtual void visiteEnter ( TPrintout* elt ) {}
		virtual void visiteEnter ( TEditorial* elt ) {}
		virtual void visiteEnter ( TBezier* elt ) {}
};

/*! @} */

} // namespace MusicXML


#endif
