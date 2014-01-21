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

#ifndef __TScanVisitor__
#define __TScanVisitor__

#include "exports.h"
#include "bimap.h"
#include "TRoutedVisitor.h"
#include <map>
#include <string>

using namespace std;
using namespace MusicXML;

/*!
\addtogroup visitors
@{
*/

/*!
\brief Produces a summary of a MusicXML part.

	A part summary consists in a list of voices included in the part,
	a count of cue notes associated to each staff, a count of
	staves associated to a unique index in the global score context. 
*/
class VEXP TPartSummary : public smartable {
	public:
		VEXP friend SMARTP<TPartSummary> newPartSummary(const string & id);

		//! returns the part ID
		const string& getID () const		{ return fID; }

		//! associates the staff to a value (a global unique index when used by TScanVisitor)
		void setStaff (long staff, long index)	{ fIndexes[staff] = index; }
		//! returns the staff associated value (a global unique index when used by TScanVisitor)
        long getStaffIndex (long staff) 		{ return fIndexes[staff]; }
		//! returns the staff number from its index in the staff list
		long getStaff (long index) const;

		//! increments the number of cue notes for the corresponding staff
        void incCue (long staff)		{ fCues[staff] = fCues[staff] + 1; }
		//! increments the number of notes for the corresponding voice
		void incVoice (long voice, long staff) { fVoices[voice][staff] = fVoices[voice][staff] + 1; }

		//! returns the number of staves for the part
		long countStaves () const				{ return fIndexes.size(); }
		//! returns the number of cue notes for the staff
		long countCueNotes (long staff) 		{ return fCues[staff]; }
		//! returns the number of voices for the part
		long countVoices () const 				{ return fVoices.size(); }
		//! returns the number of voices for the staff
		long countVoices (long staff);
		//! returns the number of notes on a voice
		long countVoiceNotes (long voice);
		//! returns the number of notes on a staff
		long countStaffNotes (long staff);
		//! returns the number of notes for both a voices and a staff
		long countNotes (long voice, long staff)	{ return (fVoices[voice])[staff]; }

		//! returns the voice number for the corresponding index
		long getVoice (long index) const;
		//! returns the main staff for a voice (the staff where most of the notes are located)
		long getMainStaff (long voice);
		
	protected:
		TPartSummary(const string& id);
		~TPartSummary();
		
	private:
		string  fID;
		// voices and corresponding notes count per staff
		map< long, map<long,long> > 	fVoices;
		// staves and number of cue notes per staff
		map<long, long> 	fCues;
		// staves and corresponding global index
		map<long, long> 	fIndexes;
};
typedef SMARTP<TPartSummary> SPartSummary;

/*!
\brief A visitor that only scans a score to gather information.	
*/
class VEXP TScanVisitor : public TRoutedVisitor {

    public:
		TScanVisitor();
		virtual ~TScanVisitor();

		virtual void visite ( SNote& elt );
		virtual void visite ( SPWPart& elt );

        long countParts() const						{ return fParts.size(); }
		map<string, SPartSummary>&	summary()		{ return fParts; }
		const SPartSummary& getPartSummary(const string& partID) { return fParts[partID]; }
		const SPartSummary& current() const			{ return fCurrent; }

	private:
		long	fCurrentStaffIndex;
		map<string, SPartSummary>	fParts;
		SPartSummary				fCurrent;
};

/*! @} */


#endif
