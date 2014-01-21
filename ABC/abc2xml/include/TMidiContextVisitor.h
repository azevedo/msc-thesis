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

#ifndef __TMidiContextVisitor__
#define __TMidiContextVisitor__

#include "exports.h"
#include "TNodeVisitor.h"
#include "smartpointer.h"
#include <string>
#include <map>

using namespace std;
using namespace MusicXML;


/*!
\internal
\brief ...
\todo class documentation
*/
class TInstrument {

    private :
        long fChan;
        long fPart;
    
    public :
        TInstrument(long chan, long part):fChan(chan),fPart(part){}
        const long getPart() {return fPart;}
        const long getChan() {return fChan;}
};

class TChannelTable : public map<string,TInstrument>, public smartable {
    friend SMARTP<TChannelTable> newChannelTable();
};
typedef SMARTP<TChannelTable> SChannelTable; 
   

/*!
\addtogroup visitors
@{
*/

/*
    Time advance only for normal notes, graces notes are not handled.
	Time advance in the \b visiteLeave method of non-terminal elements that deal with time : SChord and SNote.	 
*/

/*!
\brief A Visitor that maintains a context for MIDI generation.
\todo Handling of grace notes. Handling of divisions attribute in coda or segno sound attribute. 
Handling of actual duration in fine sound element.
*/

class VEXP TMidiContextVisitor : public TNodeVisitor {

    private:
        
        map<string, SChannelTable> fMidiInstrumentTable; // table part name, score part
        SChannelTable fCurMap;
    
 		long 	fDivisions; 	// current division
        long 	fCurDate;		// current date
        long 	fMaxDate;		// max of the date kept in a measure
        long 	fDynamics;		// current dynamic value
        long 	fTranspose; 	// current transpose value
        long 	fPart;     		// MIDIFile track number used when parsing score-part, then saved in Instrument
        long	fTempo;			// current tempo;
		long 	fTPQ;			// ticks-per-quater value for date conversion
        bool	fInChord;       // In chord state
     
        void addDuration(long dur);
        void addScorePart(string ident);       
        void addInstrument(string ident, long chan);
    	void setDivisions(long divisions) { fDivisions = divisions; }
        void setDynamics(long dynamics) { fDynamics = (long)((dynamics*90.0)/100.0); }
        void setTranspose(long transpose) { fTranspose = transpose; }
        void setTempo(long tempo) { fTempo = tempo; }
        long getTranspose() { return fTranspose; }
        void initTrack(string ident);
  		long convert2Tick(long val) { return (val*fTPQ)/fDivisions; }

    public:
    
		TMidiContextVisitor(long tpq);
		TMidiContextVisitor();
       	virtual ~TMidiContextVisitor();
          
		/*!
		\brief Get the MIDI channel
		\param elt The note element.
		\return The note MIDI channel from 1 to 16.
		*/
        long getChan(SNote& elt); 

		/*!
		\brief Get the note part number, to be used possibly as the MIDIFile track number.
		\param elt The note element.
		\return The note part number.
		*/
        long getPart(SNote& elt); 


        bool getInstrument(string ident, TInstrument& instrument);        
        bool getInstrument(TInstrument& instrument);

		/*!
		\brief Get the note MIDI pitch.
		\param elt The note element.
		\return The note MIDI pitch.
		*/
        long getPitch(SNote& elt);

		/*!
		\brief Get the note full duration (including the duration of tied notes).
		\param elt The note element.
		\return The note full duration.
		*/
        unsigned long getFullDuration(SNote& elt);

		/*!
		\brief Get the note dynamics, to be used as the MIDI velocity.
		\param elt The note element.
		\return The note dynamics.
		*/
        long getDynamics(SNote& elt) { return fDynamics; } // a revoir 

		/*!
		\brief Get the current date in ticks.
		(MusicXML note durations and dates are expressed using a global Ticks Per Quarter value)
		\return The current date.
		*/
        long getDate() { return convert2Tick(fCurDate); }

		/*!
		\brief Get the current part. 
		\return The current part.
		*/
        long getPart() { return fPart; } // a revoir

		/*!
		\brief Get the current tempo. 
		\return The current tempo in Beat Per Minutes or TSound::undefined is no tempo has been encountered.
		*/
        long getTempo() { return fTempo; }

		/*!
		\brief Get the current TicksPerQuarter value. 
		\return The current TicksPerQuarter value.
		*/
        long getTPQ() { return (long)fTPQ; }
              
        virtual void visiteEnter ( SAttributes& elt );
		virtual void visiteEnter ( SBackup& elt );
        virtual void visiteEnter ( SForward& elt );
        virtual void visiteEnter ( SChord& elt );
        virtual void visiteLeave ( SChord& elt );
        virtual void visiteEnter ( SMidiInstrument& elt );
   		virtual void visiteLeave ( SNote& elt );
        virtual void visiteEnter ( SPWMeasure& elt );
        virtual void visiteEnter ( SPWPart& elt );
        virtual void visiteEnter ( SScoreHeader& elt );
        virtual void visiteEnter ( SScorePart& elt ); 
        virtual void visiteEnter ( SSound& elt );
        virtual void visiteEnter ( STranspose& elt );
    		
};

/*! @} */


#endif
