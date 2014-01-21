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

#ifndef __TRolledVisitor__
#define __TRolledVisitor__

#include "exports.h"
#include "TRoutedVisitor.h"
#include "TNodeVisitor.h"

using namespace std;

namespace MusicXML 
{

/*!
\addtogroup visitors
@{
*/

/*!
\brief A visitor that transport a \b TNodeVisitor along a score.

For non-terminal nodes, The TRolledVisitor calls:
	- the TNodeVisitor::visiteEnter method
	- TRoutedVisitor::visite on the node to visite sub-nodes
	- the TNodeVisitor::visiteLeave method

For terminal nodes, only the TNodeVisitor::visiteEnter method is called.
*/

class VEXP TRolledVisitor : public TRoutedVisitor {

    protected:
        TNodeVisitor* fVisitor;
         
    public:
        TRolledVisitor(TNodeVisitor * visitor);
		virtual ~TRolledVisitor();
        
        virtual void visite ( SAccidental& elt ); //
		virtual void visite ( SArticulationElement& elt );
		virtual void visite ( SArticulations& elt );
		virtual void visite ( SAttributes& elt );
        virtual void visite ( SBackup& elt );  //
		virtual void visite ( SBarline& elt );
        virtual void visite ( SBeam& elt );   //
		virtual void visite ( SBracket& elt );
		virtual void visite ( SChord& elt );
        virtual void visite ( SClef& elt ); //
		virtual void visite ( SCoda& elt );
        virtual void visite ( SCreator& elt ); //
		virtual void visite ( SDashes& elt );
		virtual void visite ( SDirection& elt );
		virtual void visite ( SDirectionType& elt );
        virtual void visite ( SDirective& elt ); //
		virtual void visite ( SDynamic& elt );
        virtual void visite ( SEncoding& elt ); //
		virtual void visite ( SEnding& elt ); //
		virtual void visite ( SExpression& elt ); //
		virtual void visite ( SFermata& elt );
        virtual void visite ( SForward& elt ); //
        virtual void visite ( SGraceNote& elt ); //
		virtual void visite ( SGraphNote& elt );
		virtual void visite ( SIdentification& elt );
        virtual void visite ( SKey& elt ); // 
		virtual void visite ( SMeasure& elt ); //
		virtual void visite ( SMetronome& elt ); //
		virtual void visite ( SMidiDevice& elt ); //
		virtual void visite ( SMidiInstrument& elt ); //
		virtual void visite ( SNotation& elt );
		virtual void visite ( SNote& elt );
        virtual void visite ( SNoteHead& elt ); //
		virtual void visite ( SOctaveShift& elt );
		virtual void visite ( SPWMeasure& elt );
		virtual void visite ( SPWPart& elt );
        virtual void visite ( SPart& elt ); //
        virtual void visite ( SPartGroup& elt ); //
		virtual void visite ( SPartList& elt );
        virtual void visite ( SPitch& elt ); //
        virtual void visite ( SRepeat& elt ); //
		virtual void visite ( SRest& elt ); //
		virtual void visite ( SRights& elt ); //
		virtual void visite ( SScoreHeader& elt );
        virtual void visite ( SScoreInstrument& elt ); //
		virtual void visite ( SScorePart& elt );
		virtual void visite ( SScorePartwise& elt );
		virtual void visite ( SScoreTimewise& elt );
		virtual void visite ( SSegno& elt );
		virtual void visite ( SSlur& elt );
		virtual void visite ( SSound& elt );
		virtual void visite ( SStrongAccent& elt );
		virtual void visite ( STWMeasure& elt );
		virtual void visite ( STWPart& elt );
		virtual void visite ( STie& elt );
        virtual void visite ( STimeModification& elt ); //
        virtual void visite ( STimeSign& elt ); //
		virtual void visite ( STranspose& elt ); //
		virtual void visite ( STuplet& elt );
        virtual void visite ( STupletDesc& elt ); //
		virtual void visite ( SUnpitched& elt ); //
		virtual void visite ( SWavyLine& elt );
		virtual void visite ( SWedge& elt );
		virtual void visite ( SWords& elt );
        virtual void visite ( SWork& elt ); //
     
};

/*! @} */

} // namespace MusicXML


#endif
