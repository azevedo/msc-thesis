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

#ifndef __TChord__
#define __TChord__

#include "exports.h"
#include "common.h"
#include "TNote.h"

namespace MusicXML 
{

/*!
\brief a notes container that denotes a chord.
*/
class EXP TChord : public TMusicData {

    public:
		enum sorttype { pitch, duration };
		EXP friend SMARTP<TChord> newChord();

        virtual void accept(TScoreVisitor& visitor);
		long add(const SNote& note);
		vvector<SNote>& notes()			{ return fNotes; }
		
		//! returns the chord duration that is the shortest note duration (or 0 for an empty chord)
		unsigned long getDuration () const;
		//! returns the voice of the chord notes
		long getVoice () const;
		//! returns the staff of the first chord note
		long getStaff() const;
		//! returns the style of the chord notes (normal, cue, grace)
		TNote::style getStyle () const;

		//! sorts the notes of the chord
		void sort (sorttype type);
		
	protected:
        TChord() {}
        virtual ~TChord() {}

	private:
		vvector<SNote> 	fNotes;
};
typedef SMARTP<TChord> SChord;


} // namespace MusicXML


#endif
