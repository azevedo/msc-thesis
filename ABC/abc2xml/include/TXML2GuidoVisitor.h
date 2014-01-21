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

#ifndef __TXML2GuidoVisitor__
#define __TXML2GuidoVisitor__

#include "exports.h"
#include "TNotation.h"
#include "TScorePartwise.h"
#include "TScoreVisitor.h"
#include "TScanVisitor.h"
#include "TRational.h"
#include "guido.h"

#include <stack>

using namespace std;
using namespace MusicXML;

/*!
\ingroup visitors guido
@{
*/

/*!
\brief A score visitor to produce a generic Guido representation.
*/

class VEXP TXML2GuidoVisitor : public TScoreVisitor {


    public:
		TXML2GuidoVisitor(bool comments=true, bool checkStem=true, bool bar=false);
		virtual ~TXML2GuidoVisitor();

		void visite ( SAccidental& elt );
		void visite ( SArticulationElement& elt );
		void visite ( SArticulations& elt );
		void visite ( SAttributes& elt );
		void visite ( SBackup& elt );
		void visite ( SBarline& elt );
		void visite ( SBeam& elt );
		void visite ( SBracket& elt );
		void visite ( SChord& elt );
		void visite ( SClef& elt );
		void visite ( SCoda& elt );
		void visite ( SCreator& elt );
		void visite ( SDirection& elt );
		void visite ( SDirectionType& elt );
		void visite ( SDirective& elt );
		void visite ( SDynamic& elt );
		void visite ( SEncoding& elt );
		void visite ( SEnding& elt );
		void visite ( SFermata& elt );
		void visite ( SForward& elt );
		void visite ( SGraceNote& elt );
		void visite ( SGraphNote& elt );
		void visite ( SIdentification& elt );
		void visite ( SKey& elt );
		void visite ( SMetronome& elt );
		void visite ( SMidiInstrument& elt );
		void visite ( SNotation& elt );
		void visite ( SNote& elt );
		void visite ( SNoteHead& elt );
		void visite ( SOctaveShift& elt );
		void visite ( SPWMeasure& elt );
		void visite ( SPWPart& elt );
		void visite ( SPartGroup& elt );
		void visite ( SPartList& elt );
		void visite ( SPitch& elt );
		void visite ( SRepeat& elt );
		void visite ( SRest& elt );
		void visite ( SScoreInstrument& elt );
		void visite ( SScorePart& elt );
		void visite ( SScorePartwise& elt );
		void visite ( SSegno& elt );
		void visite ( SSlur& elt );
		void visite ( SSound& elt );
		void visite ( SStrongAccent& elt );
		void visite ( STie& elt );
		void visite ( STimeModification& elt );
		void visite ( STimeSign& elt );
		void visite ( STranspose& elt );
		void visite ( STuplet& elt );
		void visite ( STupletDesc& elt );
		void visite ( SUnpitched& elt );
		void visite ( SWavyLine& elt );
		void visite ( SWedge& elt );
		void visite ( SWords& elt );

		Sguidoelement& current ()			{ return fStack.top(); }

	protected:
		void add (Sguidoelement& elt)		{ fStack.top()->add(elt); }
		void push (Sguidoelement& elt)		{ add(elt); fStack.push(elt); }
		void pop ()							{ fStack.pop(); }
		long	fCurrentVoice;
		long	fCurrentDivision;
		bool	fMeasureEmpty;

	private:
		enum { kFirst, kMiddle, kLast };
		void startmeasure ();
		void measurend (SPWMeasure meas, int position=kMiddle);
		bool checkgrace (const SNote& note);
		bool checkcue 	(const SNote& note);
		void checkstem 	(const SNote& note);
		void checkstaff (long staff);
		void guidonote (const SNote& note);

		void partheader ( SScoreHeader& elt );
        void guidoheader(SScoreHeader& elt);
        void guidotime(STimeSign& elt);
        const string guidobarstyle(int style) const;

		void addInstr (string instr);

		// the guido elements stack
		stack<Sguidoelement>	fStack;
		// the root element: may be used to search for linked elements
		SScorePartwise		fScore;
		// header is inserted at the beginning of the first sequence and not at its regular place
		bool	fHeaderPending;
		// this is for wedge element with stop attribute handling
		bool	fCrescPending;
		// this is used to scan a part as it may contain several staves and / or voices
		TScanVisitor fScan;
		// a visitor used to scan the slurs and rewrite guido nested slurs (unnested in MusicXML)
		class SlursVisitor: public TRoutedVisitor {
			public:				
				int flag; int num;
				void visite ( SSlur& elt )	{ num++; flag |= elt->getType(); }
		} fSlurs;

        enum location { before, after };
        location fNotePass;
		long	fStaffPass;
		long	fStaffNum;
		long	fMeasNum;
		long	fCurrentStaff;
		int		fStemDirection; 
		
		long	fPendingPops;
		TRational fCurTimeSign;
        
        SPartSummary fCurrentPart;

		bool	fInGrace;
		bool	fInChord;
		bool	fInCue;
		bool	fInRepeat;
		bool	fInTieSlur;
		bool	fBeamOpened;
		int		fBeamNumber;
		
		bool	fComments;
		bool	fCheckStem;
		bool	fGenerateBars;
		bool	fInhibitNextBar;
};

#endif
